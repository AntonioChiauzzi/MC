#include "HarvesterPawn.h"

AHarvesterPawn::AHarvesterPawn() { PrimaryActorTick.bCanEverTick = true; }

void AHarvesterPawn::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    if (BatteryLevel > 0.0f)
    {
        BatteryLevel = FMath::Max(0.0f, BatteryLevel - (BatteryConsumptionRate * DeltaTime));
        if (BatteryLevel <= 0.0f)
        {
            Velocity = FVector::ZeroVector;
            return;
        }

        FVector NewLoc = GetActorLocation() + (Velocity * DeltaTime);
        NewLoc.X = FMath::Clamp(NewLoc.X, MapMin.X, MapMax.X);
        NewLoc.Y = FMath::Clamp(NewLoc.Y, MapMin.Y, MapMax.Y);
        SetActorLocation(NewLoc);

        if (!Velocity.IsNearlyZero())
        {
            HarvestCapacity += 0.1f * DeltaTime;
        }
    }
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

    (*Params)->TryGetNumberField(TEXT("battery"), BatteryLevel);
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