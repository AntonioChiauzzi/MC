#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "MyGameMode.generated.h"

class UEntityRegistryDataAsset;
class UWorldStateManager;
class UMCGameInstance;

UCLASS(Blueprintable)
class MC_API AMyGameMode : public AGameMode
{
    GENERATED_BODY()

public:

    AMyGameMode();

    virtual void BeginPlay() override;

    UPROPERTY()
    UEntityRegistryDataAsset* RegistryConfig;

    UFUNCTION()
    void InitAfterMapReady();

    UFUNCTION()
    void StartPlayerController();

    UFUNCTION()
    void AfterResizeNextTick();

private:
    UPROPERTY()
    UWorldStateManager* WorldManager;

    FTimerHandle SaveTimerHandle;

    void SaveWorld();

    void SetupWorldManagerPaths(UMCGameInstance* GI);

    void ResizeMap(UMCGameInstance* GI);

protected:
    void Load();
    FTimerHandle LoadTimerHandle;
};
