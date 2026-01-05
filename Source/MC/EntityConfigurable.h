#pragma once
#include "UObject/Interface.h"
#include "EntityConfigurable.generated.h"

UINTERFACE()
class UEntityConfigurable : public UInterface
{
    GENERATED_BODY()
};

class IEntityConfigurable
{
    GENERATED_BODY()

public:
    virtual void ConfigureFromJson(const TSharedPtr<FJsonObject>& Json) = 0;

    virtual void SaveToJson(const TSharedPtr<FJsonObject>& Json) = 0;
};
