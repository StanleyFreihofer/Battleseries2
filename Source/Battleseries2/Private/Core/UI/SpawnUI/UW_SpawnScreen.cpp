#include "Core/UI/SpawnUI/UW_SpawnScreen.h"
#include "Components/ScrollBox.h"       // For UScrollBox
#include "Components/VerticalBox.h"     // For UVerticalBox
#include "Components/Button.h"          // For UButton
#include "Components/TextBlock.h"       // For UTextBlock
#include "Components/PanelWidget.h"     // Base class for panels

void UUW_SpawnScreen::NativeConstruct()
{
	Super::NativeConstruct();
}

void UUW_SpawnScreen::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
}
