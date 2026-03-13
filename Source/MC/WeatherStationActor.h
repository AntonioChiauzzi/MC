#pragma once

#include "CoreMinimal.h"
#include "MyBaseActor.h"
#include "EntityConfigurable.h"
#include "WeatherStationActor.generated.h"


UCLASS()
class MC_API AWeatherStationActor : public AMyBaseActor, public IEntityConfigurable
{
    GENERATED_BODY()

public:
    AWeatherStationActor();

    virtual void ConfigureFromJson(const TSharedPtr<FJsonObject>& Json) override;
    virtual void SaveToJson(const TSharedPtr<FJsonObject>& Json) override;

    virtual FString GetEntityType() const override;

    UFUNCTION(BlueprintCallable)
    void ApplyAutoScalingToMesh();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, Category = "Weather Readings")
    float AirTemperature;

    UPROPERTY(VisibleAnywhere, Category = "Weather Readings")
    float AirHumidity;

    UPROPERTY(VisibleAnywhere, Category = "Weather Readings")
    float WindSpeed;

};