#include "SimPlayerController.h"
#include "EntityInfoWidget.h"
#include "MyBaseActor.h"
#include "Blueprint/UserWidget.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "GameFramework/GameUserSettings.h"
#include "InputCoreTypes.h"

void ASimPlayerController::BeginPlay()
{
    Super::BeginPlay();
    SetGameMouseMode();
    if (EntityInfoWidgetClass)
    {
        EntityInfoWidget = CreateWidget<UEntityInfoWidget>(this, EntityInfoWidgetClass);
        if (EntityInfoWidget)
        {
            EntityInfoWidget->AddToViewport(100);
            EntityInfoWidget->SetPositionInViewport(FVector2D(20.f, 20.f));
            EntityInfoWidget->SetAlignmentInViewport(FVector2D(0.f, 0.f));
            EntityInfoWidget->SetVisibility(ESlateVisibility::Collapsed);
        }
    }
    if (CrosshairWidgetClass)
    {
        CrosshairWidget = CreateWidget<UUserWidget>(this, CrosshairWidgetClass);
        if (CrosshairWidget)
        {
            CrosshairWidget->AddToViewport(9999);
        }
    }
    UE_LOG(LogTemp, Warning, TEXT("SimPlayerController BeginPlay: %s"), *GetClass()->GetName());
}

void ASimPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();
    if (!InputComponent)
    {
        return;
    }
    InputComponent->BindAction(TEXT("InspectSelect"), IE_Pressed, this, &ASimPlayerController::OnInspectPressed);
    InputComponent->BindKey(EKeys::F1, IE_Pressed, this, &ASimPlayerController::OnToggleMouseModePressed);
}

void ASimPlayerController::SetGameMouseMode()
{
    bUiMouseModeEnabled = false;
    bShowMouseCursor = false;
    bEnableClickEvents = true;
    bEnableMouseOverEvents = true;
    FInputModeGameOnly Mode;
    SetInputMode(Mode);
    SetIgnoreLookInput(false);
    SetIgnoreMoveInput(false);
    UE_LOG(LogTemp, Warning, TEXT("Mouse mode: GAME"));
}

void ASimPlayerController::SetUiMouseMode()
{
    bUiMouseModeEnabled = true;
    bShowMouseCursor = true;
    bEnableClickEvents = true;
    bEnableMouseOverEvents = true;
    FInputModeGameAndUI Mode;
    Mode.SetHideCursorDuringCapture(false);
    Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
    SetInputMode(Mode);
    SetIgnoreLookInput(true);
    UE_LOG(LogTemp, Warning, TEXT("Mouse mode: UI"));
}

void ASimPlayerController::OnToggleMouseModePressed()
{
    if (bUiMouseModeEnabled)
    {
        SetGameMouseMode();
    }
    else
    {
        SetUiMouseMode();
    }
}

void ASimPlayerController::OnInspectPressed()
{
    if (bUiMouseModeEnabled)
    {
        return;
    }
    UE_LOG(LogTemp, Warning, TEXT("CLICK DETECTED"));
    int32 SizeX = 0;
    int32 SizeY = 0;
    GetViewportSize(SizeX, SizeY);
    const FVector2D ScreenCenter(SizeX * 0.5f, SizeY * 0.5f);
    FVector WorldLocation;
    FVector WorldDirection;
    if (!DeprojectScreenPositionToWorld(ScreenCenter.X, ScreenCenter.Y, WorldLocation, WorldDirection))
    {
        HideEntityInfo();
        return;
    }
    const FVector Start = WorldLocation;
    const FVector End = Start + (WorldDirection * 100000.f);
    FHitResult Hit;
    FCollisionQueryParams Params(SCENE_QUERY_STAT(InspectTrace), true);
    Params.bReturnPhysicalMaterial = false;
    const bool bHit = GetWorld()->LineTraceSingleByChannel(
        Hit,
        Start,
        End,
        ECC_Visibility,
        Params
    );
    if (!bHit || !Hit.GetActor())
    {
        HideEntityInfo();
        return;
    }
    AMyBaseActor* SelectedActor = Cast<AMyBaseActor>(Hit.GetActor());
    if (!SelectedActor)
    {
        HideEntityInfo();
        return;
    }
    ShowEntityInfo(SelectedActor);
}

void ASimPlayerController::ShowEntityInfo(AMyBaseActor* SelectedActor)
{
    if (!EntityInfoWidget || !SelectedActor)
    {
        UE_LOG(LogTemp, Warning, TEXT("ShowEntityInfo: widget o actor null"));
        return;
    }
    const FString InfoText = SelectedActor->ToString();
    const FVector ActorLocation = SelectedActor->GetActorLocation();
    const FVector2D WidgetPosition(20.f, 20.f);
    EntityInfoWidget->SetPositionInViewport(WidgetPosition);
    EntityInfoWidget->SetVisibility(ESlateVisibility::Visible);
    EntityInfoWidget->SetInfo(InfoText);
}

void ASimPlayerController::HideEntityInfo()
{
    if (EntityInfoWidget)
    {
        EntityInfoWidget->SetVisibility(ESlateVisibility::Collapsed);
    }
}