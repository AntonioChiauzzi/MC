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
    TRefCountPtr<FPakFile> PakFile = new FPakFile(PakPlatformFile, *PakPath, false);
    FString PakMountPoint = PakFile->GetMountPoint();
    if (FPaths::IsRelative(PakMountPoint)) {
        FString PakDirectory = FPaths::GetPath(PakPath);
        PakMountPoint = FPaths::Combine(PakDirectory, PakMountPoint);
        FPaths::CollapseRelativeDirectories(PakMountPoint);
    }
    FPaths::MakeStandardFilename(PakMountPoint);
    if (!PakMountPoint.EndsWith(TEXT("/"))) PakMountPoint += TEXT("/");
    FPackageName::RegisterMountPoint(TEXT("/Game/"), PakMountPoint);
    FString LevelPath = TEXT("MappaEsterna");
    UGameplayStatics::OpenLevel(
        GetWorld(),
        FName(*LevelPath)
    );
}

