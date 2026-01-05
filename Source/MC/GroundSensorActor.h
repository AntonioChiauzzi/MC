#pragma once

#include "CoreMinimal.h"
#include "MyBaseActor.h"
#include "GameFramework/Actor.h"
#include "GroundSensorActor.generated.h"

class UMyEnvironmentState;

UCLASS()
class MC_API AGroundSensorActor : public AMyBaseActor
{
    GENERATED_BODY()

public:
    AGroundSensorActor();

    void SetEnvironmentState(UMyEnvironmentState* InEnvironment);

    void ReadSoilData() const;

    virtual FString GetEntityType() const override;

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY()
    UMyEnvironmentState* Environment;
};
