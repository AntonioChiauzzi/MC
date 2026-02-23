#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "FreeCamPawn.generated.h"

UCLASS()
class MC_API AFreeCamPawn : public APawn
{
    GENERATED_BODY()

public:
    AFreeCamPawn();

    virtual void Tick(float DeltaSeconds) override;
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

private:
    UPROPERTY()
    class USceneComponent* Root;

    UPROPERTY()
    class UCameraComponent* Camera;

    float MoveForward = 0.f;
    float MoveRight = 0.f;
    float MoveUp = 0.f;

    float Turn = 0.f;
    float LookUp = 0.f;

    UPROPERTY(EditAnywhere, Category = "FreeCam")
    float MoveSpeed = 2000.f;

    UPROPERTY(EditAnywhere, Category = "FreeCam")
    float LookSpeed = 2.f;

    void Input_MoveForward(float V);
    void Input_MoveRight(float V);
    void Input_MoveUp(float V);

    void Input_Turn(float V);
    void Input_LookUp(float V);

    void Input_SpeedBoost_Pressed();
    void Input_SpeedBoost_Released();

    float SpeedMultiplier = 1.f;
};