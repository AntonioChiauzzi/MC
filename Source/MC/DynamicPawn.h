#pragma once

#include "CoreMinimal.h"
#include "MyBaseActor.h"
#include "EntityConfigurable.h"
#include "MovableVehicle.h"
#include "DynamicPawn.generated.h"

class UStaticMeshComponent;

UCLASS()
class MC_API ADynamicPawn : public AMyBaseActor, public IEntityConfigurable, public IMovableVehicle
{
    GENERATED_BODY()

public:
    ADynamicPawn();

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Visuals")
    UStaticMeshComponent* MeshComponent;

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    virtual void ConfigureFromJson(const TSharedPtr<FJsonObject>& Json) override;
    virtual void SaveToJson(const TSharedPtr<FJsonObject>& Json) override;
    virtual FString GetEntityType() const override;

    virtual void SetTargetLocation(TOptional<FVector> NewTarget) override;
    virtual void Move(float DeltaTime) override;
    virtual void MoveToTarget(float DeltaTime, FVector Target) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
    float BatteryLevel;

    UPROPERTY(EditAnywhere)
    float BatteryConsumptionRate = 0.5f;

    UPROPERTY()
    FVector Velocity;

    TOptional<FVector> TargetLocation;

    UPROPERTY(EditAnywhere, Category = "Visuals")
    FVector MaxDimensions = FVector(150.0f, 150.0f, 150.0f);

    void ApplyAutoScalingToMesh();
};