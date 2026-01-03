#include "DronePawn.h"

ADronePawn::ADronePawn()
{
    Velocity = FVector(0.f, 100.f, 50.f);
}

void ADronePawn::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    SetActorLocation(
        GetActorLocation() + Velocity * DeltaTime
    );
}
