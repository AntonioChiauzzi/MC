#pragma once

#include "CoreMinimal.h"
#include "MyBaseActor.h"
#include "EntityConfigurable.h"
#include "MovableVehicle.h"
#include "DronePawn.generated.h"

class UStaticMeshComponent;

UCLASS()
class MC_API ADronePawn : public AMyBaseActor, public IEntityConfigurable, public IMovableVehicle
{
    GENERATED_BODY()

public:
    ADronePawn();

    UPROPERTY()
    float CropMaturity;

    UPROPERTY()
    FVector Velocity;

    TOptional<FVector> TargetLocation;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
    float BatteryLevel;

    UPROPERTY(EditAnywhere)
    float BatteryConsumptionRate = 0.5f;

    virtual void Tick(float DeltaTime) override;

    virtual void SetTargetLocation(TOptional<FVector> NewTarget) override;
    virtual void Move(float DeltaTime) override;
    virtual void MoveToTarget(float DeltaTime, FVector Target) override;

    virtual void ConfigureFromJson(const TSharedPtr<FJsonObject>& Json) override;
    virtual void SaveToJson(const TSharedPtr<FJsonObject>& Json) override;
    virtual FString GetEntityType() const override;

    UFUNCTION(BlueprintCallable)
    void ApplyAutoScalingToMesh();

protected:
    virtual void BeginPlay() override;
};