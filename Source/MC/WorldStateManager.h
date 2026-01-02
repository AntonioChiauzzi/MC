#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "WorldStateManager.generated.h"

UCLASS()
class MC_API WorldStateManager : public UObject
{
    GENERATED_BODY()

public:
    void Initialize(UWorld* InWorld);

    void LoadEnvironment();
    void LoadEntities();
    void SaveEntities();

private:
    UWorld* World;

    UPROPERTY()
    TArray<AActor*> SpawnedEntities;

    TMap<FString, TSubclassOf<AActor>> ClassRegistry;

    void RegisterClasses();

    void LoadEnvironment();

    void LoadEntities();
    
    void SaveEntities();
};

