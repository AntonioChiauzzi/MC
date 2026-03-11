#include "MyBaseActor.h"

AMyBaseActor::AMyBaseActor()
{
    PrimaryActorTick.bCanEverTick = true;
    BaseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BaseMesh"));
    RootComponent = BaseMesh;
}

void AMyBaseActor::BeginPlay()
{
    Super::BeginPlay();
    SpawnLocation = GetActorLocation();

    TArray<UPrimitiveComponent*> PrimComps;
    GetComponents<UPrimitiveComponent>(PrimComps);
    for (UPrimitiveComponent* C : PrimComps)
    {
        if (C)
        {
            C->SetMobility(EComponentMobility::Movable);
        }
    }
}

void AMyBaseActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

FString AMyBaseActor::GetEntityId()
{
    return ID;
}

FString AMyBaseActor::GetEntityName()
{
    return Name;
}