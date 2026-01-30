#include "HarvesterPawn.h"

AHarvesterPawn::AHarvesterPawn()
{
    PrimaryActorTick.bCanEverTick = true;
}

void AHarvesterPawn::BeginPlay()
{
    Super::BeginPlay();
    ApplyAutoScalingToMesh();
}

void AHarvesterPawn::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    if (BatteryLevel > 0.0f)
    {
        if (BatteryLevel <= 0.0f)
        {
            Velocity = FVector::ZeroVector;
            return;
        }
        if (TargetLocation.IsSet())
        {
            MoveToTarget(DeltaTime, TargetLocation.GetValue());
            BatteryLevel = FMath::Max(0.0f, BatteryLevel - (BatteryConsumptionRate * DeltaTime));
        }
        else if (!Velocity.IsNearlyZero())
        {
            Move(DeltaTime);
            BatteryLevel = FMath::Max(0.0f, BatteryLevel - (BatteryConsumptionRate * DeltaTime));
        }
        if (!GetVelocity().IsNearlyZero() || (TargetLocation.IsSet() && !Velocity.IsNearlyZero()))
        {
            HarvestCapacity += 0.1f * DeltaTime;
        }
    }
}

void AHarvesterPawn::Move(float DeltaTime)
{
    FRotator TargetRotation = Velocity.Rotation();
    TargetRotation.Pitch = 0.0f;
    TargetRotation.Roll = 0.0f;
    SetActorRotation(FMath::RInterpTo(GetActorRotation(), TargetRotation, DeltaTime, 3.0f));
    FVector NewLoc = GetActorLocation() + (Velocity * DeltaTime);
    if (NewLoc.X < MapMin.X || NewLoc.X > MapMax.X || NewLoc.Y < MapMin.Y || NewLoc.Y > MapMax.Y)
    {
        Velocity = FVector::ZeroVector;
        return;
    }
    SetActorLocation(NewLoc);
}

void AHarvesterPawn::MoveToTarget(float DeltaTime, FVector Target)
{
    FVector CurrentLoc = GetActorLocation();
    float Speed = Velocity.Size();
    if (Speed <= 0.0f) Speed = 250.0f;
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
    SetActorRotation(FMath::RInterpTo(GetActorRotation(), TargetRotation, DeltaTime, 3.0f));
    FVector NewLoc = CurrentLoc + (Direction * Speed * DeltaTime);
    if (NewLoc.X < MapMin.X || NewLoc.X > MapMax.X || NewLoc.Y < MapMin.Y || NewLoc.Y > MapMax.Y)
    {
        Velocity = FVector::ZeroVector;
        TargetLocation.Reset();
        return;
    }
    SetActorLocation(NewLoc);
}

void AHarvesterPawn::SetTargetLocation(TOptional<FVector> NewTarget)
{
    TargetLocation = NewTarget;
}

void AHarvesterPawn::ConfigureFromJson(const TSharedPtr<FJsonObject>& Json)
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
    (*Params)->TryGetNumberField(TEXT("harvestCapacity"), HarvestCapacity);
}

void AHarvesterPawn::SaveToJson(const TSharedPtr<FJsonObject>& Json)
{
    TSharedPtr<FJsonObject> SpeedObj = MakeShared<FJsonObject>();
    SpeedObj->SetNumberField(TEXT("x"), Velocity.X);
    SpeedObj->SetNumberField(TEXT("y"), Velocity.Y);
    SpeedObj->SetNumberField(TEXT("z"), Velocity.Z);

    Json->SetObjectField(TEXT("speed"), SpeedObj);
    Json->SetNumberField(TEXT("battery"), BatteryLevel);
    Json->SetNumberField(TEXT("harvestCapacity"), HarvestCapacity);
}

FString AHarvesterPawn::GetEntityType() const
{
    return TEXT("Harvester");
}

void AHarvesterPawn::ApplyAutoScalingToMesh()
{
    USceneComponent* RootToScale = GetRootComponent();
    FVector Origin, BoxExtent;
    GetActorBounds(true, Origin, BoxExtent);
    FVector TotalSize = BoxExtent * 2.0f;
    if (TotalSize.X > 1.0f)
    {
        float ScaleX = MaxDimensions.X / TotalSize.X;
        float ScaleY = MaxDimensions.Y / TotalSize.Y;
        float ScaleZ = MaxDimensions.Z / TotalSize.Z;
        float UniformScale = FMath::Min3(ScaleX, ScaleY, ScaleZ);
        if (RootToScale)
        {
            RootToScale->SetRelativeScale3D(FVector(UniformScale));
            UE_LOG(LogTemp, Warning, TEXT("Harvester intero scalato a: %f"), UniformScale);
        }
    }
}