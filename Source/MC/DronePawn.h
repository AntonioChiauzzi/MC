#pragma once

#include "CoreMinimal.h"
#include "MyBasePawn.h"
#include "EntityConfigurable.h"
#include "DronePawn.generated.h"

UCLASS()
class MC_API ADronePawn : public AMyBasePawn, public IEntityConfigurable
{
    GENERATED_BODY()

public:
    ADronePawn();

    UPROPERTY()
    FVector Velocity;

    virtual void Tick(float DeltaTime) override;
    virtual void ConfigureFromJson(const TSharedPtr<FJsonObject>& Json) override;

protected:
};
