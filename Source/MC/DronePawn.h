#pragma once

#include "CoreMinimal.h"
#include "MyBaseActor.h"
#include "EntityConfigurable.h"
#include "DronePawn.generated.h"

UCLASS()
class MC_API ADronePawn : public AMyBaseActor, public IEntityConfigurable
{
    GENERATED_BODY()

public:
    ADronePawn();

    UPROPERTY()
    FVector Velocity;

    virtual void Tick(float DeltaTime) override;
    virtual void ConfigureFromJson(const TSharedPtr<FJsonObject>& Json) override;

    virtual void SaveToJson(const TSharedPtr<FJsonObject>& Json) override;

    virtual FString GetEntityType() const override;
    TSharedPtr<FJsonObject> GetParametersAsJson();

protected:
};
