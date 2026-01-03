#pragma once

#include "CoreMinimal.h"
#include "MyBasePawn.h"
#include "DronePawn.generated.h"

UCLASS()
class MC_API ADronePawn : public AMyBasePawn
{
    GENERATED_BODY()

public:
    ADronePawn();

protected:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere)
    FVector Velocity;
};
