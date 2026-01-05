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

// Called every frame
void AMyBaseActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

FString AMyBaseActor::GetEntityId()
{
    return ID;
}
