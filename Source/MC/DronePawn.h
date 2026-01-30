#pragma once

#include "CoreMinimal.h"
#include "MyBaseActor.h"
#include "EntityConfigurable.h"
#include "MyEnvironmentState.h"
#include "EnvironmentInjectable.h"
#include "MovableVehicle.h"
#include "DronePawn.generated.h"

UCLASS()
class MC_API ADronePawn : public AMyBaseActor, public IEntityConfigurable, public IEnvironmentInjectable, public IMovableVehicle
{
    GENERATED_BODY()

public:
    ADronePawn();

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

    virtual void SetEnvironment_Implementation(UMyEnvironmentState* Environment) override;

    UPROPERTY(EditAnywhere, Category = "Visuals")
    FVector MaxDimensions = FVector(40.0f, 40.0f, 15.0f);

    void ApplyAutoScalingToMesh();

    void ReadValueFromSoil();


private:
    UPROPERTY()
    UMyEnvironmentState* Environment;

protected:
    virtual void BeginPlay() override;
};
