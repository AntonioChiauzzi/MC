#include "WorldStateManager.h"

#include "Json.h"
#include "JsonUtilities.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "EngineUtils.h"
#include "TractorPawn.h"
#include "DronePawn.h"
#include "HarvesterPawn.h"
#include "GroundSensorActor.h"
#include "WeatherStationActor.h"
#include "EntityConfigurable.h"
#include "Landscape.h"
#include "LandscapeProxy.h"
#include "MovableVehicle.h"
#include "MyBaseActor.h"



void UWorldStateManager::Initialize(UWorld* InWorld)
{
    World = InWorld;
    if (!ResourceManager)
    {
        ResourceManager = NewObject<UResourceManager>(this);
        if (Environment)
        {
            ResourceManager->EnvState = Environment;
        }
    }
}


void UWorldStateManager::LoadEnvironment()
{
    FString JsonString;
    const FString FilePath = FPaths::ProjectContentDir() + TEXT("Data/environment.json");

    if (!FFileHelper::LoadFileToString(JsonString, *FilePath)) return;

    TSharedPtr<FJsonObject> RootJson;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

    if (!FJsonSerializer::Deserialize(Reader, RootJson) || !RootJson.IsValid()) return;

    TSharedPtr<FJsonObject> EnvJson = RootJson->GetObjectField(TEXT("environment"));
    if (!EnvJson.IsValid()) return;

    if (!Environment)
    {
        Environment = NewObject<UMyEnvironmentState>(this);
    }

    EnvJson->TryGetNumberField(TEXT("soilTemperature"), Environment->SoilTemperature);
    EnvJson->TryGetNumberField(TEXT("soilHumidity"), Environment->SoilHumidity);
    EnvJson->TryGetNumberField(TEXT("soilSolarIrradiance"), Environment->SoilSolarIrradiance);
    EnvJson->TryGetNumberField(TEXT("soilpH"), Environment->SoilpH);
    EnvJson->TryGetNumberField(TEXT("cropMaturity"), Environment->cropMaturity);
    EnvJson->TryGetNumberField(TEXT("windSpeed"), Environment->WindSpeed);
    EnvJson->TryGetNumberField(TEXT("airTemperature"), Environment->AirTemperature);
    EnvJson->TryGetNumberField(TEXT("airHumidity"), Environment->AirHumidity);

    Environment->SoilChemicalComposition.Empty();
    const TSharedPtr<FJsonObject>* ChemJson;
    if (EnvJson->TryGetObjectField(TEXT("soilChemicalComposition"), ChemJson))
    {
        for (const auto& Elem : (*ChemJson)->Values)
        {
            Environment->SoilChemicalComposition.Add(Elem.Key, Elem.Value->AsNumber());
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Environment loaded correctly"));
}


void UWorldStateManager::SaveEnvironment() const
{
    if (!Environment) return;

    TSharedPtr<FJsonObject> EnvJson = MakeShared<FJsonObject>();

    EnvJson->SetNumberField("airTemperature", Environment->AirTemperature);
    EnvJson->SetNumberField("airHumidity", Environment->AirHumidity);
    EnvJson->SetNumberField("windSpeed", Environment->WindSpeed);

    EnvJson->SetNumberField("soilTemperature", Environment->SoilTemperature);
    EnvJson->SetNumberField("soilHumidity", Environment->SoilHumidity);
    EnvJson->SetNumberField("soilSolarIrradiance",
                            Environment->SoilSolarIrradiance);
    EnvJson->SetNumberField("soilpH", Environment->SoilpH);
    EnvJson->SetNumberField("cropMaturity", Environment->cropMaturity);

    TSharedPtr<FJsonObject> ChemJson = MakeShared<FJsonObject>();
    for (const auto& Elem : Environment->SoilChemicalComposition)
    {
        ChemJson->SetNumberField(Elem.Key, Elem.Value);
    }

    EnvJson->SetObjectField("soilChemicalComposition", ChemJson);

    TSharedPtr<FJsonObject> RootJson = MakeShared<FJsonObject>();
    RootJson->SetObjectField("environment", EnvJson);

    FString OutputString;
    TSharedRef<TJsonWriter<>> Writer =
        TJsonWriterFactory<>::Create(&OutputString);

    FJsonSerializer::Serialize(RootJson.ToSharedRef(), Writer);

    const FString FilePath =
        FPaths::ProjectContentDir() + TEXT("Data/environment.json");

    FFileHelper::SaveStringToFile(OutputString, *FilePath);

    UE_LOG(LogTemp, Log, TEXT("Environment saved"));
}


void UWorldStateManager::LoadEntities()
{
    FBox TotalBounds(ForceInit);
    for (TActorIterator<ALandscapeProxy> It(World); It; ++It)
    {
        ALandscapeProxy* LP = *It;
        if (LP)
        {
            TotalBounds += LP->GetComponentsBoundingBox();
        }
    }
    if (TotalBounds.IsValid)
    {
        MinBound = TotalBounds.Min;
        MaxBound = TotalBounds.Max;
    }
    else
    {
        for (TActorIterator<ALandscape> It(World); It; ++It)
        {
            FVector Origin, Extent;
            It->GetActorBounds(false, Origin, Extent);
            MinBound = Origin - Extent;
            MaxBound = Origin + Extent;
            break;
        }
    }
    UE_LOG(LogTemp, Log, TEXT("Confini Landscape Finali: Min %s - Max %s"), *MinBound.ToString(), *MaxBound.ToString());

    FString Json;
    const FString FilePath = FPaths::ProjectContentDir() + TEXT("Data/entities.json");
    if (!FFileHelper::LoadFileToString(Json, *FilePath)) return;

    TSharedPtr<FJsonObject> Root;
    if (!FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Json), Root) || !Root.IsValid()) return;

    const TArray<TSharedPtr<FJsonValue>>* Arr;
    if (!Root->TryGetArrayField(TEXT("entities"), Arr)) return;

    for (const auto& V : *Arr)
    {
        auto Obj = V->AsObject();
        if (!Obj.IsValid()) continue;

        FString Type = Obj->GetStringField(TEXT("type"));

        if (!EntityRegistryAsset || !EntityRegistryAsset->EntityMappings.Contains(Type))
        {
            UE_LOG(LogTemp, Warning, TEXT("Tipo %s non trovato nel Data Asset"), *Type);
            continue;
        }

        TSubclassOf<AActor> ClassToSpawn = EntityRegistryAsset->EntityMappings[Type].ActorClass;

        auto LocObj = Obj->GetObjectField(TEXT("location"));
        FVector Loc(
            LocObj->GetNumberField(TEXT("x")),
            LocObj->GetNumberField(TEXT("y")),
            FMath::Max(0.0f, LocObj->GetNumberField(TEXT("z"))) 
        );

        if (Loc.X < MinBound.X || Loc.X > MaxBound.X || Loc.Y < MinBound.Y || Loc.Y > MaxBound.Y)
        {
            UE_LOG(LogTemp, Warning, TEXT("Entità %s fuori Landscape! Loc: %s | Limiti: Min(%f, %f) Max(%f, %f)"),*Type, *Loc.ToString(), MinBound.X, MinBound.Y, MaxBound.X, MaxBound.Y);
            continue;
        }

        AActor* A = World->SpawnActor<AActor>(ClassToSpawn, Loc, FRotator::ZeroRotator);
        if (!A) continue;

        if (AMyBaseActor* MyBaseActor = Cast<AMyBaseActor>(A))
        {
            MyBaseActor->ID = Obj->GetStringField(TEXT("id"));
            FString NameField;
            if (Obj->TryGetStringField(TEXT("name"), NameField)) {
                MyBaseActor->Name = NameField;
            }
            else {
                MyBaseActor->Name = MyBaseActor->ID;
            }
            MyBaseActor->MapMin = MinBound;
            MyBaseActor->MapMax = MaxBound;

#if WITH_EDITOR
            A->SetActorLabel(MyBaseActor->Name);
#endif
        }

        const TSharedPtr<FJsonObject>* TargetObjPtr;
        if (Obj->TryGetObjectField(TEXT("targetLocation"), TargetObjPtr))
        {
            auto TargetObj = *TargetObjPtr;
            FVector TLoc(
                TargetObj->GetNumberField(TEXT("x")),
                TargetObj->GetNumberField(TEXT("y")),
                TargetObj->GetNumberField(TEXT("z"))
            );
            if (TLoc.X >= MinBound.X && TLoc.X <= MaxBound.X && TLoc.Y >= MinBound.Y && TLoc.Y <= MaxBound.Y)
            {
                if (IMovableVehicle* Movable = Cast<IMovableVehicle>(A))
                {
                    Movable->SetTargetLocation(TLoc);
                }
            }
        }

        if (A->GetClass()->ImplementsInterface(UEntityConfigurable::StaticClass()))
            if (IEntityConfigurable* Configurable = Cast<IEntityConfigurable>(A))
            {
                Configurable->ConfigureFromJson(Obj);
            }

        if (A->Implements<UEnvironmentInjectable>())
        {
            IEnvironmentInjectable::Execute_SetEnvironment(A, Environment);
        }

        if (A->Implements<UEnvironmentReader>())
        {
            IEnvironmentReader::Execute_ReadEnvironment(A, Environment);
        }

        SpawnedEntities.Add(A);

        if (ResourceManager)
        {
            ResourceManager->ManagedEntities.Add(A);
        }
    }
}


