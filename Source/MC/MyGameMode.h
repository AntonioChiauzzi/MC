#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "WorldStateManager.h"
#include "MyGameMode.generated.h"

class UEntityRegistryDataAsset;

UCLASS(Blueprintable)
class MC_API AMyGameMode : public AGameMode
{
    GENERATED_BODY()

public:

    AMyGameMode();

    virtual void BeginPlay() override;

    UPROPERTY()
    UEntityRegistryDataAsset* RegistryConfig;

private:
    UPROPERTY()
    UWorldStateManager* WorldManager;

    FTimerHandle SaveTimerHandle;

    void SaveWorld();

protected:
    void Load();
    FTimerHandle LoadTimerHandle;
};
