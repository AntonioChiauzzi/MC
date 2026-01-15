#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "EntityRegistryDataAsset.generated.h"

USTRUCT(BlueprintType)
struct FEntityClassMapping
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TSubclassOf<AActor> ActorClass;
};

UCLASS(BlueprintType)
class MC_API UEntityRegistryDataAsset : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Registry")
    TMap<FString, FEntityClassMapping> EntityMappings;
};