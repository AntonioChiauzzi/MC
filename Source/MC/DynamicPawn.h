#pragma once

#include "CoreMinimal.h"
#include "MyBaseActor.h"
#include "EntityConfigurable.h"
#include "GameFramework/Pawn.h"
#include "DynamicPawn.generated.h"

UCLASS()
class MC_API ADynamicPawn : public AMyBaseActor, public IEntityConfigurable
{
	GENERATED_BODY()

public:
	ADynamicPawn();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	virtual void ConfigureFromJson(const TSharedPtr<FJsonObject>& Json) override;

	virtual void SaveToJson(const TSharedPtr<FJsonObject>& Json) override;

	virtual FString GetEntityType() const override;

};
