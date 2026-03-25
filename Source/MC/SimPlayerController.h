#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SimPlayerController.generated.h"

class UEntityInfoWidget;
class AMyBaseActor;

UCLASS()
class MC_API ASimPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    virtual void BeginPlay() override;
    virtual void SetupInputComponent() override;

protected:
    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UEntityInfoWidget> EntityInfoWidgetClass;

    UPROPERTY()
    UEntityInfoWidget* EntityInfoWidget = nullptr;

    UFUNCTION()
    void OnInspectPressed();

    UFUNCTION()
    void OnToggleMouseModePressed();

    void ShowEntityInfo(AMyBaseActor* SelectedActor);
    void HideEntityInfo();

    void SetGameMouseMode();
    void SetUiMouseMode();

    bool bUiMouseModeEnabled = false;

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UUserWidget> CrosshairWidgetClass;

    UPROPERTY()
    UUserWidget* CrosshairWidget = nullptr;
};