void UWorldStateManager::SaveEntities()
{
    TSharedPtr<FJsonObject> RootObject = MakeShared<FJsonObject>();
    TArray<TSharedPtr<FJsonValue>> EntitiesArray;

    if (!World) return;

    for (TActorIterator<AMyBaseActor> It(World); It; ++It)
    {
        AMyBaseActor* MyBaseActor = *It;
        if (!MyBaseActor) continue;

        TSharedPtr<FJsonObject> EntityObj = MakeShared<FJsonObject>();

        EntityObj->SetStringField(TEXT("id"), MyBaseActor->ID);
        EntityObj->SetStringField(TEXT("type"), MyBaseActor->GetEntityType());

        FVector L = MyBaseActor->GetActorLocation();

        TSharedPtr<FJsonObject> Loc = MakeShared<FJsonObject>();

        Loc->SetNumberField(TEXT("x"), L.X);
        Loc->SetNumberField(TEXT("y"), L.Y);
        Loc->SetNumberField(TEXT("z"), FMath::Max(0.0f, L.Z));
        EntityObj->SetObjectField(TEXT("location"), Loc);

        TSharedPtr<FJsonObject> Params = MakeShared<FJsonObject>();
        if (MyBaseActor->Implements<UEntityConfigurable>())
        {
            if (MyBaseActor->GetClass()->ImplementsInterface(UEntityConfigurable::StaticClass()))
                if (IEntityConfigurable* Configurable = Cast<IEntityConfigurable>(MyBaseActor))
                {
                    Configurable->SaveToJson(Params);
                }
            if (Params->Values.Num() > 0)
            {
                EntityObj->SetObjectField(TEXT("params"), Params);
            }
        }

        TOptional<FVector> CurrentTarget;
        if (ATractorPawn* Tractor = Cast<ATractorPawn>(MyBaseActor)) {
            CurrentTarget = Tractor->TargetLocation;
        }
        else if (ADronePawn* Drone = Cast<ADronePawn>(MyBaseActor)) {
            CurrentTarget = Drone->TargetLocation;
        }
        else if (AHarvesterPawn* Harvester = Cast<AHarvesterPawn>(MyBaseActor)) {
            CurrentTarget = Harvester->TargetLocation;
        }
        if (CurrentTarget.IsSet())
        {
            TSharedPtr<FJsonObject> TObj = MakeShared<FJsonObject>();
            TObj->SetNumberField(TEXT("x"), CurrentTarget.GetValue().X);
            TObj->SetNumberField(TEXT("y"), CurrentTarget.GetValue().Y);
            TObj->SetNumberField(TEXT("z"), CurrentTarget.GetValue().Z);
            Params->SetObjectField(TEXT("targetLocation"), TObj);
        }
        EntitiesArray.Add(MakeShared<FJsonValueObject>(EntityObj));
    }

    RootObject->SetArrayField(TEXT("entities"), EntitiesArray);

    FString Output;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Output);
    if (FJsonSerializer::Serialize(RootObject.ToSharedRef(), Writer))
    {
        FString FilePath = FPaths::ProjectContentDir() + TEXT("Data/entities.json");
        FFileHelper::SaveStringToFile(Output, *FilePath);
    }
}