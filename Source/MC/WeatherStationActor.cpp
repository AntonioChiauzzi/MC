#include "WeatherStationActor.h"
#include "MyEnvironmentState.h"


void AWeatherStationActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
}
void AWeatherStationActor::BeginPlay()
{
    Super::BeginPlay();
   GetWorldTimerManager().SetTimer(
   TimerHandle_Action,
this,
&AWeatherStationActor::timerAction,
120.f,
true
);
}


void AWeatherStationActor::ReadEnvironment_Implementation(const UMyEnvironmentState* Environment)
{
	if (!IsValid(Environment))
	{
		return;
	}
	UE_LOG(LogTemp, Log, TEXT("--- Weather Station Readings ---"));
	UE_LOG(LogTemp, Log, TEXT("Air Temperature: %f"), Environment->AirTemperature);
	UE_LOG(LogTemp, Log, TEXT("Air Humidity: %f"), Environment->AirHumidity);
	UE_LOG(LogTemp, Log, TEXT("Wind Speed: %f"), Environment->WindSpeed);
}



FString AWeatherStationActor::GetEntityType() const 
{ 
    return TEXT("WeatherStation"); 
}

void AWeatherStationActor::timerAction()
{
    UE_LOG(LogTemp, Warning, TEXT("⏱ Timer scattato! Tempo: %f"), GetWorld()->GetTimeSeconds());
}

