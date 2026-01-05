#include "DronePawn.h"

ADronePawn::ADronePawn()
{
    Velocity = FVector::ZeroVector;
}

void ADronePawn::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    SetActorLocation(GetActorLocation() + Velocity * DeltaTime);
}


void ADronePawn::ConfigureFromJson(const TSharedPtr<FJsonObject>& Json)
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
