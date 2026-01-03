#include "TractorPawn.h"

ATractorPawn::ATractorPawn()
{
    Velocity = FVector(100.f, 0.f, 0.f);
}

void ATractorPawn::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    FVector NewLocation =
        GetActorLocation() + Velocity * DeltaTime;

    SetActorLocation(NewLocation);
}
