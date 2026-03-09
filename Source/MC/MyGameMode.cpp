#include "MyGameMode.h"
#include "WorldStateManager.h"
#include "EntityRegistryDataAsset.h"
#include "UObject/ConstructorHelpers.h"
#include <Kismet/GameplayStatics.h>
#include "WorldStateManager.h"
#include "MCGameInstance.h"
#include "FreeCamPawn.h"
#include "SimPlayerController.h"
#include "EngineUtils.h"
#include "Landscape.h"
#include "DrawDebugHelpers.h"

AMyGameMode::AMyGameMode()
{
    PlayerControllerClass = ASimPlayerController::StaticClass();
    DefaultPawnClass = AFreeCamPawn::StaticClass();
    static ConstructorHelpers::FObjectFinder<UEntityRegistryDataAsset> RegistryAssetObj(TEXT("/Game/Data/DA_EntityRegistry.DA_EntityRegistry"));

    if (RegistryAssetObj.Succeeded())
    {
        RegistryConfig = RegistryAssetObj.Object;
    }
}

void AMyGameMode::BeginPlay()
{
    Super::BeginPlay();
    const FString CurrentLevelName = UGameplayStatics::GetCurrentLevelName(this, true);
    const FString WorldName = GetWorld() ? GetWorld()->GetName() : TEXT("NO_WORLD");
    UE_LOG(LogTemp, Warning,
        TEXT("AMyGameMode::BeginPlay() | World=%s | Level=%s | GameMode=%s"),
        *WorldName,
        *CurrentLevelName,
        *GetClass()->GetName());
    InitAfterMapReady();
}

void AMyGameMode::InitAfterMapReady()
{
    UE_LOG(LogTemp, Warning, TEXT("InitAfterMapReady: START"));
    StartPlayerController();
    UMCGameInstance* GI = Cast<UMCGameInstance>(GetGameInstance());
    if (!GI)
    {
        UE_LOG(LogTemp, Error, TEXT("InitAfterMapReady: GameInstance nulla"));
        return;
    }
    if (!WorldManager)
    {
        WorldManager = NewObject<UWorldStateManager>(this);
    }
    if (!WorldManager)
    {
        UE_LOG(LogTemp, Error, TEXT("InitAfterMapReady: impossibile creare WorldManager"));
        return;
    }
    SetupWorldManagerPaths(GI);
    WorldManager->BaseDataPath = GI->SavedBaseDataPath;
    WorldManager->EntityRegistryAsset = RegistryConfig;
    WorldManager->Initialize(GetWorld());
    const FVector FitTarget = WorldManager->ComputeTargetSizeFromEntitiesJson(200.f);
    if (GI->UserMapSize.X > 0.f && GI->UserMapSize.Y > 0.f)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("InitAfterMapReady: uso UserMapSize=%s"),
            *GI->UserMapSize.ToString());

        WorldManager->ResizeLandscape(GI->UserMapSize);
    }
    else if (FitTarget.X > 0.f && FitTarget.Y > 0.f)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("InitAfterMapReady: uso FitTarget=%s"),
            *FitTarget.ToString());

        WorldManager->ResizeLandscape(FitTarget);
    }
    else
    {
        UE_LOG(LogTemp, Warning,
            TEXT("InitAfterMapReady: nessun target valido, aggiorno solo i bounds"));

        WorldManager->UpdateLandscapeBounds();
    }
    GetWorldTimerManager().SetTimerForNextTick(this, &AMyGameMode::AfterResizeNextTick);
    GetWorldTimerManager().SetTimer(SaveTimerHandle, this, &AMyGameMode::SaveWorld, 1800.0f, true);
    UE_LOG(LogTemp, Warning,
        TEXT("InitAfterMapReady: END | BaseDataPath=%s | FitTarget=%s | UserMapSize=%s"),
        *WorldManager->BaseDataPath,
        *FitTarget.ToString(),
        *GI->UserMapSize.ToString());
}

void AMyGameMode::SetupWorldManagerPaths(UMCGameInstance* GI)
{
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
    if (!WorldManager)
    {
        UE_LOG(LogTemp, Error, TEXT("Load: WorldManager nullo"));
        return;
    }
    UE_LOG(LogTemp, Warning,
        TEXT("--- LOAD JSON STARTED --- World=%s Level=%s"),
        *GetWorld()->GetName(),
        *UGameplayStatics::GetCurrentLevelName(this, true));
    WorldManager->RemoveOldInstances();
    WorldManager->LoadEnvironment();
    WorldManager->LoadEntities();
    UE_LOG(LogTemp, Warning, TEXT("--- LOAD JSON FINISHED ---"));
}

void AMyGameMode::ResizeMap(UMCGameInstance* GI) {
    if (GI->UserMapSize.X > 0 && GI->UserMapSize.Y > 0)
    {
        WorldManager->ResizeLandscape(GI->UserMapSize);
    }
    else
    {
        WorldManager->UpdateLandscapeBounds();
    }
}

void AMyGameMode::AfterResizeNextTick()
{
    if (!WorldManager)
        return;
    WorldManager->UpdateLandscapeBounds();
    const FVector Min = WorldManager->MinBound;
    const FVector Max = WorldManager->MaxBound;
    const FVector CenterXYWorld(
        (Min.X + Max.X) * 0.5f,
        (Min.Y + Max.Y) * 0.5f,
        0.f
    );
    const float CamZ = Max.Z + 2000.f;
    const FVector CamLocWorld(CenterXYWorld.X, CenterXYWorld.Y, CamZ);
    if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
    {
        if (APawn* Pawn = PC->GetPawn())
        {
            Pawn->SetActorLocation(CamLocWorld);
            PC->SetControlRotation(FRotator(-60.f, 0.f, 0.f));
        }
    }
    Load();
    if (UMCGameInstance* GI = Cast<UMCGameInstance>(GetGameInstance()))
    {
        const float Interval = (GI->UserRefreshRate <= 0.0f) ? 20.0f : GI->UserRefreshRate;
        GetWorldTimerManager().ClearTimer(LoadTimerHandle);
        GetWorldTimerManager().SetTimer(LoadTimerHandle, this, &AMyGameMode::Load, Interval, true);
    }
}

void AMyGameMode::StartPlayerController()
{
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PC)
    {
        UE_LOG(LogTemp, Error, TEXT("No PlayerController"));
        return;
    }
    PC->SetInputMode(FInputModeGameOnly());
    PC->bShowMouseCursor = false;
    PC->bEnableClickEvents = false;
    PC->bEnableMouseOverEvents = false;
    if (!PC->GetPawn())
    {
        FActorSpawnParameters SP;
        SP.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        AFreeCamPawn* CamPawn = GetWorld()->SpawnActor<AFreeCamPawn>(
            AFreeCamPawn::StaticClass(),
            FVector(0, 0, 2000), FRotator(-60, 0, 0), SP);
        PC->Possess(CamPawn);
        UE_LOG(LogTemp, Warning, TEXT("Spawned+Possessed FreeCamPawn"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Already has Pawn: %s"), *PC->GetPawn()->GetName());
    }
}