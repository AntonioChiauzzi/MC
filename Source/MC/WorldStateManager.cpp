#include "WorldStateManager.h"

#include "MCGameInstance.h"
#include "MyEnvironmentState.h"
#include "EntityRegistryDataAsset.h"
#include "ResourceManager.h"
#include "TractorPawn.h"
#include "DronePawn.h"
#include "HarvesterPawn.h"
#include "GroundSensorActor.h"
#include "WeatherStationActor.h"
#include "EntityConfigurable.h"
#include "MovableVehicle.h"
#include "MyBaseActor.h"

#include "Json.h"
#include "JsonUtilities.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"
#include "Landscape.h"      
#include "LandscapeProxy.h"
#include "Engine/SkyLight.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"

#include "Developer/DesktopPlatform/Public/IDesktopPlatform.h"
#include "Developer/DesktopPlatform/Public/DesktopPlatformModule.h" 



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
    const FString FilePath = FPaths::Combine(BaseDataPath, TEXT("environment.json"));

    FString JsonString;
    if (!FFileHelper::LoadFileToString(JsonString, *FilePath)) return;

    TSharedPtr<FJsonObject> RootJson;
    if (!FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(JsonString), RootJson) || !RootJson.IsValid()) return;

    TSharedPtr<FJsonObject> EnvJson = RootJson->GetObjectField(TEXT("environment"));
    if (EnvJson.IsValid())
    {
        UpdateEnvironmentFromJson(EnvJson);
        UE_LOG(LogTemp, Log, TEXT("Environment loaded correctly from: %s"), *FilePath);
    }
}

void UWorldStateManager::UpdateEnvironmentFromJson(const TSharedPtr<FJsonObject>& EnvJson)
{
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
}

void UWorldStateManager::SaveEnvironment() const
{
    if (!Environment) return;
    FString PathToUse = BaseDataPath;
    if (UMCGameInstance* GI = Cast<UMCGameInstance>(GetWorld()->GetGameInstance()))
    {
        if (!GI->SavedBaseDataPath.IsEmpty()) PathToUse = GI->SavedBaseDataPath;
    }

    if (PathToUse.IsEmpty()) return;
    TSharedPtr<FJsonObject> RootJson = MakeShared<FJsonObject>();
    RootJson->SetObjectField("environment", ConvertEnvironmentToJson());
    FString OutputString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
    if (FJsonSerializer::Serialize(RootJson.ToSharedRef(), Writer))
    {
        const FString FilePath = FPaths::Combine(PathToUse, TEXT("environment.json"));
        FFileHelper::SaveStringToFile(OutputString, *FilePath);
        UE_LOG(LogTemp, Log, TEXT("Environment salvato correttamente in: %s"), *FilePath);
    }
}

TSharedPtr<FJsonObject> UWorldStateManager::ConvertEnvironmentToJson() const
{
    TSharedPtr<FJsonObject> EnvJson = MakeShared<FJsonObject>();
    EnvJson->SetNumberField("airTemperature", Environment->AirTemperature);
    EnvJson->SetNumberField("airHumidity", Environment->AirHumidity);
    EnvJson->SetNumberField("windSpeed", Environment->WindSpeed);
    EnvJson->SetNumberField("soilTemperature", Environment->SoilTemperature);
    EnvJson->SetNumberField("soilHumidity", Environment->SoilHumidity);
    EnvJson->SetNumberField("soilSolarIrradiance", Environment->SoilSolarIrradiance);
    EnvJson->SetNumberField("soilpH", Environment->SoilpH);
    EnvJson->SetNumberField("cropMaturity", Environment->cropMaturity);
    TSharedPtr<FJsonObject> ChemJson = MakeShared<FJsonObject>();
    for (const auto& Elem : Environment->SoilChemicalComposition)
    {
        ChemJson->SetNumberField(Elem.Key, Elem.Value);
    }
    EnvJson->SetObjectField("soilChemicalComposition", ChemJson);
    return EnvJson;
}

