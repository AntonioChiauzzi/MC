#pragma once
#include "UObject/Interface.h"
#include "CoreMinimal.h"
#include "EnvironmentReader.generated.h"

UINTERFACE()
class MC_API UEnvironmentReader : public UInterface
{
    GENERATED_BODY()
};

class MC_API IEnvironmentReader
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintNativeEvent)
    void ReadEnvironment(const class UMyEnvironmentState* Environment);
};