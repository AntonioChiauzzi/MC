#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "MyEnvironmentState.h"
#include "EntityRegistryDataAsset.h"
#include "ResourceManager.h"
#include "WorldStateManager.generated.h"

class UEntityRegistryDataAsset;

UCLASS()
class MC_API UWorldStateManager : public UObject
{
    GENERATED_BODY()

public:

    void Initialize(UWorld* InWorld);

    void LoadEnvironment();
    void SaveEnvironment() const;

    UMyEnvironmentState* GetEnvironment() const;

    void LoadEntities();
    void SaveEntities();

    UPROPERTY(EditAnywhere)
    FString BaseDataPath;

    void OpenDirectoryDialog();

    void RemoveOldInstances();

    UPROPERTY(EditAnywhere, Category = "Registry")
    UEntityRegistryDataAsset* EntityRegistryAsset;

private:
    UWorld* World;

    UPROPERTY()
    TArray<AActor*> SpawnedEntities;

    UPROPERTY()
    UMyEnvironmentState* Environment;

    UPROPERTY()
    class UResourceManager* ResourceManager;

    UPROPERTY(VisibleAnywhere, Category = "Map Bounds")
    FVector MinBound;

    UPROPERTY(VisibleAnywhere, Category = "Map Bounds")
    FVector MaxBound;

};

