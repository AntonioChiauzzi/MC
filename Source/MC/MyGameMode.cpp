#include "MyGameMode.h"
#include "WorldStateManager.h"
#include "EntityRegistryDataAsset.h"
#include "UObject/ConstructorHelpers.h"
#include <Kismet/GameplayStatics.h>
#include "WorldStateManager.h"
#include "MCGameInstance.h"

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
    UMCGameInstance* GI = Cast<UMCGameInstance>(GetGameInstance());
    WorldManager = NewObject<UWorldStateManager>(this);

    if (GI && WorldManager)
    {
        SetupWorldManagerPaths(GI);
        WorldManager->BaseDataPath = GI->SavedBaseDataPath;
        WorldManager->EntityRegistryAsset = RegistryConfig;
        WorldManager->Initialize(GetWorld());
        Load();

        GetWorldTimerManager().SetTimer(
            SaveTimerHandle, 
            this, 
            &AMyGameMode::SaveWorld, 
            1800.0f, 
            true);
        GetWorldTimerManager().SetTimer(
            LoadTimerHandle, 
            this, 
            &AMyGameMode::Load, 
            60.0f, 
            true);

        UE_LOG(LogTemp, Log, TEXT("WorldManager inizializzato con successo."));
    }
}

void AMyGameMode::SetupWorldManagerPaths(UMCGameInstance* GI)
{
    if (GI->SavedMapPath.IsEmpty())
    {
        WorldManager->OpenDirectoryDialogMap();
        WorldManager->UploadMap();
    }
    if (GI->SavedBaseDataPath.IsEmpty())
    {
        WorldManager->OpenDirectoryDialogJson();
    }
}

void AMyGameMode::SaveWorld()
{
    if (WorldManager)
        WorldManager->SaveEntities();
}

void AMyGameMode::Load()
{
    if (WorldManager)
    {
        UE_LOG(LogTemp, Warning, TEXT("--- LOAD JSON STARTED ---"));
        WorldManager->RemoveOldInstances();
        WorldManager->LoadEnvironment();
        WorldManager->LoadEntities();
    }
}