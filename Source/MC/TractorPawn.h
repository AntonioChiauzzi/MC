#pragma once

#include "CoreMinimal.h"
#include "MyBasePawn.h"
#include "EntityConfigurable.h"
#include "TractorPawn.generated.h"

UCLASS()
class MC_API ATractorPawn : public AMyBasePawn,  public IEntityConfigurable
{
    GENERATED_BODY()

public:
    ATractorPawn();

    UPROPERTY()
    FVector Velocity;

    virtual void Tick(float DeltaTime) override;
    virtual void ConfigureFromJson(const TSharedPtr<FJsonObject>& Json) override;
    

protected:

};
