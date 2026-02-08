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
    void UploadMap();
    void OpenDirectoryDialogJson();
    void OpenDirectoryDialogMap();
    void RemoveOldInstances();

    UPROPERTY(EditAnywhere)
    FString BaseDataPath;

    UPROPERTY(EditAnywhere, Category = "Registry")
    UEntityRegistryDataAsset* EntityRegistryAsset;

private:
    UWorld* World;

    UPROPERTY()
    TArray<AActor*> SpawnedEntities;

    UPROPERTY()
    UMyEnvironmentState* Environment;

    UPROPERTY()
    UResourceManager* ResourceManager;

    UPROPERTY(VisibleAnywhere, Category = "Map Bounds")
    FVector MinBound;

    UPROPERTY(VisibleAnywhere, Category = "Map Bounds")
    FVector MaxBound;
};