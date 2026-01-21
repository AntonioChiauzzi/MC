#pragma once

#include "CoreMinimal.h"
#include "MyBaseActor.h"
#include "EnvironmentReader.h"
#include "GameFramework/Actor.h"
#include "GroundSensorActor.generated.h"


UCLASS()
class MC_API AGroundSensorActor : public AMyBaseActor, public IEnvironmentReader
{
    GENERATED_BODY()

public:
    AGroundSensorActor();

    virtual FString GetEntityType() const override;

    virtual void ReadEnvironment_Implementation(const UMyEnvironmentState* Environment) override;


protected:
    virtual void BeginPlay() override;
    
    UFUNCTION(Blueprintable) 
    void timerAction();
    FTimerHandle TimerHandle_Action;
    
    UPROPERTY(VisibleAnywhere, Category = "Soil Readings", meta = (DisplayName = "Soil pH"))
    float SoilpH;

    UPROPERTY(VisibleAnywhere, Category="Soil Readings")
    float SoilHumidity;

    UPROPERTY(VisibleAnywhere, Category="Soil Readings")
    float SoilTemperature;
    
    UPROPERTY(VisibleAnywhere, Category="Soil Readings")
    float SoilSolarIrradiance;
    
    UPROPERTY(VisibleAnywhere, Category="Soil Readings")
    TMap<FString, float> SoilChemicalComposition;
    
    UPROPERTY(EditAnywhere, Category="Environment")
    UMyEnvironmentState* EnvironmentState;
    
    
};
