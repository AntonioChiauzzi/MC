#include "MCGameInstance.h"
#include "HAL/PlatformFileManager.h"
#include "IPlatformFilePak.h"
#include "Misc/PackageName.h"
#include "Kismet/GameplayStatics.h"


void UMCGameInstance::UploadMap()
{
    if (SavedMapPath.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("SavedMapPath vuoto"));
        return;
    }
    FString PakPath = SavedMapPath;
    FPakPlatformFile* PakPlatformFile = nullptr;
    IPlatformFile& CurrentPlatformFile =
        FPlatformFileManager::Get().GetPlatformFile();
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
    if (!PakPlatformFile->Mount(*PakPath, PakOrder))
    {
        UE_LOG(LogTemp, Error, TEXT("Mount fallito: %s"), *PakPath);
        return;
    }
    UE_LOG(LogTemp, Log, TEXT("Pak montato: %s"), *PakPath);
    FPakFile* PakFile = new FPakFile(PakPlatformFile, *PakPath, false);
    FString PakMountPoint = PakFile->GetMountPoint();
    FPaths::MakeStandardFilename(PakMountPoint);
    UE_LOG(LogTemp, Log, TEXT("Mount point pak: %s"), *PakMountPoint);
    FPackageName::RegisterMountPoint(TEXT("/Game/"), PakMountPoint);
    FString LevelPath = TEXT("/Game/MappaEsterna");

    UE_LOG(LogTemp, Log, TEXT("Tentativo apertura livello: %s"), *LevelPath);

    UGameplayStatics::OpenLevel(
        GetWorld(),
        FName(*LevelPath)
    );
}