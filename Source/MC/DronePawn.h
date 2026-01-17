#pragma once

#include "CoreMinimal.h"
#include "MyBaseActor.h"
#include "EntityConfigurable.h"
#include "MyEnvironmentState.h"
#include "EnvironmentInjectable.h"
#include "DronePawn.generated.h"

UCLASS()
class MC_API ADronePawn : public AMyBaseActor, public IEntityConfigurable, public IEnvironmentInjectable
{
    GENERATED_BODY()

public:
    ADronePawn();

    UPROPERTY()
    FVector Velocity;

    UPROPERTY(BlueprintReadWrite)
    float BatteryLevel = 100.0f;

    UPROPERTY(EditAnywhere)
    float BatteryConsumptionRate = 1.0f;

    virtual void Tick(float DeltaTime) override;
    virtual void ConfigureFromJson(const TSharedPtr<FJsonObject>& Json) override;

    virtual void SaveToJson(const TSharedPtr<FJsonObject>& Json) override;

    virtual FString GetEntityType() const override;

    virtual void SetEnvironment_Implementation(UMyEnvironmentState* Environment) override;

    void ReadValueFromSoil();

protected:


private:
    UPROPERTY()
    UMyEnvironmentState* Environment;
};
