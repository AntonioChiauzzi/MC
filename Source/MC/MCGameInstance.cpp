#include "MCGameInstance.h"
#include "HAL/PlatformFileManager.h"
#include "IPlatformFilePak.h"
#include "Misc/PackageName.h"
#include "Kismet/GameplayStatics.h"
#include <AssetRegistry/AssetRegistryModule.h>


void UMCGameInstance::UploadMap()
{
    if (SavedMapPath.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("SavedMapPath vuoto"));
        return;
    }
    FString PakPath = SavedMapPath;
    FPakPlatformFile* PakPlatformFile = nullptr;
    IPlatformFile& CurrentPlatformFile = FPlatformFileManager::Get().GetPlatformFile();
    if (CurrentPlatformFile.GetName() == FString(TEXT("PakFile")))
    {
        PakPlatformFile = static_cast<FPakPlatformFile*>(&CurrentPlatformFile);
    }
    else
    {
        PakPlatformFile = new FPakPlatformFile();
        PakPlatformFile->Initialize(&CurrentPlatformFile, TEXT(""));
        FPlatformFileManager::Get().SetPlatformFile(*PakPlatformFile);
    }
    const int32 PakOrder = 0;
    const FString MountPoint = TEXT("../../../MC/Content/");
    if (!PakPlatformFile->Mount(*PakPath, PakOrder, *MountPoint))
    {
        UE_LOG(LogTemp, Error, TEXT("Mount fallito: %s"), *PakPath);
        return;
    }
    UE_LOG(LogTemp, Log, TEXT("Pak montato: %s"), *PakPath);
    FPackageName::RegisterMountPoint(TEXT("/Game/"), MountPoint);
    {
        FAssetRegistryModule& AssetRegistryModule =
            FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
        AssetRegistryModule.Get().ScanPathsSynchronous({ TEXT("/Game/MappaEsterna") }, true);

        UE_LOG(LogTemp, Log, TEXT("AssetRegistry aggiornato per /Game/MappaEsterna"));
    }
    FString LevelPath = TEXT("/Game/MappaEsterna");
    UE_LOG(LogTemp, Log, TEXT("Tentativo apertura livello: %s"), *LevelPath);
    UGameplayStatics::OpenLevel(GetWorld(), FName(*LevelPath));
}
