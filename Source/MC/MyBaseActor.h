#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "MyBaseActor.generated.h"

UCLASS()
class MC_API AMyBaseActor : public AActor
{
    GENERATED_BODY()

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
    UStaticMeshComponent* BaseMesh;

    AMyBaseActor();

    UPROPERTY(VisibleAnywhere, Category = "Base Actor")
    FString ID;

    UPROPERTY(VisibleAnywhere, Category = "Base Actor")
    FString Name;

    UPROPERTY(VisibleAnywhere, Category = "Base Actor")
    FVector SpawnLocation;

    UPROPERTY(VisibleAnywhere, Category = "Base Actor")
    FVector MapMin;

    UPROPERTY(VisibleAnywhere, Category = "Base Actor")
    FVector MapMax;

    UPROPERTY(VisibleAnywhere, Category = "Base Actor")
    FVector WorldOffset = FVector::ZeroVector;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Assets")
    bool bHasAssetOverride = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Assets")
    FString RuntimeOverrideAssetPath;

    FString GetEntityId();

    FString GetEntityName();

    virtual void Tick(float DeltaTime) override;

    virtual FString GetEntityType() const { return TEXT("BaseEntity"); }

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Assets")
    bool bHasRuntimeOverrideMaxDimensions = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
    FVector MaxDimensions = FVector(0.f, 0.f, 0.f);

    virtual FString ToString() const;

protected:
    virtual void BeginPlay() override;
};
