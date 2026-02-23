#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SimPlayerController.generated.h"

UCLASS()
class MC_API ASimPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    virtual void BeginPlay() override;
};