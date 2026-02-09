#pragma once

#include "CoreMinimal.h"
#include "MyBaseActor.h"
#include "EntityConfigurable.h"
#include "MovableVehicle.h"
#include "TractorPawn.generated.h"

class UStaticMeshComponent;

UCLASS()
class MC_API ATractorPawn : public AMyBaseActor, public IEntityConfigurable, public IMovableVehicle
{
    GENERATED_BODY()

public:
    ATractorPawn();

    UPROPERTY()
    FVector Velocity;

    TOptional<FVector> TargetLocation;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Status")
    float BatteryLevel;

    UPROPERTY(EditAnywhere, Category = "Config")
    float BatteryConsumptionRate = 0.1f;

    virtual void Tick(float DeltaTime) override;

    virtual void SetTargetLocation(TOptional<FVector> NewTarget) override;
    virtual void Move(float DeltaTime) override;
    virtual void MoveToTarget(float DeltaTime, FVector Target) override;

    virtual void ConfigureFromJson(const TSharedPtr<FJsonObject>& Json) override;
    virtual void SaveToJson(const TSharedPtr<FJsonObject>& Json) override;
    virtual FString GetEntityType() const override;

    UPROPERTY(EditAnywhere, Category = "Visuals")
    FVector MaxDimensions = FVector(350.0f, 200.0f, 250.0f);

    void ApplyAutoScalingToMesh();

protected:
    virtual void BeginPlay() override;
};