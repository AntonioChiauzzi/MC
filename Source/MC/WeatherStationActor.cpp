#include "WeatherStationActor.h"
#include "MyEnvironmentState.h"

AWeatherStationActor::AWeatherStationActor()
{
    PrimaryActorTick.bCanEverTick = false;
}

void AWeatherStationActor::BeginPlay()
{
    Super::BeginPlay();
}

void AWeatherStationActor::ReadEnvironment_Implementation(const UMyEnvironmentState* Environment)
{
    if (!IsValid(Environment))
    {
        return;
    }
    CurrentAirTemperature = Environment->AirTemperature;
    CurrentAirHumidity = Environment->AirHumidity;
    CurrentWindSpeed = Environment->WindSpeed;

    UE_LOG(LogTemp, Log, TEXT("Stazione Meteo '%s' aggiornata."), *GetName());
}

FString AWeatherStationActor::GetEntityType() const
{
    return TEXT("WeatherStation");
}

void AWeatherStationActor::timerAction()
{
    if (IsValid(EnvironmentState))
    {
        IEnvironmentReader::Execute_ReadEnvironment(this, EnvironmentState);
    }
}