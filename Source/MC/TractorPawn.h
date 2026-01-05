#pragma once

#include "CoreMinimal.h"
#include "MyBaseActor.h"
#include "EntityConfigurable.h"
#include "TractorPawn.generated.h"

UCLASS()
class MC_API ATractorPawn : public AMyBaseActor,  public IEntityConfigurable
{
    GENERATED_BODY()

public:
    ATractorPawn();

    UPROPERTY()
    FVector Velocity;

    virtual void Tick(float DeltaTime) override;
    virtual void ConfigureFromJson(const TSharedPtr<FJsonObject>& Json) override;
    
    virtual void SaveToJson(const TSharedPtr<FJsonObject>& Json) override;

    virtual FString GetEntityType() const override;

protected:

};
