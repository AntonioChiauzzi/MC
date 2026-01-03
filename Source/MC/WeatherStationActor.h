#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeatherStationActor.generated.h"

class UMyEnvironmentState;

UCLASS()
class MC_API AWeatherStationActor : public AActor
{
    GENERATED_BODY()

public:
    AWeatherStationActor();

    void ReadEnvironment(const UMyEnvironmentState* Environment) const;

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, Category="Sensor")
    float ReadIntervalSeconds;
};