void UWorldStateManager::LoadEntities()
{
    const FString FilePath = FPaths::Combine(BaseDataPath, TEXT("entities.json"));
    FString Json;
    if (!FFileHelper::LoadFileToString(Json, *FilePath))
        return;
    TSharedPtr<FJsonObject> Root;
    if (!FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Json), Root) || !Root.IsValid())
        return;
    const TArray<TSharedPtr<FJsonValue>>* Arr;
    if (!Root->TryGetArrayField(TEXT("entities"), Arr))
        return;
    for (const auto& V : *Arr)
    {
        const TSharedPtr<FJsonObject> Obj = V->AsObject();
        if (!Obj.IsValid())
            continue;
        const FString Type = Obj->GetStringField(TEXT("type"));
        if (!EntityRegistryAsset || !EntityRegistryAsset->EntityMappings.Contains(Type))
            continue;
        if (AActor* NewActor = SpawnEntityFromJson(Obj))
        {
            SpawnedEntities.Add(NewActor);
            if (ResourceManager)
            {
                ResourceManager->ManagedEntities.Add(NewActor);
            }
        }
    }
}

void UWorldStateManager::ResizeLandscape(const FVector TargetSize)
{
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("ResizeLandscape: World nullo"));
        return;
    }
    TArray<ALandscapeProxy*> LandscapeProxies;
    FBox TotalBounds(ForceInit);
    for (TActorIterator<ALandscapeProxy> It(World); It; ++It)
    {
        ALandscapeProxy* LP = *It;
        if (LP)
        {
            LandscapeProxies.Add(LP);
            TotalBounds += LP->GetComponentsBoundingBox(true);

            UE_LOG(LogTemp, Warning, TEXT("ResizeLandscape: Found LandscapeProxy %s  Scale=%s"),
                *LP->GetName(),
                *LP->GetActorScale3D().ToString());
        }
    }
    if (LandscapeProxies.Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("ResizeLandscape: nessun LandscapeProxy trovato"));
        return;
    }
    if (!TotalBounds.IsValid)
    {
        UE_LOG(LogTemp, Error, TEXT("ResizeLandscape: TotalBounds non validi"));
        return;
    }
    const FVector SizeBefore = TotalBounds.GetSize();
    if (SizeBefore.X <= KINDA_SMALL_NUMBER || SizeBefore.Y <= KINDA_SMALL_NUMBER)
    {
        UE_LOG(LogTemp, Error, TEXT("ResizeLandscape: bounds invalidi (%s)"), *SizeBefore.ToString());
        return;
    }
    const FVector OldScale = LandscapeProxies[0]->GetActorScale3D();
    FVector NewScale = OldScale;
    if (TargetSize.X > 0.f)
    {
        NewScale.X = OldScale.X * (TargetSize.X / SizeBefore.X);
    }
    if (TargetSize.Y > 0.f)
    {
        NewScale.Y = OldScale.Y * (TargetSize.Y / SizeBefore.Y);
    }
    NewScale.Z = OldScale.Z;
    UE_LOG(LogTemp, Warning,
        TEXT("ResizeLandscape BEFORE: Size=%s  OldScale=%s  Target=%s  NewScale=%s"),
        *SizeBefore.ToString(),
        *OldScale.ToString(),
        *TargetSize.ToString(),
        *NewScale.ToString()
    );
    for (ALandscapeProxy* LP : LandscapeProxies)
    {
        if (!LP)
            continue;
        LP->SetActorScale3D(NewScale);
        LP->MarkComponentsRenderStateDirty();
        LP->ReregisterAllComponents();
        UE_LOG(LogTemp, Warning, TEXT("ResizeLandscape: Applied scale to %s  NewScale=%s"),
            *LP->GetName(),
            *LP->GetActorScale3D().ToString());
    }
    UpdateLandscapeBounds();
    const FVector CenterAfter = (MinBound + MaxBound) * 0.5f;
    WorldOffset = FVector(-CenterAfter.X, -CenterAfter.Y, 0.f);
    const FVector SizeAfter = MaxBound - MinBound;
    UE_LOG(LogTemp, Warning,
        TEXT("ResizeLandscape AFTER: Size=%s  Center=%s  WorldOffset=%s"),
        *SizeAfter.ToString(),
        *CenterAfter.ToString(),
        *WorldOffset.ToString()
    );
}

