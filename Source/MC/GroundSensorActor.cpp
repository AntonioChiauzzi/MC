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
    SoilTemperature = Environment->SoilTemperature;
    SoilHumidity = Environment->SoilHumidity;
    SoilpH = Environment->SoilpH;
    SoilSolarIrradiance = Environment->SoilSolarIrradiance;
    SoilChemicalComposition = Environment->SoilChemicalComposition;

    UE_LOG(LogTemp, Log, TEXT("GroundSensor '%s' ha aggiornato le letture del suolo."), *GetName());
}

FString AGroundSensorActor::GetEntityType() const
{
    return TEXT("GroundSensor");
}

void AGroundSensorActor::timerAction()
{
    if (IsValid(EnvironmentState))
    {
        IEnvironmentReader::Execute_ReadEnvironment(this, EnvironmentState);
    }
}
