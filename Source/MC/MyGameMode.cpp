#include "MyGameMode.h"
#include "WorldStateManager.h"
#include "EntityRegistryDataAsset.h"
#include "UObject/ConstructorHelpers.h"

AMyGameMode::AMyGameMode()
{
    static ConstructorHelpers::FObjectFinder<UEntityRegistryDataAsset> RegistryAssetObj(TEXT("/Game/Data/DA_EntityRegistry.DA_EntityRegistry"));

    if (RegistryAssetObj.Succeeded())
    {
        RegistryConfig = RegistryAssetObj.Object;
    }
}

void AMyGameMode::BeginPlay()
{
    Super::BeginPlay();

    WorldManager = NewObject<UWorldStateManager>(this);

    if (WorldManager)
    {
        WorldManager->EntityRegistryAsset = RegistryConfig;
        WorldManager->Initialize(GetWorld());
        WorldManager->LoadEnvironment();
        WorldManager->LoadEntities();
    }
    GetWorldTimerManager().SetTimer(
        SaveTimerHandle,
        this,
        &AMyGameMode::SaveWorld,
        1800.0f,
        true
    );
}

void AMyGameMode::SaveWorld()
{
    if (WorldManager)
        WorldManager->SaveEntities();
}