void UWorldStateManager::UpdateLandscapeBounds()
{
    FBox TotalBounds(ForceInit);

    for (TActorIterator<ALandscapeProxy> It(World); It; ++It)
    {
        if (ALandscapeProxy* LP = *It)
        {
            TotalBounds += LP->GetComponentsBoundingBox(true);
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
    const FVector MinLogical = MinBound + WorldOffset;
    const FVector MaxLogical = MaxBound + WorldOffset;
    const FVector LogicalSize = MaxLogical - MinLogical;
    const FVector WorldSize = MaxBound - MinBound;
    UE_LOG(LogTemp, Log,
        TEXT("Confini Landscape Aggiornati: Min %s - Max %s | Logical Min %s - Max %s | WorldSize %s | LogicalSize %s | WorldOffset %s"),
        *MinBound.ToString(),
        *MaxBound.ToString(),
        *MinLogical.ToString(),
        *MaxLogical.ToString(),
        *WorldSize.ToString(),
        *LogicalSize.ToString(),
        *WorldOffset.ToString());
}

AActor* UWorldStateManager::SpawnEntityFromJson(const TSharedPtr<FJsonObject>& Obj)
{
    FString Type = Obj->GetStringField(TEXT("type"));
    if (!EntityRegistryAsset || !EntityRegistryAsset->EntityMappings.Contains(Type))
        return nullptr;
    TSubclassOf<AActor> ClassToSpawn = EntityRegistryAsset->EntityMappings[Type].ActorClass;
    if (!World || !*ClassToSpawn)
        return nullptr;
    const FVector MinLogical = MinBound + WorldOffset;
    const FVector MaxLogical = MaxBound + WorldOffset;
    auto LocObj = Obj->GetObjectField(TEXT("location"));
    const FVector LocLogical(
        LocObj->GetNumberField(TEXT("x")),
        LocObj->GetNumberField(TEXT("y")),
        FMath::Max(0.0f, LocObj->GetNumberField(TEXT("z")))
    );
    if (LocLogical.X < MinLogical.X || LocLogical.X > MaxLogical.X ||
        LocLogical.Y < MinLogical.Y || LocLogical.Y > MaxLogical.Y)
    {
        UE_LOG(LogTemp, Warning, TEXT("Entità %s fuori Landscape (Logical)! Loc=%s  LogicalMin=%s  LogicalMax=%s"),
            *Type, *LocLogical.ToString(), *MinLogical.ToString(), *MaxLogical.ToString());
        return nullptr;
    }
    const FVector LocWorld = ApplyWorldOffset_LogicalToWorld(LocLogical);
    AActor* A = World->SpawnActor<AActor>(ClassToSpawn, LocWorld, FRotator::ZeroRotator);
    if (!A) return nullptr;
    if (AMyBaseActor* MyBaseActor = Cast<AMyBaseActor>(A))
    {
        MyBaseActor->ID = Obj->GetStringField(TEXT("id"));
        MyBaseActor->Name = Obj->HasField(TEXT("name")) ? Obj->GetStringField(TEXT("name")) : MyBaseActor->ID;
        MyBaseActor->MapMin = MinLogical;
        MyBaseActor->MapMax = MaxLogical;
        MyBaseActor->WorldOffset = WorldOffset;

#if WITH_EDITOR
        A->SetActorLabel(MyBaseActor->Name);
#endif
    }

    const TSharedPtr<FJsonObject>* TargetObjPtr;
    if (Obj->TryGetObjectField(TEXT("targetLocation"), TargetObjPtr) && TargetObjPtr && TargetObjPtr->IsValid())
    {
        auto TargetObj = *TargetObjPtr;
        const FVector TargetLogical(
            TargetObj->GetNumberField(TEXT("x")),
            TargetObj->GetNumberField(TEXT("y")),
            TargetObj->GetNumberField(TEXT("z"))
        );
        const FVector TargetWorld = ApplyWorldOffset_LogicalToWorld(TargetLogical);
        if (IMovableVehicle* Movable = Cast<IMovableVehicle>(A))
        {
            Movable->SetTargetLocation(TOptional<FVector>(TargetWorld));
        }
    }
    if (IEntityConfigurable* Configurable = Cast<IEntityConfigurable>(A))
    {
        Configurable->ConfigureFromJson(Obj);
    }
    if (A->Implements<UEnvironmentInjectable>())
        IEnvironmentInjectable::Execute_SetEnvironment(A, Environment);
    if (A->Implements<UEnvironmentReader>())
        IEnvironmentReader::Execute_ReadEnvironment(A, Environment);
    return A;
}

void UWorldStateManager::RemoveOldInstances()
{
    for (AActor* Actor : SpawnedEntities) {
        if (Actor && Actor->IsValidLowLevel()) {
            Actor->Destroy();
        }
    }
    SpawnedEntities.Empty();
    if (ResourceManager)
    {
        for (AActor* Actor : ResourceManager->ManagedEntities) {
            if (Actor && Actor->IsValidLowLevel()) {
                Actor->Destroy();
            }
        }
        ResourceManager->ManagedEntities.Empty();
    }
    UE_LOG(LogTemp, Warning, TEXT("Entità distrutte"));
}

void UWorldStateManager::SaveEntities()
{
    if (!World) return;
    FString PathToUse = BaseDataPath;
    if (UMCGameInstance* GI = Cast<UMCGameInstance>(World->GetGameInstance()))
    {
        if (!GI->SavedBaseDataPath.IsEmpty()) PathToUse = GI->SavedBaseDataPath;
    }
    if (PathToUse.IsEmpty()) return;
    TSharedPtr<FJsonObject> RootObject = MakeShared<FJsonObject>();
    TArray<TSharedPtr<FJsonValue>> EntitiesArray;
    for (TActorIterator<AMyBaseActor> It(World); It; ++It)
    {
        if (AMyBaseActor* MyBaseActor = *It)
        {
            EntitiesArray.Add(MakeShared<FJsonValueObject>(ConvertEntityToJson(MyBaseActor)));
        }
    }
    RootObject->SetArrayField(TEXT("entities"), EntitiesArray);
    FString Output;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Output);
    if (FJsonSerializer::Serialize(RootObject.ToSharedRef(), Writer))
    {
        const FString FilePath = FPaths::Combine(PathToUse, TEXT("entities.json"));
        FFileHelper::SaveStringToFile(Output, *FilePath);
        UE_LOG(LogTemp, Log, TEXT("Entità salvate in: %s"), *FilePath);
    }
}

TSharedPtr<FJsonObject> UWorldStateManager::ConvertEntityToJson(AMyBaseActor* Actor)
{
    TSharedPtr<FJsonObject> EntityObj = MakeShared<FJsonObject>();
    EntityObj->SetStringField(TEXT("id"), Actor->ID);
    EntityObj->SetStringField(TEXT("type"), Actor->GetEntityType());
    const FVector WorldLoc = Actor->GetActorLocation();
    const FVector LogicalLoc = WorldLoc + Actor->WorldOffset;
    TSharedPtr<FJsonObject> Loc = MakeShared<FJsonObject>();
    Loc->SetNumberField(TEXT("x"), LogicalLoc.X);
    Loc->SetNumberField(TEXT("y"), LogicalLoc.Y);
    Loc->SetNumberField(TEXT("z"), FMath::Max(0.0f, LogicalLoc.Z));
    EntityObj->SetObjectField(TEXT("location"), Loc);
    TSharedPtr<FJsonObject> Params = MakeShared<FJsonObject>();
    if (IEntityConfigurable* Configurable = Cast<IEntityConfigurable>(Actor))
    {
        Configurable->SaveToJson(Params);
    }
    if (Params->Values.Num() > 0)
    {
        EntityObj->SetObjectField(TEXT("params"), Params);
    }
    TOptional<FVector> CurrentTargetWorld;
    if (ATractorPawn* Tractor = Cast<ATractorPawn>(Actor)) CurrentTargetWorld = Tractor->TargetLocation;
    else if (ADronePawn* Drone = Cast<ADronePawn>(Actor)) CurrentTargetWorld = Drone->TargetLocation;
    else if (AHarvesterPawn* Harvester = Cast<AHarvesterPawn>(Actor)) CurrentTargetWorld = Harvester->TargetLocation;
    if (CurrentTargetWorld.IsSet())
    {
        const FVector TargetLogical = CurrentTargetWorld.GetValue() + Actor->WorldOffset;
        TSharedPtr<FJsonObject> TObj = MakeShared<FJsonObject>();
        TObj->SetNumberField(TEXT("x"), TargetLogical.X);
        TObj->SetNumberField(TEXT("y"), TargetLogical.Y);
        TObj->SetNumberField(TEXT("z"), TargetLogical.Z);
        EntityObj->SetObjectField(TEXT("targetLocation"), TObj);
    }
    return EntityObj;
}

FVector UWorldStateManager::ComputeTargetSizeFromEntitiesJson(float Margin) const
{
    const FString FilePath = FPaths::Combine(BaseDataPath, TEXT("entities.json"));
    FString Json;
    if (!FFileHelper::LoadFileToString(Json, *FilePath))
    {
        UE_LOG(LogTemp, Warning, TEXT("ComputeTargetSizeFromEntitiesJson: impossibile leggere %s"), *FilePath);
        return FVector::ZeroVector;
    }
    TSharedPtr<FJsonObject> Root;
    if (!FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Json), Root) || !Root.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("ComputeTargetSizeFromEntitiesJson: JSON non valido"));
        return FVector::ZeroVector;
    }
    const TArray<TSharedPtr<FJsonValue>>* Arr = nullptr;
    if (!Root->TryGetArrayField(TEXT("entities"), Arr) || !Arr)
    {
        UE_LOG(LogTemp, Warning, TEXT("ComputeTargetSizeFromEntitiesJson: campo 'entities' mancante"));
        return FVector::ZeroVector;
    }
    double MaxAbsX = 0.0;
    double MaxAbsY = 0.0;
    for (const auto& V : *Arr)
    {
        const TSharedPtr<FJsonObject> Obj = V->AsObject();
        if (!Obj.IsValid() || !Obj->HasField(TEXT("location")))
            continue;
        const TSharedPtr<FJsonObject> LocObj = Obj->GetObjectField(TEXT("location"));
        const double X = LocObj->GetNumberField(TEXT("x"));
        const double Y = LocObj->GetNumberField(TEXT("y"));
        MaxAbsX = FMath::Max(MaxAbsX, FMath::Abs(X));
        MaxAbsY = FMath::Max(MaxAbsY, FMath::Abs(Y));
    }
    const float TargetX = float(MaxAbsX * 2.0 + Margin * 2.0);
    const float TargetY = float(MaxAbsY * 2.0 + Margin * 2.0);
    UE_LOG(LogTemp, Warning, TEXT("FitToData: MaxAbsX=%.2f MaxAbsY=%.2f => TargetSize X=%.2f Y=%.2f (Margin=%.2f)"),
        float(MaxAbsX), float(MaxAbsY), TargetX, TargetY, Margin);
    return FVector(TargetX, TargetY, 0.f);
}

