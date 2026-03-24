#include "EntityInfoWidget.h"
#include "Components/TextBlock.h"

void UEntityInfoWidget::NativeConstruct()
{
    Super::NativeConstruct();
}

void UEntityInfoWidget::SetInfo(const FString& Body)
{
    if (InfoText)
    {
        InfoText->SetText(FText::FromString(Body));
    }
}