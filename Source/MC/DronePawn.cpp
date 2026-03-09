#include "DronePawn.h"
#include "MyEnvironmentState.h"
#include "Components/StaticMeshComponent.h"

ADronePawn::ADronePawn()
{
    PrimaryActorTick.bCanEverTick = true;
    Velocity = FVector::ZeroVector;
}

void ADronePawn::BeginPlay()
{
    Super::BeginPlay();
    ApplyAutoScalingToMesh();
    ReadValueFromSoil();
}

void ADronePawn::Tick(float DeltaTime)
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

void ADronePawn::Move(float DeltaTime)
{
    FRotator TargetRotation = Velocity.Rotation();
    SetActorRotation(FMath::RInterpTo(GetActorRotation(), TargetRotation, DeltaTime, 5.0f));
    const FVector NewLocWorld = GetActorLocation() + (Velocity * DeltaTime);
    const FVector NewLocLogical = NewLocWorld + WorldOffset;

    if (NewLocLogical.X < MapMin.X || NewLocLogical.X > MapMax.X ||
        NewLocLogical.Y < MapMin.Y || NewLocLogical.Y > MapMax.Y ||
        NewLocWorld.Z < 0.0f || NewLocWorld.Z > 2000.0f)
    {
        Velocity = FVector::ZeroVector;
        return;
    }
    SetActorLocation(NewLocWorld);
}

void ADronePawn::MoveToTarget(float DeltaTime, FVector Target)
{
    const FVector CurrentLocWorld = GetActorLocation();
    const float Speed = Velocity.Size() > 0.0f ? Velocity.Size() : 400.0f;
    const float Distance = FVector::Dist(CurrentLocWorld, Target);
    if (Distance < FMath::Max(15.0f, Speed * DeltaTime))
    {
        SetActorLocation(Target);
        Velocity = FVector::ZeroVector;
        TargetLocation.Reset();
        return;
    }
    const FVector Direction = (Target - CurrentLocWorld).GetSafeNormal();
    SetActorRotation(FMath::RInterpTo(GetActorRotation(), Direction.Rotation(), DeltaTime, 5.0f));
    const FVector NewLocWorld = CurrentLocWorld + (Direction * Speed * DeltaTime);
    const FVector NewLocLogical = NewLocWorld + WorldOffset;
    if (NewLocLogical.X < MapMin.X || NewLocLogical.X > MapMax.X ||
        NewLocLogical.Y < MapMin.Y || NewLocLogical.Y > MapMax.Y ||
        NewLocWorld.Z < 0.0f || NewLocWorld.Z > 2000.0f)
    {
        Velocity = FVector::ZeroVector;
        TargetLocation.Reset();
        return;
    }
    SetActorLocation(NewLocWorld);
}

void ADronePawn::SetTargetLocation(TOptional<FVector> NewTarget)
{
    TargetLocation = NewTarget;
}

void ADronePawn::ConfigureFromJson(const TSharedPtr<FJsonObject>& Json)
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

void ADronePawn::SaveToJson(const TSharedPtr<FJsonObject>& Json)
{
    TSharedPtr<FJsonObject> SpeedObj = MakeShared<FJsonObject>();
    SpeedObj->SetNumberField(TEXT("x"), Velocity.X);
    SpeedObj->SetNumberField(TEXT("y"), Velocity.Y);
    SpeedObj->SetNumberField(TEXT("z"), Velocity.Z);
    Json->SetObjectField(TEXT("speed"), SpeedObj);
    Json->SetNumberField(TEXT("battery"), BatteryLevel);
}

FString ADronePawn::GetEntityType() const { return TEXT("Drone"); }

void ADronePawn::SetEnvironment_Implementation(UMyEnvironmentState* InEnvironment)
{
    Environment = InEnvironment;
}

void ADronePawn::ReadValueFromSoil()
{
    if (IsValid(Environment))
    {
        UE_LOG(LogTemp, Log, TEXT("Drone Scan - Crop Maturity: %f"), Environment->cropMaturity);
    }
}

void ADronePawn::ApplyAutoScalingToMesh()
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
            }
        }
    }
}