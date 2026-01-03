#pragma once

#include "CoreMinimal.h"
#include "MyBasePawn.h"
#include "TractorPawn.generated.h"

UCLASS()
class MC_API ATractorPawn : public AMyBasePawn
{
    GENERATED_BODY()

public:
    ATractorPawn();

protected:
    virtual void Tick(float DeltaTime) override;

    
    UPROPERTY(EditAnywhere)
    FVector Velocity;
};
