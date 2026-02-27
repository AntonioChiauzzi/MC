#include "MenuHandle.h"
#include "DesktopPlatformModule.h"
#include "IDesktopPlatform.h"
#include "Engine/World.h"
#include "MCGameInstance.h"

bool UMenuHandle::OpenFolderDialog(const UObject* WorldContextObject, FString DialogTitle, FString DefaultPath, FString& SelectedFolder)
{
#if PLATFORM_WINDOWS
    IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
    if (DesktopPlatform)
    {
        void* ParentWindowHandle = nullptr;
        bool bSuccess = DesktopPlatform->OpenDirectoryDialog(
            ParentWindowHandle,
            DialogTitle,
            DefaultPath,
            SelectedFolder
        );
        if (bSuccess)
        {
            if (UMCGameInstance* GI = Cast<UMCGameInstance>(WorldContextObject->GetWorld()->GetGameInstance()))
            {
                GI->SavedBaseDataPath = SelectedFolder;
                UE_LOG(LogTemp, Log, TEXT("SavedJsonPath impostato a: %s"), *GI->SavedBaseDataPath);
            }
        }
        return bSuccess;
    }
#endif

    return false;
}

bool UMenuHandle::OpenFileDialog(const UObject* WorldContextObject, FString DialogTitle, FString DefaultPath, FString FileTypes, FString& SelectedFile)
{
#if PLATFORM_WINDOWS
    IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();

    if (DesktopPlatform)
    {
        void* ParentWindowHandle = nullptr;

        TArray<FString> OutFiles;

        bool bSuccess = DesktopPlatform->OpenFileDialog(
            ParentWindowHandle,
            DialogTitle,
            DefaultPath,
            TEXT(""),
            FileTypes,
            EFileDialogFlags::None,
            OutFiles
        );

        if (bSuccess && OutFiles.Num() > 0)
        {
            SelectedFile = OutFiles[0];
            if (UMCGameInstance* GI = Cast<UMCGameInstance>(WorldContextObject->GetWorld()->GetGameInstance()))
            {
                GI->SavedMapPath = SelectedFile;
                UE_LOG(LogTemp, Log, TEXT("SavedMapPath impostato a: %s"), *GI->SavedMapPath);
            }
            return true;
        }
    }
#endif
    return false;
}

bool UMenuHandle::OpenFileDialogAssets(const UObject* WorldContextObject, FString DialogTitle, FString DefaultPath, FString FileTypes, FString& SelectedFile)
{
#if PLATFORM_WINDOWS
    IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();

    if (DesktopPlatform)
    {
        void* ParentWindowHandle = nullptr;

        TArray<FString> OutFiles;

        bool bSuccess = DesktopPlatform->OpenFileDialog(
            ParentWindowHandle,
            DialogTitle,
            DefaultPath,
            TEXT(""),
            FileTypes,
            EFileDialogFlags::None,
            OutFiles
        );

        if (bSuccess && OutFiles.Num() > 0)
        {
            SelectedFile = OutFiles[0];
            if (UMCGameInstance* GI = Cast<UMCGameInstance>(WorldContextObject->GetWorld()->GetGameInstance()))
            {
                GI->SavedDynamicGltfPath = SelectedFile;
                UE_LOG(LogTemp, Log, TEXT("Asset per il dynamic pawn impostato a: %s"), *GI->SavedDynamicGltfPath);
            }
            return true;
        }
    }
#endif
    return false;
}