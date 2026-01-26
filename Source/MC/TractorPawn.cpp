#include "TractorPawn.h"

ATractorPawn::ATractorPawn()
{
    Velocity = FVector::ZeroVector;
}

void ATractorPawn::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    if (BatteryLevel > 0.0f)
    {
        BatteryLevel = FMath::Max(0.0f, BatteryLevel - (BatteryConsumptionRate * DeltaTime));
        if (BatteryLevel <= 0.0f) { Velocity = FVector::ZeroVector; return; }
        if (!Velocity.IsNearlyZero())
        {
            FRotator TargetRotation = Velocity.Rotation();
            TargetRotation.Pitch = 0.0f;
            TargetRotation.Roll = 0.0f;
            SetActorRotation(FMath::RInterpTo(GetActorRotation(), TargetRotation, DeltaTime, 4.0f));
        }
        FVector NewLoc = GetActorLocation() + (Velocity * DeltaTime);
        if (NewLoc.X < MapMin.X || NewLoc.X > MapMax.X || NewLoc.Y < MapMin.Y || NewLoc.Y > MapMax.Y)
        {
            Velocity = FVector::ZeroVector;
            return;
        }
        SetActorLocation(NewLoc);
    }
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

    (*Params)->TryGetNumberField(TEXT("battery"), BatteryLevel);
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

FString ATractorPawn::GetEntityType() const 
{ 
    return TEXT("Tractor"); 
}
