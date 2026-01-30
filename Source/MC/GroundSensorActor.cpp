#include "GroundSensorActor.h"
#include "MyEnvironmentState.h"

AGroundSensorActor::AGroundSensorActor()
{
    PrimaryActorTick.bCanEverTick = false;
}

void AGroundSensorActor::BeginPlay()
{
    Super::BeginPlay();
    /*GetWorldTimerManager().SetTimer(
    TimerHandle_Action,
    this,
    &AGroundSensorActor::timerAction,
    120.f,
    true
);*/
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
