#include "WorldStateManager.h"

#include "Json.h"
#include "JsonUtilities.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

#include "TractorPawn.h"
#include "DronePawn.h"
#include "GroundSensorActor.h"
#include "WeatherStationActor.h"
#include "EntityConfigurable.h"


void UWorldStateManager::Initialize(UWorld* InWorld)
{
    World = InWorld;
    RegisterClasses();
}

void UWorldStateManager::RegisterClasses()
{
    EntityClassRegistry.Add("Tractor", ATractorPawn::StaticClass());
    EntityClassRegistry.Add("Drone", ADronePawn::StaticClass());
    EntityClassRegistry.Add("GroundSensor", AGroundSensorActor::StaticClass());
    EntityClassRegistry.Add("WeatherStation", AWeatherStationActor::StaticClass());
}

void UWorldStateManager::LoadEnvironment()
{
    FString JsonString;
    const FString FilePath = FPaths::ProjectContentDir() + TEXT("Data/environment.json");

    if (!FFileHelper::LoadFileToString(JsonString, *FilePath))
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot load environment.json"));
        return;
    }

    TSharedPtr<FJsonObject> RootJson;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

    if (!FJsonSerializer::Deserialize(Reader, RootJson) || !RootJson.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid JSON"));
        return;
    }

    const TSharedPtr<FJsonObject>* EnvJson;

    if (!RootJson->TryGetObjectField(TEXT("environment"), EnvJson))
    {
        UE_LOG(LogTemp, Error, TEXT("Missing environment object"));
        return;
    }

    if (!Environment)
    {
        Environment = NewObject<UMyEnvironmentState>(this);
    }

    Environment->SoilTemperature = (*EnvJson)->GetNumberField(TEXT("soilTemperature"));
    Environment->SoilHumidity = (*EnvJson)->GetNumberField(TEXT("soilHumidity"));
    Environment->SoilSolarIrradiance = (*EnvJson)->GetNumberField(TEXT("soilSolarIrradiance"));
    Environment->SoilpH = (*EnvJson)->GetNumberField(TEXT("soilpH"));
    Environment->WindSpeed = (*EnvJson)->GetNumberField(TEXT("windSpeed"));
    Environment->AirTemperature = (*EnvJson)->GetNumberField(TEXT("airTemperature"));
    Environment->AirHumidity = (*EnvJson)->GetNumberField(TEXT("airHumidity"));

    Environment->SoilChemicalComposition.Empty();

    const TSharedPtr<FJsonObject>* ChemJson;
    if ((*EnvJson)->TryGetObjectField(TEXT("soilChemicalComposition"), ChemJson))
    {
        for (const auto& Elem : (*ChemJson)->Values)
        {
            Environment->SoilChemicalComposition.Add(
                Elem.Key,
                Elem.Value->AsNumber()
            );
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Environment loaded correctly"));
}


void UWorldStateManager::SaveEnvironment() const
{
    if (!Environment) return;

    TSharedPtr<FJsonObject> EnvJson = MakeShared<FJsonObject>();

    EnvJson->SetNumberField("air_temperature", Environment->AirTemperature);
    EnvJson->SetNumberField("air_humidity", Environment->AirHumidity);
    EnvJson->SetNumberField("wind_speed", Environment->WindSpeed);

    EnvJson->SetNumberField("soil_temperature", Environment->SoilTemperature);
    EnvJson->SetNumberField("soil_humidity", Environment->SoilHumidity);
    EnvJson->SetNumberField("soil_solar_irradiance",
                            Environment->SoilSolarIrradiance);
    EnvJson->SetNumberField("soil_ph", Environment->SoilpH);

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
    FString Json;
    FFileHelper::LoadFileToString(Json,
        *(FPaths::ProjectContentDir() + "Data/entities.json"));

    TSharedPtr<FJsonObject> Root;
    FJsonSerializer::Deserialize(
        TJsonReaderFactory<>::Create(Json), Root);

    const auto& Arr = Root->GetArrayField(TEXT("entities"));

    for (const auto& V : Arr)
    {
        auto Obj = V->AsObject();
        FString Type = Obj->GetStringField(TEXT("type"));

        if (!EntityClassRegistry.Contains(Type)) continue;

        auto L = Obj->GetObjectField(TEXT("location"));
        FVector Loc(
            L->GetNumberField(TEXT("x")),
            L->GetNumberField(TEXT("y")),
            L->GetNumberField(TEXT("z"))
        );

        AActor* A = GetWorld()->SpawnActor<AActor>(
            EntityClassRegistry[Type], Loc, FRotator::ZeroRotator);

        if (AMyBasePawn* BP = Cast<AMyBasePawn>(A))
            BP->PawnID = Obj->GetStringField(TEXT("id"));

        if (A->GetClass()->ImplementsInterface(UEntityConfigurable::StaticClass()))
            if (IEntityConfigurable* Configurable = Cast<IEntityConfigurable>(A))
            {
                Configurable->ConfigureFromJson(Obj);
            }
    }
}



void UWorldStateManager::SaveEntities()
{
    /*TSharedPtr<FJsonObject> Root = MakeShared<FJsonObject>();
    TArray<TSharedPtr<FJsonValue>> EntitiesArray;

    for (AActor* Actor : SpawnedEntities)
    {
        if (!Actor)
            continue;

        TSharedPtr<FJsonObject> Obj = MakeShared<FJsonObject>();

        
        if (ABasePawn* Pawn = Cast<ABasePawn>(Actor))
        {
            Obj->SetStringField(
                "id",
                Pawn->GetEntityId()
            );
        }

        
        Obj->SetStringField(
            "class",
            Actor->GetClass()->GetName()
        );

        
        FVector L = Actor->GetActorLocation();
        TSharedPtr<FJsonObject> Loc = MakeShared<FJsonObject>();
        Loc->SetNumberField("x", L.X);
        Loc->SetNumberField("y", L.Y);
        Loc->SetNumberField("z", L.Z);
        Obj->SetObjectField("location", Loc);

        EntitiesArray.Add(
            MakeShared<FJsonValueObject>(Obj)
        );
    }

    Root->SetArrayField("entities", EntitiesArray);

    FString Output;
    TSharedRef<TJsonWriter<>> Writer =
        TJsonWriterFactory<>::Create(&Output);

    FJsonSerializer::Serialize(
        Root.ToSharedRef(),
        Writer
    );

    FFileHelper::SaveStringToFile(
        Output,
        *(FPaths::ProjectContentDir() + "Data/entities.json")
    );*/
}


