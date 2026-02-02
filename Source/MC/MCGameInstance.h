#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "MCGameInstance.generated.h"

UCLASS()
class MC_API UMCGameInstance : public UGameInstance
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadWrite)
    FString SavedBaseDataPath;
};