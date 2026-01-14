#pragma once

#include "CoreMinimal.h"
#include "MyBaseActor.h"
#include "EnvironmentReader.h"
#include "GameFramework/Actor.h"
#include "WeatherStationActor.generated.h"

class UMyEnvironmentState;

UCLASS()
class MC_API AWeatherStationActor : public AMyBaseActor, public IEnvironmentReader
{
    GENERATED_BODY()

public:
    AWeatherStationActor();

    virtual void ReadEnvironment_Implementation(const UMyEnvironmentState* Environment) override;

    virtual FString GetEntityType() const override;

    virtual void Tick(float DeltaTime) override;
protected:
    virtual void BeginPlay() override;
    UFUNCTION(Blueprintable)
    void timerAction();
    FTimerHandle TimerHandle_Action;

};