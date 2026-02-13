#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UUW_HUD_LockOnIndicator_Base.generated.h"

UCLASS()
class BATTLESERIES2_API UUW_HUD_LockOnIndicator_Base : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	class UImage* LockOnIndicator = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	class UImage* Diamond = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	class UOverlay* IndicatorGroup; 

	UFUNCTION(BlueprintCallable)
	void UpdateIndicatorPosition(FVector Location);

	UFUNCTION(BlueprintCallable)
	void UpdateLockIndicatorStatus(ELockOnState LockOnState);
};

