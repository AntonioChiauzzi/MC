#include "SimPlayerController.h"

void ASimPlayerController::BeginPlay()
{
    Super::BeginPlay();

    bShowMouseCursor = false;
    bEnableClickEvents = false;
    bEnableMouseOverEvents = false;

    FInputModeGameOnly Mode;
    SetInputMode(Mode);
}