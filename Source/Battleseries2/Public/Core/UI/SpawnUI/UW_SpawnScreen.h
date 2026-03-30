#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_SpawnScreen.generated.h"

UCLASS()
class BATTLESERIES2_API UUW_SpawnScreen : public UUserWidget
{
	GENERATED_BODY()

public:
	//loadout box (class selection box in battlefield)
	//spawn button list
	//match status info (conquest ticket progress bar, captured objectives, etc)
	//match info (game mode name, map name, etc)

	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
};