// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_DropdownOption.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnOptionClicked, UUW_DropdownOption*, ClickedOption);
//DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnOptionHovered, UUW_DropdownOption*, HoveredOption);
/**
 * 
 */
UCLASS()
class BATTLESERIES2_API UUW_DropdownOption : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	class UButton* Btn_DropdownOption = nullptr;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* T_OptionName = nullptr;

	UPROPERTY(BlueprintAssignable)
	FOnOptionClicked OnOptionClicked;
	//UPROPERTY(BlueprintAssignable)
	//FOnOptionHovered OnOptionHovered;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	int32 OptionIndex = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ItemID = NAME_None;

	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintCallable)
	void Init_DropdownOption(FName ID, FText DisplayName, int32 InputOptionIndex, ESlateVisibility ButtonVisibility);
	UFUNCTION(BlueprintCallable)
	void SetOptionText(FText DisplayName);
	UFUNCTION(BlueprintCallable)
	void SetItemID(FName ID);
	UFUNCTION(BlueprintCallable)
	void SetButtonSelected();

	UFUNCTION()
	void HandleClicked();
};