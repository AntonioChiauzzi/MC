#include "MCGameInstance.h"

#include "HAL/PlatformFileManager.h"
#include "IPlatformFilePak.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
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

void UMCGameInstance::ResetRuntimeDefaults()
{
    SavedBaseDataPath = TEXT("");
    SavedMapPath = TEXT("");
    SavedDynamicGltfPath = TEXT("");
    UserMapSize = FVector(7000.f, 7000.f, 0.f);
    UserRefreshRate = 20.0f;
    bLaunchConfigLoaded = false;
    bExternalMapRequested = false;
}

void UMCGameInstance::Init()
{
    Super::Init();
    ResetRuntimeDefaults();
    bLaunchConfigLoaded = LoadRuntimeConfigFromCommandLine();
    UE_LOG(LogTemp, Warning,
        TEXT("MCGameInstance::Init | ConfigLoaded=%s | ExternalRequested=%s | SavedMapPath=%s"),
        bLaunchConfigLoaded ? TEXT("true") : TEXT("false"),
        bExternalMapRequested ? TEXT("true") : TEXT("false"),
        *SavedMapPath);
}

bool UMCGameInstance::HasValidLaunchConfig() const
{
    return bLaunchConfigLoaded;
}

bool UMCGameInstance::ShouldLoadExternalMap() const
{
    if (!bLaunchConfigLoaded)
    {
        return false;
    }
    if (!bExternalMapRequested)
    {
        return false;
    }
    if (SavedMapPath.IsEmpty())
    {
        return false;
    }
    const FString FullPakPath = FPaths::ConvertRelativePathToFull(SavedMapPath);
    return IFileManager::Get().FileExists(*FullPakPath);
}

bool UMCGameInstance::LoadRuntimeConfigFromCommandLine()
{
    ResetRuntimeDefaults();
    FString ConfigPath;
    if (FParse::Value(FCommandLine::Get(), TEXT("config="), ConfigPath))
    {
        ConfigPath = ConfigPath.TrimQuotes();
        UE_LOG(LogTemp, Warning, TEXT("Config da command line: %s"), *ConfigPath);
    }
    else
    {
        ConfigPath = FPaths::ConvertRelativePathToFull(
            FPaths::ProjectDir() / TEXT("Launcher/runtime_launch_config.json")
        );
        UE_LOG(LogTemp, Warning, TEXT("Config fallback: %s"), *ConfigPath);
    }
    if (ConfigPath.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("Config path vuoto."));
        return false;
    }
    if (!FPaths::FileExists(ConfigPath))
    {
        UE_LOG(LogTemp, Error, TEXT("Runtime config non trovata: %s"), *ConfigPath);
        return false;
    }
    FString JsonText;
    if (!FFileHelper::LoadFileToString(JsonText, *ConfigPath))
    {
        UE_LOG(LogTemp, Error, TEXT("Impossibile leggere runtime config: %s"), *ConfigPath);
        return false;
    }
    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonText);
    if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Runtime config JSON non valida."));
        return false;
    }
    FString Mode;
    if (!JsonObject->TryGetStringField(TEXT("Mode"), Mode) || Mode.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("Runtime config: campo 'Mode' mancante o vuoto."));
        return false;
    }
    JsonObject->TryGetStringField(TEXT("PakPath"), SavedMapPath);
    JsonObject->TryGetStringField(TEXT("JsonPath"), SavedBaseDataPath);
    JsonObject->TryGetStringField(TEXT("AssetOverridesPath"), SavedDynamicGltfPath);
    double MapSizeX = 0.0;
    double MapSizeY = 0.0;
    double RefreshRate = 0.0;
    const bool bHasMapSizeX = JsonObject->TryGetNumberField(TEXT("MapSizeX"), MapSizeX);
    const bool bHasMapSizeY = JsonObject->TryGetNumberField(TEXT("MapSizeY"), MapSizeY);
    const bool bHasRefreshRate = JsonObject->TryGetNumberField(TEXT("RefreshRate"), RefreshRate);
    if (bHasMapSizeX && bHasMapSizeY && MapSizeX > 0.0 && MapSizeY > 0.0)
    {
        UserMapSize = FVector((float)MapSizeX, (float)MapSizeY, 0.f);
    }
    else
    {
        UserMapSize = FVector(7000.f, 7000.f, 0.f);
    }
    if (bHasRefreshRate && RefreshRate > 0.0)
    {
        UserRefreshRate = (float)RefreshRate;
    }
    else
    {
        UserRefreshRate = 20.0f;
    }
    if (Mode.Equals(TEXT("external"), ESearchCase::IgnoreCase))
    {
        bExternalMapRequested = !SavedMapPath.IsEmpty();
        UE_LOG(LogTemp, Warning,
            TEXT("Runtime config caricata | Mode=external | PakPath=%s | JsonPath=%s | AssetOverridesPath=%s | MapSize=%s | RefreshRate=%.2f"),
            *SavedMapPath,
            *SavedBaseDataPath,
            *SavedDynamicGltfPath,
            *UserMapSize.ToString(),
            UserRefreshRate);
        return true;
    }
    if (Mode.Equals(TEXT("default"), ESearchCase::IgnoreCase))
    {
        bExternalMapRequested = false;
        SavedMapPath = TEXT("");
        UE_LOG(LogTemp, Warning,
            TEXT("Runtime config caricata | Mode=default | JsonPath=%s | AssetOverridesPath=%s | MapSize=%s | RefreshRate=%.2f"),
            *SavedBaseDataPath,
            *SavedDynamicGltfPath,
            *UserMapSize.ToString(),
            UserRefreshRate);

        return true;
    }

    UE_LOG(LogTemp, Error, TEXT("Runtime config: Mode non supportato: %s"), *Mode);
    return false;
}

