#include "DronePawn.h"

ADronePawn::ADronePawn()
{
    Velocity = FVector::ZeroVector;
}

void ADronePawn::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    if (BatteryLevel > 0.0f)
    {
        BatteryLevel = FMath::Max(0.0f, BatteryLevel - (BatteryConsumptionRate * DeltaTime));
        if (BatteryLevel <= 0.0f) { Velocity = FVector::ZeroVector; return; }
        if (!Velocity.IsNearlyZero())
        {
            FRotator TargetRotation = Velocity.Rotation();
            SetActorRotation(FMath::RInterpTo(GetActorRotation(), TargetRotation, DeltaTime, 5.0f));
        }
        FVector NewLoc = GetActorLocation() + (Velocity * DeltaTime);
        if (NewLoc.X < MapMin.X || NewLoc.X > MapMax.X ||
            NewLoc.Y < MapMin.Y || NewLoc.Y > MapMax.Y ||
            NewLoc.Z < 0.0f || NewLoc.Z > 2000.0f)
        {
            Velocity = FVector::ZeroVector;
            return;
        }
        SetActorLocation(NewLoc);
        ReadValueFromSoil();
    }
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

    (*Params)->TryGetNumberField(TEXT("battery"), BatteryLevel);
}

FString ADronePawn::GetEntityType() const 
{ 
    return TEXT("Drone"); 
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

void ADronePawn::SetEnvironment_Implementation(UMyEnvironmentState* InEnvironment)
{
    Environment = InEnvironment;
    UE_LOG(LogTemp, Log, TEXT("Drone received environment"));
}

void ADronePawn::ReadValueFromSoil()
{
    if (!IsValid(Environment)) return;

    UE_LOG(LogTemp, Log, TEXT("--- Ground Sensor Readings ---"));
    UE_LOG(LogTemp, Log, TEXT("Soil Temp: %f"), Environment->SoilTemperature);
    UE_LOG(LogTemp, Log, TEXT("Soil Humidity: %f"), Environment->SoilHumidity);
    UE_LOG(LogTemp, Log, TEXT("Soil pH: %f"), Environment->SoilpH);

    for (const auto& KVP : Environment->SoilChemicalComposition)
    {
        UE_LOG(LogTemp, Log, TEXT("Chemical %s: %f"), *KVP.Key, KVP.Value);
    }
}
