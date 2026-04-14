// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/UI/CustomizationUI/UW_DropdownOption.h"
#include "Components/Button.h"          // For UButton
#include "Components/TextBlock.h"       // For UTextBlock

void UUW_DropdownOption::NativeConstruct()
{
	Super::NativeConstruct();
	//Btn_DropdownOption->OnHovered.AddDynamic(this, &UUW_DropdownOption::HandleHovered);
}

void UUW_DropdownOption::Init_DropdownOption(FName ID, FText DisplayName, int32 InputOptionIndex, ESlateVisibility ButtonVisibility)
{
	OptionIndex = InputOptionIndex;
	SetItemID(ID);
	SetOptionText(DisplayName);
	SetVisibility(ButtonVisibility);

	Btn_DropdownOption->OnClicked.AddDynamic(this, &UUW_DropdownOption::HandleClicked);
}

void UUW_DropdownOption::SetOptionText(FText DisplayName)
{
	T_OptionName->SetText(DisplayName);
}

void UUW_DropdownOption::SetItemID(FName ID)
{
	ItemID = ID;
}

void UUW_DropdownOption::HandleClicked()
{
	OnOptionClicked.Broadcast(this);
}

/**
void UUW_DropdownOption::HandleHovered()
{
	OnOptionClicked.Broadcast(this);
}
**/

void UUW_DropdownOption::SetButtonSelected()
{
	Btn_DropdownOption->SetColorAndOpacity(FLinearColor::Black);
	Btn_DropdownOption->SetBackgroundColor(FLinearColor::White);
}
