#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MenuHandle.generated.h"

UCLASS()
class MC_API UMenuHandle : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "DirectoryDialog")
	static bool OpenFolderDialog(const UObject* WorldContextObject, FString DialogTitle, FString DefaultPath, FString& SelectedFolder);

	UFUNCTION(BlueprintCallable, Category = "FileDialog")
	static bool OpenFileDialog(const UObject* WorldContextObject, FString DialogTitle, FString DefaultPath, FString FileTypes, FString& SelectedFile);
};
