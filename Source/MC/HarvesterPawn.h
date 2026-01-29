#pragma once

#include "CoreMinimal.h"
#include "MyBaseActor.h"
#include "EntityConfigurable.h"
#include "MovableVehicle.h"
#include "HarvesterPawn.generated.h"

UCLASS()
class MC_API AHarvesterPawn : public AMyBaseActor, public IEntityConfigurable, public IMovableVehicle
{
    GENERATED_BODY()

public:
    AHarvesterPawn();

    TOptional<FVector> TargetLocation;

    virtual void Tick(float DeltaTime) override;

    virtual void SetTargetLocation(TOptional<FVector> NewTarget) override;
    virtual void Move(float DeltaTime) override;
    virtual void MoveToTarget(float DeltaTime, FVector Target) override;

    virtual void ConfigureFromJson(const TSharedPtr<FJsonObject>& Json) override;
    virtual void SaveToJson(const TSharedPtr<FJsonObject>& Json) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
    float BatteryLevel = 100.0f;

    UPROPERTY(EditAnywhere)
    float BatteryConsumptionRate = 0.3f; 

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
    float HarvestCapacity = 0.0f; 

    UPROPERTY(EditAnywhere)
    FVector Velocity = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, Category = "Visuals")
    FVector MaxDimensions = FVector(600.0f, 400.0f, 350.0f);

    void ApplyAutoScalingToMesh();

    virtual FString GetEntityType() const override;

protected:
    virtual void BeginPlay() override;
};