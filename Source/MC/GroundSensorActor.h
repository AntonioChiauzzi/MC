#pragma once

#include "CoreMinimal.h"
#include "MyBaseActor.h"
#include "EntityConfigurable.h"
#include "GroundSensorActor.generated.h"

class UMyEnvironmentState;

UCLASS()
class MC_API AGroundSensorActor : public AMyBaseActor, public IEntityConfigurable
{
    GENERATED_BODY()

public:
    AGroundSensorActor();

    virtual void ConfigureFromJson(const TSharedPtr<FJsonObject>& Json) override;
    virtual void SaveToJson(const TSharedPtr<FJsonObject>& Json) override;

    virtual FString GetEntityType() const override;

    UFUNCTION(BlueprintCallable)
    void ApplyAutoScalingToMesh();

    virtual FString ToString() const override;
protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, Category = "Soil Readings", meta = (DisplayName = "Soil pH"))
    float SoilpH;

    UPROPERTY(VisibleAnywhere, Category = "Soil Readings")
    float SoilHumidity;

    UPROPERTY(VisibleAnywhere, Category = "Soil Readings")
    float SoilTemperature;

    UPROPERTY(VisibleAnywhere, Category = "Soil Readings")
    float SoilSolarIrradiance;

    UPROPERTY(VisibleAnywhere, Category = "Soil Readings")
    TMap<FString, float> SoilChemicalComposition;
};