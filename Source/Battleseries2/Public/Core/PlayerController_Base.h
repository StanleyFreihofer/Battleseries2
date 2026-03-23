// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Core/UI/UW_Customization.h"
#include "LoadoutPreviewStage.h"
#include "PlayerController_Base.generated.h"

/**
 *	things that are done outside of gameplay (think main menus, customization, etc) should be done here
 * longer lifetime than character
 */

UCLASS()
class BATTLESERIES2_API APlayerController_Base : public APlayerController
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "HUD | Customization")
	void Init_CustomizationScreen();
	UFUNCTION(BlueprintCallable)
	void EnterCustomizationScreen();
	UFUNCTION(BlueprintCallable)
	void ExitCustomizationScreen();

	UFUNCTION(BlueprintCallable)
	UHUDSubsystem* GetHUDSystem();
	UFUNCTION(BlueprintCallable, BlueprintPure)
	UDataManagerSubsystem* GetDataSystem();
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	ALoadoutPreviewStage* PreviewStageActor = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TWeakObjectPtr<AActor> LastViewTarget = nullptr;

	virtual void BeginPlay() override;
};
