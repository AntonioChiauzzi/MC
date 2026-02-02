#include "MyGameMode.h"
#include "WorldStateManager.h"
#include "EntityRegistryDataAsset.h"
#include "UObject/ConstructorHelpers.h"
#include <Kismet/GameplayStatics.h>
#include "../../../../../../../EpicGames/UE_5.7/Engine/Plugins/VirtualProduction/TextureShare/Source/TextureShareCore/Private/Module/TextureShareCoreLogDefines.h"
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
        if (GI->SavedBaseDataPath.IsEmpty())
        {
            WorldManager->OpenDirectoryDialog();
            if (!WorldManager->BaseDataPath.IsEmpty())
            {
                GI->SavedBaseDataPath = WorldManager->BaseDataPath;
                UploadMap();
            }
            return;
        }
        UE_LOG(LogTemp, Log, TEXT("Path recuperato dalla GameInstance: %s"), *GI->SavedBaseDataPath);
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

void AMyGameMode::UploadMap()
{
    UMCGameInstance* GI = Cast<UMCGameInstance>(GetGameInstance());
    if (!GI || GI->SavedBaseDataPath.IsEmpty()) return;
    FString ExternalMapPath = GI->SavedBaseDataPath + TEXT("NewMap.umap");
    FString PackagePath;
    if (FPaths::FileExists(ExternalMapPath))
    {
        if (FPackageName::TryConvertFilenameToLongPackageName(ExternalMapPath, PackagePath))
        {
            UE_LOG(LogTemp, Log, TEXT("Caricamento mappa utente: %s"), *PackagePath);
            UGameplayStatics::OpenLevel(GetWorld(), FName(*PackagePath));
            return;
        }
    }
    UE_LOG(LogTemp, Warning, TEXT("Mappa esterna non trovata o non valida. Carico fallback da /Content/NewMap.umap"));
    UGameplayStatics::OpenLevel(GetWorld(), FName(TEXT("/Game/NewMap")));
}