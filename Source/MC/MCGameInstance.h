#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "MCGameInstance.generated.h"

UCLASS()
class MC_API UMCGameInstance : public UGameInstance
{
    GENERATED_BODY()

public:
    virtual void Init() override;

    UPROPERTY(BlueprintReadWrite, Category = "Setup")
    FString SavedBaseDataPath = "";

    UPROPERTY(BlueprintReadWrite, Category = "Setup")
    FString SavedMapPath = "";

    UPROPERTY(BlueprintReadWrite, Category = "Setup")
    float UserRefreshRate = 20.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Setup")
    FVector UserMapSize = FVector(7000.f, 7000.f, 0.f);

    UPROPERTY(BlueprintReadWrite, Category = "Setup")
    FString SavedDynamicGltfPath = "";

    UPROPERTY(BlueprintReadOnly, Category = "Setup")
    bool bLaunchConfigLoaded = false;

    UPROPERTY(BlueprintReadOnly, Category = "Setup")
    bool bExternalMapRequested = false;

    UPROPERTY(BlueprintReadOnly, Category = "Setup")
    bool bWorldBootstrapped = false;

    UFUNCTION(BlueprintCallable)
    bool ShouldLoadExternalMap() const;

    UFUNCTION(BlueprintCallable)
    bool HasValidLaunchConfig() const;

    UFUNCTION(BlueprintCallable)
    void UploadMap();

    UFUNCTION()
    void OnMapLoaded(UWorld* LoadedWorld);

private:
    bool LoadRuntimeConfigFromCommandLine();
    void ResetRuntimeDefaults();
};