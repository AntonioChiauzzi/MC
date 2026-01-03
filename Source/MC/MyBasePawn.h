#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "MyBasePawn.generated.h"

UCLASS()
class MC_API AMyBasePawn : public APawn
{
    GENERATED_BODY()

public:
    AMyBasePawn();

    
    UPROPERTY(VisibleAnywhere, Category="Base Pawn")
    FString PawnID;

    
    UPROPERTY(VisibleAnywhere, Category="Base Pawn")
    FVector SpawnLocation;

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;
};
