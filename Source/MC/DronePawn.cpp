#include "DronePawn.h"

ADronePawn::ADronePawn()
{
    Velocity = FVector::ZeroVector;
}

void ADronePawn::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    FVector NewLoc =  GetActorLocation() + (Velocity * DeltaTime);
    NewLoc.X = FMath::Clamp(NewLoc.X, MapMin.X, MapMax.X);
    NewLoc.Y = FMath::Clamp(NewLoc.Y, MapMin.Y, MapMax.Y);
    SetActorLocation(NewLoc);
}

void ADronePawn::ConfigureFromJson(const TSharedPtr<FJsonObject> &Json)
{
    if (!Json->HasField(TEXT("params")))
        return;
    auto Params = Json->GetObjectField(TEXT("params"));

    if (!Params->HasField(TEXT("speed")))
        return;
    auto Speed = Params->GetObjectField(TEXT("speed"));

    Velocity = FVector(
        Speed->GetNumberField(TEXT("x")),
        Speed->GetNumberField(TEXT("y")),
        Speed->GetNumberField(TEXT("z")));
}

TSharedPtr<FJsonObject> ADronePawn::GetParametersAsJson()
{
    TSharedPtr<FJsonObject> ParamsJson = MakeShared<FJsonObject>();
    TSharedPtr<FJsonObject> SpeedJson = MakeShared<FJsonObject>();

    SpeedJson->SetNumberField(TEXT("x"), Velocity.X);
    SpeedJson->SetNumberField(TEXT("y"), Velocity.Y);
    SpeedJson->SetNumberField(TEXT("z"), Velocity.Z);

    ParamsJson->SetObjectField(TEXT("speed"), SpeedJson);
    return ParamsJson;
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
