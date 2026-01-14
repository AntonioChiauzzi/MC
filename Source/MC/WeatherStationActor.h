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

    virtual void Tick(float DeltaTime) override;
protected:
    virtual void BeginPlay() override;
    UFUNCTION(Blueprintable) 
    void timerAction();
    FTimerHandle TimerHandle_Action;
    
    

    UPROPERTY(EditAnywhere, Category="Sensor")
    float ReadIntervalSeconds;
};

