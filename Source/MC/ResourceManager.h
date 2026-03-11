#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "MyEnvironmentState.h"
#include "ResourceManager.generated.h"

UCLASS(BlueprintType)
class MC_API UResourceManager : public UObject
{
    GENERATED_BODY()

public:
    UPROPERTY()
    UMyEnvironmentState* EnvState;

    UPROPERTY()
    TArray<AActor*> ManagedEntities;

    void EvaluateBusinessRules();

private:
    void DispatchDrone(AActor* Drone);
    void DispatchTractor(AActor* Tractor);
};