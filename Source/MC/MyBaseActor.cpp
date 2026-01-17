#include "MyBaseActor.h"

AMyBaseActor::AMyBaseActor()
{
 	PrimaryActorTick.bCanEverTick = true;

}

void AMyBaseActor::BeginPlay()
{
	Super::BeginPlay();

    SpawnLocation = GetActorLocation();
	
}

void AMyBaseActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

FString AMyBaseActor::GetEntityId()
{
    return ID;
}
