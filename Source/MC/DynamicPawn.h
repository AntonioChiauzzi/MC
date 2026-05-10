#pragma once

#include "CoreMinimal.h"
#include "MyBaseActor.h"
#include "EntityConfigurable.h"
#include "MovableVehicle.h"
#include "DynamicPawn.generated.h"

class USceneComponent;
class UStaticMeshComponent;

UCLASS()
class MC_API ADynamicPawn : public AMyBaseActor
{
    GENERATED_BODY()

public:
    ADynamicPawn();

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Visuals")
    USceneComponent* SceneRoot;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Visuals")
    UStaticMeshComponent* MeshComponent;

    virtual void Tick(float DeltaTime) override;

    virtual FString GetEntityType() const override;


    UFUNCTION(BlueprintCallable)
    void ApplyAutoScalingToMesh();

    UFUNCTION(BlueprintCallable)
    void AutoOrientModelFromBounds();

    virtual FString ToString() const override;
protected:
    virtual void BeginPlay() override;
    
};