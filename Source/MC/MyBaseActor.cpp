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
    SetupCollisionForAllMeshes();
}

void AMyBaseActor::SetupCollisionForAllMeshes()
{
    TArray<UStaticMeshComponent*> MeshComponents;
    GetComponents<UStaticMeshComponent>(MeshComponents);
    for (UStaticMeshComponent* Mesh : MeshComponents)
    {
        if (!Mesh) continue;
        TArray<USceneComponent*> ChildrenComponents;
        Mesh->GetChildrenComponents(false, ChildrenComponents);
        bool bHasBox = false;
        for (USceneComponent* Child : ChildrenComponents)
        {
            if (Cast<UBoxComponent>(Child))
            {
                bHasBox = true;
                break;
            }
        }
        if (bHasBox) continue;
        CreateClickableCollision(Mesh);
    }
}

void AMyBaseActor::CreateClickableCollision(UStaticMeshComponent* Mesh)
{
    if (!Mesh) return;
    UBoxComponent* Box = NewObject<UBoxComponent>(this);
    Box->RegisterComponent();
    Box->AttachToComponent(Mesh, FAttachmentTransformRules::KeepRelativeTransform);
    FVector Origin, Extent;
    Mesh->GetLocalBounds(Origin, Extent);
    Box->SetBoxExtent(Extent);
    Box->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    Box->SetCollisionResponseToAllChannels(ECR_Block);
    Box->SetCollisionObjectType(ECC_WorldDynamic);
    Box->SetGenerateOverlapEvents(true);
    Box->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

    UE_LOG(LogTemp, Warning, TEXT("Collision box creata per mesh: %s"), *Mesh->GetName());
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

FString AMyBaseActor::ToString() const
{
    return FString::Printf(TEXT("ID: %s\nName: %s"), *ID, *Name);
}