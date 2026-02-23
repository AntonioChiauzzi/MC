#include "FreeCamPawn.h"
#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/FloatingPawnMovement.h"

AFreeCamPawn::AFreeCamPawn()
{
    PrimaryActorTick.bCanEverTick = true;
    Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    SetRootComponent(Root);
    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(Root);
    AutoPossessPlayer = EAutoReceiveInput::Player0; 
}

void AFreeCamPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    check(PlayerInputComponent);
    PlayerInputComponent->BindAxis(TEXT("Move Forward / Backward"), this, &AFreeCamPawn::Input_MoveForward);
    PlayerInputComponent->BindAxis(TEXT("Move Right / Left"), this, &AFreeCamPawn::Input_MoveRight);
    PlayerInputComponent->BindAxis(TEXT("SpectatorMoveUp"), this, &AFreeCamPawn::Input_MoveUp);
    PlayerInputComponent->BindAxis(TEXT("Turn Right / Left Mouse"), this, &AFreeCamPawn::Input_Turn);
    PlayerInputComponent->BindAxis(TEXT("Look Up / Down Mouse"), this, &AFreeCamPawn::Input_LookUp);
    UE_LOG(LogTemp, Verbose, TEXT("Inputs: F=%f R=%f U=%f Turn=%f Look=%f"),
        MoveForward, MoveRight, MoveUp, Turn, LookUp);
}

void AFreeCamPawn::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (Turn != 0.f || LookUp != 0.f)
    {
        FRotator R = GetActorRotation();
        R.Yaw += Turn * LookSpeed;
        R.Pitch = FMath::Clamp(R.Pitch + LookUp * LookSpeed, -89.f, 89.f);
        SetActorRotation(R);
    }
    const FVector Forward = GetActorForwardVector();
    const FVector RightV = GetActorRightVector();
    const FVector UpV = FVector::UpVector;
    FVector Delta = (Forward * MoveForward + RightV * MoveRight + UpV * MoveUp);
    if (!Delta.IsNearlyZero())
    {
        Delta = Delta.GetSafeNormal() * (MoveSpeed * SpeedMultiplier) * DeltaSeconds;
        AddActorWorldOffset(Delta, false);
    }
    Turn = 0.f;
    LookUp = 0.f;
}

void AFreeCamPawn::Input_MoveForward(float V) { MoveForward = V; }
void AFreeCamPawn::Input_MoveRight(float V) { MoveRight = V; }
void AFreeCamPawn::Input_MoveUp(float V) { MoveUp = V; }

void AFreeCamPawn::Input_Turn(float V) { Turn = V; }
void AFreeCamPawn::Input_LookUp(float V) { LookUp = V; }

void AFreeCamPawn::Input_SpeedBoost_Pressed() { SpeedMultiplier = 4.f; }
void AFreeCamPawn::Input_SpeedBoost_Released() { SpeedMultiplier = 1.f; }

