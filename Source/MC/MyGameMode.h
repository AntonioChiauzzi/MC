#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "WorldStateManager.h"
#include "MyGameMode.generated.h"

UCLASS(Blueprintable)
class MC_API AMyGameMode : public AGameMode
{
    GENERATED_BODY()

public:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere)
    UEntityRegistryDataAsset* RegistryConfig;

private:
    UPROPERTY()
    UWorldStateManager* WorldManager;

    FTimerHandle SaveTimerHandle;

    void SaveWorld();
};
