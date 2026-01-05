#pragma once

#include "CoreMinimal.h"
#include "MyBaseActor.h"
#include "GameFramework/Actor.h"
#include "WeatherStationActor.generated.h"

class UMyEnvironmentState;

UCLASS()
class MC_API AWeatherStationActor : public AMyBaseActor
{
    GENERATED_BODY()

public:
    AWeatherStationActor();

    void ReadEnvironment(const UMyEnvironmentState* Environment) const;

    virtual FString GetEntityType() const override;

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, Category="Sensor")
    float ReadIntervalSeconds;
};

