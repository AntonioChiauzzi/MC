#pragma once

#include "CoreMinimal.h"
#include "MyBaseActor.h"
#include "EntityConfigurable.h"
#include "MovableVehicle.h"
#include "DynamicPawn.generated.h"

class UStaticMeshComponent;

UCLASS()
class MC_API ADynamicPawn : public AMyBaseActor, public IEntityConfigurable
{
    GENERATED_BODY()

public:
    ADynamicPawn();

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Visuals")
    UStaticMeshComponent* MeshComponent;

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    virtual void ConfigureFromJson(const TSharedPtr<FJsonObject>& Json) override;
    virtual void SaveToJson(const TSharedPtr<FJsonObject>& Json) override;
    virtual FString GetEntityType() const override;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
    float BatteryLevel;


    UPROPERTY(EditAnywhere, Category = "Visuals")
    FVector MaxDimensions = FVector(150.0f, 150.0f, 150.0f);

    UFUNCTION(BlueprintCallable)
    void ApplyAutoScalingToMesh();
};