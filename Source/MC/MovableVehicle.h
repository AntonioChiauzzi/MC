#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "MovableVehicle.generated.h"


UINTERFACE(MinimalAPI)
class UMovableVehicle : public UInterface { 
    GENERATED_BODY() 
};


class MC_API IMovableVehicle {
    GENERATED_BODY()

public:
    
    virtual void SetTargetLocation(TOptional<FVector> NewTarget) = 0;

    
    virtual void Move(float DeltaTime) = 0;

    
    virtual void MoveToTarget(float DeltaTime, FVector Target) = 0;
};