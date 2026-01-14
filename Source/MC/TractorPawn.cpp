#include "TractorPawn.h"

ATractorPawn::ATractorPawn()
{
    Velocity = FVector::ZeroVector;
}

void ATractorPawn::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    FVector NewLoc = GetActorLocation() + (Velocity * DeltaTime);
    NewLoc.X = FMath::Clamp(NewLoc.X, MapMin.X, MapMax.X);
    NewLoc.Y = FMath::Clamp(NewLoc.Y, MapMin.Y, MapMax.Y);
    SetActorLocation(NewLoc);
}

void ATractorPawn::ConfigureFromJson(const TSharedPtr<FJsonObject>& Json)
{
    if (!Json->HasField(TEXT("params"))) return;
    auto Params = Json->GetObjectField(TEXT("params"));

    if (!Params->HasField(TEXT("speed"))) return;
    auto Speed = Params->GetObjectField(TEXT("speed"));

    Velocity = FVector(
        Speed->GetNumberField(TEXT("x")),
        Speed->GetNumberField(TEXT("y")),
        Speed->GetNumberField(TEXT("z"))
    );
}

void ATractorPawn::SaveToJson(const TSharedPtr<FJsonObject>& Json)
{
    
    TSharedPtr<FJsonObject> SpeedObj = MakeShared<FJsonObject>();
    
    
    SpeedObj->SetNumberField(TEXT("x"), Velocity.X);
    SpeedObj->SetNumberField(TEXT("y"), Velocity.Y);
    SpeedObj->SetNumberField(TEXT("z"), Velocity.Z);

    
    Json->SetObjectField(TEXT("speed"), SpeedObj);
}

FString ATractorPawn::GetEntityType() const 
{ 
    return TEXT("Tractor"); 
}
