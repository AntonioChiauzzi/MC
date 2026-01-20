#include "GroundSensorActor.h"
#include "MyEnvironmentState.h"

AGroundSensorActor::AGroundSensorActor()
{
    PrimaryActorTick.bCanEverTick = false;
}

void AGroundSensorActor::BeginPlay()
{
    Super::BeginPlay();
    GetWorldTimerManager().SetTimer(
    TimerHandle_Action,
    this,
    &AGroundSensorActor::timerAction,
    120.f,
    true
);
}

void AGroundSensorActor::ReadEnvironment_Implementation(const UMyEnvironmentState* Environment)
{
    if (!IsValid(Environment))
    {
        return;
    }
     SoilTemperature = Environment->SoilTemperature;
    SoilHumidity = Environment->SoilHumidity;
    SoilpH = Environment->SoilpH;
    SoilSolarIrradiance = Environment->SoilSolarIrradiance;
    SoilChemicalComposition = Environment->SoilChemicalComposition;
    

    UE_LOG(LogTemp, Log, TEXT("--- Ground Sensor Readings ---"));
    UE_LOG(LogTemp, Log, TEXT("Soil Temp: %f"), Environment->SoilTemperature);
    UE_LOG(LogTemp, Log, TEXT("Soil Humidity: %f"), Environment->SoilHumidity);
    UE_LOG(LogTemp, Log, TEXT("Soil pH: %f"), Environment->SoilpH);
    UE_LOG(LogTemp, Log, TEXT("Soil Solar Irradiance: %f"), Environment->SoilSolarIrradiance);
    for (const auto& KVP : Environment->SoilChemicalComposition)
    {
        UE_LOG(LogTemp, Log, TEXT("Chemical %s: %f"), *KVP.Key, KVP.Value);
    }
}

FString AGroundSensorActor::GetEntityType() const 
{ 
    return TEXT("GroundSensor"); 
}
void AGroundSensorActor::timerAction()
{
    if (EnvironmentState)
    {
        ReadEnvironment(EnvironmentState);
    }
}
