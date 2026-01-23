#pragma once

#include "CoreMinimal.h"
#include "MyBaseActor.h"
#include "EntityConfigurable.h"
#include "HarvesterPawn.generated.h"

UCLASS()
class MC_API AHarvesterPawn : public AMyBaseActor, public IEntityConfigurable
{
    GENERATED_BODY()

public:
    AHarvesterPawn();

    virtual void Tick(float DeltaTime) override;

    virtual void ConfigureFromJson(const TSharedPtr<FJsonObject>& Json) override;
    virtual void SaveToJson(const TSharedPtr<FJsonObject>& Json) override;

    UPROPERTY(EditAnywhere)
    float BatteryLevel = 100.0f;

    UPROPERTY(EditAnywhere)
    float BatteryConsumptionRate = 0.5f; 

    UPROPERTY(EditAnywhere)
    float HarvestCapacity = 0.0f; 

    UPROPERTY(EditAnywhere)
    FVector Velocity = FVector::ZeroVector;
};