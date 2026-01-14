#pragma once

#include "CoreMinimal.h"
#include "MyBaseActor.h"
#include "EnvironmentReader.h"
#include "GameFramework/Actor.h"
#include "GroundSensorActor.generated.h"


UCLASS()
class MC_API AGroundSensorActor : public AMyBaseActor, public IEnvironmentReader
{
    GENERATED_BODY()

public:
    AGroundSensorActor();

    virtual FString GetEntityType() const override;

    virtual void ReadEnvironment_Implementation(const UMyEnvironmentState* Environment) override;


protected:
    virtual void BeginPlay() override;

private:
};
