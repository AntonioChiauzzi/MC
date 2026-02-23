#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "WorldStateManager.generated.h"

class UEntityRegistryDataAsset;
class UMyEnvironmentState;
class UResourceManager;
class AActor;
class UWorld;
class ALandscapeProxy;
class ALandscape; 
class AMyBaseActor;

UCLASS()
class MC_API UWorldStateManager : public UObject
{
    GENERATED_BODY()

public:
    void Initialize(UWorld* InWorld);
    void LoadEnvironment();
    void SaveEnvironment() const;
    void LoadEntities();
    void SaveEntities();
    void OpenDirectoryDialogJson();
    void OpenDirectoryDialogMap();
    void RemoveOldInstances();

    UPROPERTY(EditAnywhere)
    FString BaseDataPath;

    UPROPERTY(EditAnywhere, Category = "Registry")
    UEntityRegistryDataAsset* EntityRegistryAsset;

    void ResizeLandscape(const FVector TargetSize);

    void UpdateLandscapeBounds();

    UPROPERTY(VisibleAnywhere, Category = "Map Bounds")
    FVector MinBound;

    UPROPERTY(VisibleAnywhere, Category = "Map Bounds")
    FVector MaxBound;

    FVector WorldOffset = FVector::ZeroVector;

    FVector ApplyWorldOffset_LogicalToWorld(const FVector& LogicalPos) const
    {
        return LogicalPos - WorldOffset;
    }

    FVector ApplyWorldOffset_WorldToLogical(const FVector& WorldPos) const
    {
        return WorldPos + WorldOffset;
    }

    FVector ComputeTargetSizeFromEntitiesJson(float Margin = 0.f) const;

private:
    UWorld* World;

    UPROPERTY()
    TArray<AActor*> SpawnedEntities;

    UPROPERTY()
    UMyEnvironmentState* Environment;

    UPROPERTY()
    UResourceManager* ResourceManager;

protected:
    AActor* SpawnEntityFromJson(const TSharedPtr<FJsonObject>& Obj);

    TSharedPtr<FJsonObject> ConvertEntityToJson(AMyBaseActor* Actor);

    void UpdateEnvironmentFromJson(const TSharedPtr<FJsonObject>& EnvJson);

    TSharedPtr<FJsonObject> ConvertEnvironmentToJson() const;
};