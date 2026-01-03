#include "GroundSensorActor.h"
#include "MyEnvironmentState.h"

AGroundSensorActor::AGroundSensorActor()
{
    PrimaryActorTick.bCanEverTick = false;
}

void AGroundSensorActor::BeginPlay()
{
    Super::BeginPlay();

    ReadSoilData();
}

void AGroundSensorActor::SetEnvironmentState(UMyEnvironmentState* InEnvironment)
{
    Environment = InEnvironment;
}

void AGroundSensorActor::ReadSoilData() const
{
    if (!Environment) return;

    UE_LOG(LogTemp, Log, TEXT("Soil Temp: %f"), Environment->SoilTemperature);
    UE_LOG(LogTemp, Log, TEXT("Soil Humidity: %f"), Environment->SoilHumidity);
    UE_LOG(LogTemp, Log, TEXT("Soil pH: %f"), Environment->SoilpH);

    for (const auto& Elem : Environment->SoilChemicalComposition)
    {
        UE_LOG(
            LogTemp,
            Log,
            TEXT("Chemical %s: %f"),
            *Elem.Key,
            Elem.Value
        );
    }
}

