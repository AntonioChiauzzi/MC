#include "MyGameMode.h"
#include "WorldStateManager.h"

void AMyGameMode::BeginPlay()
{
    Super::BeginPlay();

    WorldManager = NewObject<UWorldStateManager>(this);
    WorldManager->Initialize(GetWorld());

    WorldManager->LoadEnvironment();
    WorldManager->LoadEntities();

    GetWorldTimerManager().SetTimer(
        SaveTimerHandle,
        this,
        &AMyGameMode::SaveWorld,
        5.0f,
        true
    );
}

void AMyGameMode::SaveWorld()
{
    if (WorldManager)
        WorldManager->SaveEntities();
}