void UMCGameInstance::UploadMap()
{
    UE_LOG(LogTemp, Warning, TEXT("UploadMap CALLED"));
    if (SavedMapPath.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("SavedMapPath vuoto"));
        return;
    }
    const FString SourcePakAbs = FPaths::ConvertRelativePathToFull(SavedMapPath);
    UE_LOG(LogTemp, Warning, TEXT("SourcePakAbs: %s"), *SourcePakAbs);
    if (!IFileManager::Get().FileExists(*SourcePakAbs))
    {
        UE_LOG(LogTemp, Error, TEXT("Pak non trovato"));
        return;
    }
    FString DestPakAbs;
    if (!CopyPakToSaved(SourcePakAbs, DestPakAbs))
    {
        UE_LOG(LogTemp, Error, TEXT("Copia pak fallita"));
        return;
    }
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
    if (!PakPlatformFile->Mount(*DestPakAbs, 0))
    {
        UE_LOG(LogTemp, Error, TEXT("Mount fallito"));
        return;
    }
    UE_LOG(LogTemp, Warning, TEXT("Pak montato: %s"), *DestPakAbs);
    FCoreUObjectDelegates::PostLoadMapWithWorld.RemoveAll(this);
    FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &UMCGameInstance::OnMapLoaded);
    UGameplayStatics::OpenLevel(GetWorld(), FName(TEXT("/ExternalMaps/MappaEsterna")));
}

void UMCGameInstance::OnMapLoaded(UWorld* LoadedWorld)
{
    UE_LOG(LogTemp, Warning, TEXT("OnMapLoaded TRIGGERED"));
    FCoreUObjectDelegates::PostLoadMapWithWorld.RemoveAll(this);
    if (!LoadedWorld)
        return;
    UE_LOG(LogTemp, Warning, TEXT("OnMapLoaded: %s"), *LoadedWorld->GetName());
    AMyGameMode* GM = Cast<AMyGameMode>(LoadedWorld->GetAuthGameMode());
    if (!GM)
    {
        UE_LOG(LogTemp, Error, TEXT("GameMode non valido"));
        return;
    }
    LoadedWorld->GetTimerManager().SetTimerForNextTick(
        FTimerDelegate::CreateWeakLambda(GM, [GM]()
            {
                UE_LOG(LogTemp, Warning, TEXT("InitAfterMapReady chiamato da GameInstance"));
                GM->InitAfterMapReady();
            })
    );
    if (APlayerController* PC = UGameplayStatics::GetPlayerController(LoadedWorld, 0))
    {
        PC->SetInputMode(FInputModeGameOnly());
        PC->bShowMouseCursor = false;
    }
}