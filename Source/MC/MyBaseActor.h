#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MyBaseActor.generated.h"

UCLASS()
class MC_API AMyBaseActor : public AActor
{
	GENERATED_BODY()
	
public:	
	AMyBaseActor();

	UPROPERTY(VisibleAnywhere, Category="Base Actor")
    FString ID;

    
    UPROPERTY(VisibleAnywhere, Category="Base Actor")
    FVector SpawnLocation;

    UPROPERTY(VisibleAnywhere, Category = "Base Actor")
    FVector MapMin;

    UPROPERTY(VisibleAnywhere, Category = "Base Actor")
    FVector MapMax;

    FString GetEntityId();

    virtual void Tick(float DeltaTime) override;

    virtual FString GetEntityType() const { return TEXT("BaseEntity"); }

protected:
	virtual void BeginPlay() override;
};

	
