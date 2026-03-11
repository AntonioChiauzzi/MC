#include "MCGameInstance.h"

#include "HAL/PlatformFileManager.h"
#include "IPlatformFilePak.h"
#include "Misc/Paths.h"
#include "Kismet/GameplayStatics.h"
#include "HAL/FileManager.h"
#include "MyGameMode.h"

static bool CopyPakToSaved(const FString& SourcePakAbs, FString& OutDestPakAbs)
{
    const FString SafeDirAbs = FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir() / TEXT("PaksRuntime/"));
    IFileManager::Get().MakeDirectory(*SafeDirAbs, true);
    const FString FileName = FPaths::GetCleanFilename(SourcePakAbs);
    OutDestPakAbs = SafeDirAbs / FileName;
    const uint32 CopyResult = IFileManager::Get().Copy(*OutDestPakAbs, *SourcePakAbs, true, true);
    return CopyResult == COPY_OK;
}

void UMCGameInstance::UploadMap()
{
    if (SavedMapPath.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("SavedMapPath vuoto"));
        return;
    }
    const FString SourcePakAbs = FPaths::ConvertRelativePathToFull(SavedMapPath);
    UE_LOG(LogTemp, Warning, TEXT("SourcePakAbs: %s"), *SourcePakAbs);
    if (!IFileManager::Get().FileExists(*SourcePakAbs))
    {
        UE_LOG(LogTemp, Error, TEXT("Pak non trovato: %s"), *SourcePakAbs);
        return;
    }
    FString DestPakAbs;
    if (!CopyPakToSaved(SourcePakAbs, DestPakAbs))
    {
        UE_LOG(LogTemp, Error, TEXT("Copia pak fallita verso Saved. Source=%s"), *SourcePakAbs);
        return;
    }
    UE_LOG(LogTemp, Warning, TEXT("Pak copiato in: %s"), *DestPakAbs);
    IPlatformFile& CurrentPlatformFile = FPlatformFileManager::Get().GetPlatformFile();
    FPakPlatformFile* PakPlatformFile = nullptr;
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
    if (!PakPlatformFile->Mount(*DestPakAbs, PakOrder))
    {
        UE_LOG(LogTemp, Error, TEXT("Mount fallito: %s"), *DestPakAbs);
        return;
    }
    UE_LOG(LogTemp, Warning, TEXT("Pak montato: %s"), *DestPakAbs);
    const FString LevelPath = TEXT("/ExternalMaps/MappaEsterna");
    UE_LOG(LogTemp, Warning, TEXT("OpenLevel: %s"), *LevelPath);
    FCoreUObjectDelegates::PostLoadMapWithWorld.RemoveAll(this);
    FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &UMCGameInstance::OnMapLoaded);
    UGameplayStatics::OpenLevel(GetWorld(), FName(TEXT("/ExternalMaps/MappaEsterna")));
}

void UMCGameInstance::OnMapLoaded(UWorld* LoadedWorld)
{
    FCoreUObjectDelegates::PostLoadMapWithWorld.RemoveAll(this);
    UE_LOG(LogTemp, Warning, TEXT("OnDlcMapLoaded: World=%s  Map=%s"),
        *GetNameSafe(LoadedWorld),
        LoadedWorld ? *LoadedWorld->GetMapName() : TEXT("NULL"));
    if (!LoadedWorld)
        return;
    AGameModeBase* GMBase = LoadedWorld->GetAuthGameMode();
    UE_LOG(LogTemp, Warning, TEXT("AuthGameMode: %s"), *GetNameSafe(GMBase));
    AMyGameMode* GM = Cast<AMyGameMode>(GMBase);
    if (!GM)
    {
        UE_LOG(LogTemp, Error, TEXT("GameMode non è MyGameMode. Controlla World Settings della mappa DLC."));
        return;
    }
    LoadedWorld->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(GM, [GM]()
        {
            GM->InitAfterMapReady();
        }));
    APlayerController* PC = UGameplayStatics::GetPlayerController(LoadedWorld, 0);
    if (PC)
    {
        PC->SetInputMode(FInputModeGameOnly());
        PC->bShowMouseCursor = false;
    }
}