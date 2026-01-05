#include "WeatherStationActor.h"
#include "MyEnvironmentState.h"

AWeatherStationActor::AWeatherStationActor()
{
    PrimaryActorTick.bCanEverTick = false;
    ReadIntervalSeconds = 5.f;
}

void AWeatherStationActor::BeginPlay()
{
    Super::BeginPlay();
}

void AWeatherStationActor::ReadEnvironment(const UMyEnvironmentState* Environment) const
{
    if (!Environment) return;

    UE_LOG(LogTemp, Log, TEXT("Weather Station Readings"));
    UE_LOG(LogTemp, Log, TEXT("Air Temperature: %f"), Environment->AirTemperature);
    UE_LOG(LogTemp, Log, TEXT("Air Humidity: %f"), Environment->AirHumidity);
    UE_LOG(LogTemp, Log, TEXT("Wind Speed: %f"), Environment->WindSpeed);
}


FString AWeatherStationActor::GetEntityType() const 
{ 
    return TEXT("WeatherStation"); 
}

