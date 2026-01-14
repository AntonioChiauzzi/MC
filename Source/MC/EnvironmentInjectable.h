#pragma once
#include "UObject/Interface.h"
#include "CoreMinimal.h"
#include "EnvironmentInjectable.generated.h"

UINTERFACE()
class MC_API UEnvironmentInjectable : public UInterface
{
    GENERATED_BODY()
};

class MC_API IEnvironmentInjectable
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintNativeEvent)
    void SetEnvironment(class UMyEnvironmentState* Environment);
};