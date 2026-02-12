#pragma once
#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "MCGameInstance.generated.h"

UCLASS(Blueprintable, BlueprintType)

class MC_API UMCGameInstance : public UGameInstance
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadWrite, Category = "Setup")
    FString SavedBaseDataPath;

    UPROPERTY(BlueprintReadWrite, Category = "Setup")
    FString SavedMapPath;

    UPROPERTY(BlueprintReadWrite, Category = "Setup")
    float UserRefreshRate = 60.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Setup")
    FVector UserMapSize = FVector(0.f, 0.f, 0.f);

    UFUNCTION(BlueprintCallable)
    void UploadMap();
};