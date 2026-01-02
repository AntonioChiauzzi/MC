#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "WorldStateManager.h"
#include "MyGameMode.generated.h"

UCLASS()
class MC_API AMyGameMode : public AGameMode
{
    GENERATED_BODY()

public:
    virtual void BeginPlay() override;

private:
    UPROPERTY()
    WorldStateManager* WorldManager;

    FTimerHandle SaveTimerHandle;

    void SaveWorld();
};
