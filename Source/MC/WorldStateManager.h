#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "MyEnvironmentState.h"
#include "WorldStateManager.generated.h"

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

private:
    UWorld* World;

    UPROPERTY()
    TArray<AActor*> SpawnedEntities;

    UPROPERTY()
    UMyEnvironmentState* Environment;

    UPROPERTY()
    TMap<FString, TSubclassOf<AActor>> EntityClassRegistry;

    UPROPERTY(VisibleAnywhere, Category = "Map Bounds")
    FVector MinBound;

    UPROPERTY(VisibleAnywhere, Category = "Map Bounds")
    FVector MaxBound;

    void RegisterClasses();
};

