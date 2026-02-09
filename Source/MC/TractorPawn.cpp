#include "TractorPawn.h"
#include "Components/StaticMeshComponent.h"

ATractorPawn::ATractorPawn()
{
    PrimaryActorTick.bCanEverTick = true;
    Velocity = FVector::ZeroVector;
}

void ATractorPawn::BeginPlay()
{
    Super::BeginPlay();
    ApplyAutoScalingToMesh();
}

void ATractorPawn::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    if (BatteryLevel > 0.0f && !Velocity.IsNearlyZero())
    {
        if (TargetLocation.IsSet())
        {
            MoveToTarget(DeltaTime, TargetLocation.GetValue());
        }
        else
        {
            Move(DeltaTime);
        }
        BatteryLevel = FMath::Max(0.0f, BatteryLevel - (BatteryConsumptionRate * DeltaTime));
    }
    else if (BatteryLevel <= 0.0f)
    {
        Velocity = FVector::ZeroVector;
    }
}

void ATractorPawn::Move(float DeltaTime)
{
    FRotator TargetRotation = Velocity.Rotation();
    TargetRotation.Pitch = 0.0f;
    TargetRotation.Roll = 0.0f;
    SetActorRotation(FMath::RInterpTo(GetActorRotation(), TargetRotation, DeltaTime, 4.0f));
    FVector NewLoc = GetActorLocation() + (Velocity * DeltaTime);
    if (NewLoc.X < MapMin.X || NewLoc.X > MapMax.X || NewLoc.Y < MapMin.Y || NewLoc.Y > MapMax.Y)
    {
        Velocity = FVector::ZeroVector;
        return;
    }
    SetActorLocation(NewLoc);
}

void ATractorPawn::MoveToTarget(float DeltaTime, FVector Target)
{
    FVector CurrentLoc = GetActorLocation();
    float Speed = Velocity.Size() > 0.0f ? Velocity.Size() : 300.0f;
    float Distance = FVector::Dist2D(CurrentLoc, Target);
    if (Distance < FMath::Max(15.0f, Speed * DeltaTime))
    {
        SetActorLocation(FVector(Target.X, Target.Y, CurrentLoc.Z));
        Velocity = FVector::ZeroVector;
        TargetLocation.Reset();
        return;
    }
    FVector Direction = (Target - CurrentLoc).GetSafeNormal();
    Direction.Z = 0.0f;
    FRotator TargetRotation = Direction.Rotation();
    TargetRotation.Pitch = 0.0f;
    TargetRotation.Roll = 0.0f;
    SetActorRotation(FMath::RInterpTo(GetActorRotation(), TargetRotation, DeltaTime, 4.0f));
    FVector NewLoc = CurrentLoc + (Direction * Speed * DeltaTime);
    if (NewLoc.X < MapMin.X || NewLoc.X > MapMax.X || NewLoc.Y < MapMin.Y || NewLoc.Y > MapMax.Y)
    {
        Velocity = FVector::ZeroVector;
        TargetLocation.Reset();
        return;
    }
    SetActorLocation(NewLoc);
}

void ATractorPawn::SetTargetLocation(TOptional<FVector> NewTarget)
{
    TargetLocation = NewTarget;
}

void ATractorPawn::ConfigureFromJson(const TSharedPtr<FJsonObject>& Json)
{
    const TSharedPtr<FJsonObject>* Params;
    if (!Json->TryGetObjectField(TEXT("params"), Params)) return;
    const TSharedPtr<FJsonObject>* SpeedObj;
    if ((*Params)->TryGetObjectField(TEXT("speed"), SpeedObj))
    {
        Velocity.X = (*SpeedObj)->GetNumberField(TEXT("x"));
        Velocity.Y = (*SpeedObj)->GetNumberField(TEXT("y"));
        Velocity.Z = (*SpeedObj)->GetNumberField(TEXT("z"));
    }
    if (!(*Params)->TryGetNumberField(TEXT("battery"), BatteryLevel))
    {
        BatteryLevel = 100.0f;
    }
}

void ATractorPawn::SaveToJson(const TSharedPtr<FJsonObject>& Json)
{
    TSharedPtr<FJsonObject> SpeedObj = MakeShared<FJsonObject>();
    SpeedObj->SetNumberField(TEXT("x"), Velocity.X);
    SpeedObj->SetNumberField(TEXT("y"), Velocity.Y);
    SpeedObj->SetNumberField(TEXT("z"), Velocity.Z);
    Json->SetObjectField(TEXT("speed"), SpeedObj);
    Json->SetNumberField(TEXT("battery"), BatteryLevel);
}

FString ATractorPawn::GetEntityType() const { return TEXT("Tractor"); }

void ATractorPawn::ApplyAutoScalingToMesh()
{
    TArray<UStaticMeshComponent*> MeshComps;
    GetComponents<UStaticMeshComponent>(MeshComps);
    for (UStaticMeshComponent* CurrentMesh : MeshComps)
    {
        if (CurrentMesh && CurrentMesh->GetStaticMesh())
        {
            FVector RawSize = CurrentMesh->GetStaticMesh()->GetBoundingBox().GetSize();
            if (RawSize.X > 1.0f)
            {
                float ScaleX = MaxDimensions.X / RawSize.X;
                float ScaleY = MaxDimensions.Y / RawSize.Y;
                float ScaleZ = MaxDimensions.Z / RawSize.Z;
                float UniformScale = FMath::Min3(ScaleX, ScaleY, ScaleZ);

                CurrentMesh->SetRelativeScale3D(FVector(UniformScale));
                CurrentMesh->UpdateBounds();
            }
        }
    }
}