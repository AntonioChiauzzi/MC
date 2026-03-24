#pragma once

#include "CoreMinimal.h"
#include "MyBaseActor.h"
#include "EntityConfigurable.h"
#include "MovableVehicle.h"
#include "DynamicPawn.generated.h"

class USceneComponent;
class UStaticMeshComponent;

UCLASS()
class MC_API ADynamicPawn : public AMyBaseActor, public IEntityConfigurable
{
    GENERATED_BODY()

public:
    ADynamicPawn();

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Visuals")
    USceneComponent* SceneRoot;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Visuals")
    UStaticMeshComponent* MeshComponent;

    virtual void Tick(float DeltaTime) override;

    virtual void ConfigureFromJson(const TSharedPtr<FJsonObject>& Json) override;
    virtual void SaveToJson(const TSharedPtr<FJsonObject>& Json) override;
    virtual FString GetEntityType() const override;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
    float BatteryLevel;

    UFUNCTION(BlueprintCallable)
    void ApplyAutoScalingToMesh();

    UFUNCTION(BlueprintCallable)
    void AutoOrientModelFromBounds();

    virtual FString ToString() const override;
protected:
    virtual void BeginPlay() override;
    
};