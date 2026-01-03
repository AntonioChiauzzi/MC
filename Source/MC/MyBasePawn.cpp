#include "MyBasePawn.h"

AMyBasePawn::AMyBasePawn()
{
    PrimaryActorTick.bCanEverTick = true;
}

void AMyBasePawn::BeginPlay()
{
    Super::BeginPlay();

    SpawnLocation = GetActorLocation();
}

void AMyBasePawn::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}