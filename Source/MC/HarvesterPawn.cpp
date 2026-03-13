#include "HarvesterPawn.h"
#include "Components/StaticMeshComponent.h"

AHarvesterPawn::AHarvesterPawn()
{
    PrimaryActorTick.bCanEverTick = true;
    Velocity = FVector::ZeroVector;
    MaxDimensions = FVector(600.0f, 400.0f, 350.0f);
}

void AHarvesterPawn::BeginPlay()
{
    Super::BeginPlay();
    ApplyAutoScalingToMesh();
}

void AHarvesterPawn::Tick(float DeltaTime)
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
        HarvestCapacity += 0.1f * DeltaTime;
        HarvestCapacity = FMath::Min(HarvestCapacity, 100.0f);
    }
    else if (BatteryLevel <= 0.0f)
    {
        Velocity = FVector::ZeroVector;
    }
}

void AHarvesterPawn::Move(float DeltaTime)
{
    FRotator TargetRotation = Velocity.Rotation();
    TargetRotation.Pitch = 0.0f;
    TargetRotation.Roll = 0.0f;
    SetActorRotation(FMath::RInterpTo(GetActorRotation(), TargetRotation, DeltaTime, 3.0f));
    const FVector NewLocWorld = GetActorLocation() + (Velocity * DeltaTime);
    const FVector NewLocLogical = NewLocWorld + WorldOffset;

    if (NewLocLogical.X < MapMin.X || NewLocLogical.X > MapMax.X ||
        NewLocLogical.Y < MapMin.Y || NewLocLogical.Y > MapMax.Y)
    {
        Velocity = FVector::ZeroVector;
        return;
    }
    SetActorLocation(NewLocWorld);
}

void AHarvesterPawn::MoveToTarget(float DeltaTime, FVector Target)
{
    const FVector CurrentLocWorld = GetActorLocation();
    const float Speed = Velocity.Size() > 0.0f ? Velocity.Size() : 250.0f;
    const float Distance = FVector::Dist2D(CurrentLocWorld, Target);
    if (Distance < FMath::Max(15.0f, Speed * DeltaTime))
    {
        SetActorLocation(FVector(Target.X, Target.Y, CurrentLocWorld.Z));
        Velocity = FVector::ZeroVector;
        TargetLocation.Reset();
        return;
    }
    FVector Direction = (Target - CurrentLocWorld).GetSafeNormal();
    Direction.Z = 0.0f;
    FRotator TargetRotation = Direction.Rotation();
    TargetRotation.Pitch = 0.0f;
    TargetRotation.Roll = 0.0f;
    SetActorRotation(FMath::RInterpTo(GetActorRotation(), TargetRotation, DeltaTime, 3.0f));
    const FVector NewLocWorld = CurrentLocWorld + (Direction * Speed * DeltaTime);
    const FVector NewLocLogical = NewLocWorld + WorldOffset;
    if (NewLocLogical.X < MapMin.X || NewLocLogical.X > MapMax.X ||
        NewLocLogical.Y < MapMin.Y || NewLocLogical.Y > MapMax.Y)
    {
        Velocity = FVector::ZeroVector;
        TargetLocation.Reset();
        return;
    }
    SetActorLocation(NewLocWorld);
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

FString AHarvesterPawn::GetEntityType() const { return TEXT("Harvester"); }

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