#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GroundSensorActor.generated.h"

class UMyEnvironmentState;

UCLASS()
class MC_API AGroundSensorActor : public AActor
{
    GENERATED_BODY()

public:
    AGroundSensorActor();

    void SetEnvironmentState(UMyEnvironmentState* InEnvironment);

    void ReadSoilData() const;

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY()
    UMyEnvironmentState* Environment;
};