void UWorldStateManager::OpenDirectoryDialogJson()
{
    IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
    FString SelectedDirectory;
    if (DesktopPlatform)
    {
        bool bOpened = DesktopPlatform->OpenDirectoryDialog(
            nullptr,
            TEXT("Seleziona la cartella dei JSON (Annulla per usare Content/Data)"),
            FPaths::ProjectContentDir(),
            SelectedDirectory
        );
        if (!bOpened || SelectedDirectory.IsEmpty())
        {
            SelectedDirectory = FPaths::Combine(FPaths::ProjectContentDir(), TEXT("Data"));
            UE_LOG(LogTemp, Warning, TEXT("Dialog JSON annullato. Uso fallback: %s"), *SelectedDirectory);
        }
    }
    if (UMCGameInstance* GI = Cast<UMCGameInstance>(GetWorld()->GetGameInstance()))
    {
        FPaths::NormalizeDirectoryName(SelectedDirectory);
        GI->SavedBaseDataPath = SelectedDirectory;
        BaseDataPath = SelectedDirectory;
        UE_LOG(LogTemp, Log, TEXT("SavedBaseDataPath impostato a: %s"), *GI->SavedBaseDataPath);
    }
}

void UWorldStateManager::OpenDirectoryDialogMap()
{
    IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
    FString SelectedDirectory;
    if (DesktopPlatform)
    {
        bool bOpened = DesktopPlatform->OpenDirectoryDialog(
            nullptr,
            TEXT("Seleziona la cartella contenente la Mappa (Annulla per caricare NewMap di default)"),
            FPaths::ProjectContentDir(),
            SelectedDirectory
        );
        if (!bOpened || SelectedDirectory.IsEmpty())
        {
            UE_LOG(LogTemp, Warning, TEXT("Dialog annullato. Utilizzo percorso di default: Content/"));
            SelectedDirectory = FPaths::ProjectContentDir();
        }
    }
    if (UMCGameInstance* GI = Cast<UMCGameInstance>(GetWorld()->GetGameInstance()))
    {
        FPaths::NormalizeDirectoryName(SelectedDirectory);
        GI->SavedMapPath = SelectedDirectory;
        UE_LOG(LogTemp, Log, TEXT("SavedMapPath impostato a: %s"), *GI->SavedMapPath);
    }
}