#include "GroundSensorActor.h"
#include "MyEnvironmentState.h"

AGroundSensorActor::AGroundSensorActor()
{
    PrimaryActorTick.bCanEverTick = false;
}

void AGroundSensorActor::BeginPlay()
{
    Super::BeginPlay();
}

void AGroundSensorActor::ReadEnvironment_Implementation(const UMyEnvironmentState* Environment)
{
    if (!IsValid(Environment)) return;

    UE_LOG(LogTemp, Log, TEXT("--- Ground Sensor Readings ---"));
    UE_LOG(LogTemp, Log, TEXT("Soil Temp: %f"), Environment->SoilTemperature);
    UE_LOG(LogTemp, Log, TEXT("Soil Humidity: %f"), Environment->SoilHumidity);
    UE_LOG(LogTemp, Log, TEXT("Soil pH: %f"), Environment->SoilpH);

    for (const auto& KVP : Environment->SoilChemicalComposition)
    {
        UE_LOG(LogTemp, Log, TEXT("Chemical %s: %f"), *KVP.Key, KVP.Value);
    }
}

FString AGroundSensorActor::GetEntityType() const 
{ 
    return TEXT("GroundSensor"); 
}

