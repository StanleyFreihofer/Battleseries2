// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/UI/CustomizationUI/UW_LoadoutSlot.h"
#include "Core/UI/CustomizationUI/UW_DropdownOption.h"
#include "Core/UI/CustomizationUI/UW_Customization.h"
#include "Components/VerticalBox.h"     // For UVerticalBox
#include "Components/Button.h"          // For UButton
#include "Components/TextBlock.h"       // For UTextBlock
#include "Data/Core/CoreTypes.h"
#include "Data/Weapons/Data_InfantryWeapon.h"
#include "Data/Weapons/Data_VehicleWeapon.h"
#include "Data/Data_Optics.h"
#include "Data/Data_Camo.h"
#include "Utilities/DataManagerSubsystem.h"


void UUW_LoadoutSlot::NativeConstruct()
{
	Super::NativeConstruct();
	DataManager = GetGameInstance()->GetSubsystem<UDataManagerSubsystem>();
	SB_SlotHeightOverride_UnHovered = SB_Slot->GetHeightOverride();
}

void UUW_LoadoutSlot::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
}

void UUW_LoadoutSlot::Init_LoadoutSlot_Vehicle(FCustomizationSlotConfig CustomizationSlotData, int32 InputSeatIndex)
{
	SlotData.SlotConfig = CustomizationSlotData;
	SlotData.VehicleSlotData.AssignedSeatIndex = InputSeatIndex;
	SlotData.SlotLabel = SlotData.SlotConfig.BuildSlotLabel();
	T_SlotLabel->SetText(SlotData.SlotLabel);
	for (int32 i = 0; i < SlotData.SlotConfig.AvailableItems.Num(); i++)		
	{
		AddDropdownOption(SlotData.SlotConfig.AvailableItems[i], i);
	}
	Btn_LoadoutSlot->OnHovered.AddDynamic(this, &UUW_LoadoutSlot::HandleSlotHovered);				//hover should be show list of options
	Btn_LoadoutSlot->OnClicked.AddDynamic(this, &UUW_LoadoutSlot::HandleSlotHovered);				//click should be (if customizable) enter into this thing (weapon) to customize
	Btn_LoadoutSlot->OnUnhovered.AddDynamic(this, &UUW_LoadoutSlot::HandleSelectStatus);
}

void UUW_LoadoutSlot::AddDropdownOption(FName NewOptionID, int32 OptionIndex)
{
	UUW_DropdownOption* NewDropdownOption = CreateWidget<UUW_DropdownOption>(GetWorld(), DropdownOption);
	FText DisplayName = FText();
	if (NewOptionID == FName())
	{
		DisplayName = FText::FromString("None");
	}
	if (SlotData.VehicleSlotData.AssignedSeatIndex != -1)
	{
		switch (SlotData.SlotConfig.CoreItemType)
		{
			case ECoreItemType::VehicleWeapon:
				{
					const FVehicleWeaponData* VehicleWeaponData = DataManager->GetVehicleWeaponDataRow(NewOptionID);
					if (VehicleWeaponData)			//if add an early contingency for none, get rid of this check
					{
						DisplayName = VehicleWeaponData->WeaponData.WeaponClassification.WeaponDisplayName;
					}
				}
				break;
			case ECoreItemType::VehicleOptic:
				{
					const FOpticData* OpticData = DataManager->GetOpticDataRow(NewOptionID);
					if (OpticData)
					{
						DisplayName = OpticData->OpticDisplayName;
					}
				}
				break;
			case ECoreItemType::Camo:
				const FCamoData* CamoData = DataManager->GetCamoDataRow(NewOptionID);
				if (CamoData)
				{
					DisplayName = CamoData->CamoDisplayName;
				}
				break;
		}
	}
	else
	{
		switch (SlotData.SlotConfig.CoreItemType)
		{
			case ECoreItemType::CharacterWeapon:
			{
				const FInfantryWeaponData* InfantryWeaponData = DataManager->GetInfantryWeaponDataRow(NewOptionID);
				if (InfantryWeaponData)
				{
					DisplayName = InfantryWeaponData->WeaponClassificationData.BaseWeaponClassificationData.WeaponDisplayName;
				}
				break;
			}
		}
	}
	NewDropdownOption->Btn_DropdownOption->OnHovered.AddDynamic(this, &UUW_LoadoutSlot::UpdateSlot_Hover);		
	NewDropdownOption->Btn_DropdownOption->OnUnhovered.AddDynamic(this, &UUW_LoadoutSlot::HandleSelectStatus);
	NewDropdownOption->OnOptionClicked.AddDynamic(this, &UUW_LoadoutSlot::HandleOptionSelected);	
	NewDropdownOption->Init_DropdownOption(NewOptionID, DisplayName, OptionIndex, ESlateVisibility::Hidden);
	DropdownOptions.Add(NewDropdownOption);
}

void UUW_LoadoutSlot::HandleSlotHovered()
{
	UpdateSlot_Hover();
	OnSlotOptionUsed.Broadcast(SlotData, SlotData.SelectedOptionIndex);
}

void UUW_LoadoutSlot::HandleSlotSelected()
{
	//enters user into customization of whatever selected item in this loadout slot, we are customizing an item within the loadout (2 layers of customization down technically: loadout>item)
	//should only enter into customization if item in this slot can be customized (gun can be customized, grenade or vehicle weapon cant for example)

	switch (SlotData.SlotConfig.CoreItemType)
	{
		case ECoreItemType::CharacterWeapon:
			break;
	}
}

void UUW_LoadoutSlot::HandleOptionSelected(UUW_DropdownOption* ClickedOption)
{
	//when a dropdown option is selected
	UpdateSlotSelection(ClickedOption->OptionIndex);
	OnSlotSelectionChanged.Broadcast(SlotData.VehicleSlotData.AssignedSeatIndex, SlotData.SlotConfig, ClickedOption->ItemID);		//triggers [UW_Customization::HandleSlotSelectionChanged]
	HideOptions();
}

int32 UUW_LoadoutSlot::GetOptionIndexFromItemID(FName SelectedItemID)				//bind dropdown button to this
{
	for (UUW_DropdownOption* OptionInQuestion : DropdownOptions)
	{
		if (OptionInQuestion->ItemID == SelectedItemID)
		{

			return OptionInQuestion->OptionIndex;		//why are wereturning?
		}
	}
	return int32();
}

void UUW_LoadoutSlot::UpdateSlotSelection(int32 OptionIndex)
{
	SlotData.SelectedOptionIndex = OptionIndex;
	if (DropdownOptions.Num() > 0)
	{
		UUW_DropdownOption* OptionInQuestion = DropdownOptions[SlotData.SelectedOptionIndex];
		T_SelectedOptionLabel->SetText(OptionInQuestion->T_OptionName->GetText());
	}
}

void UUW_LoadoutSlot::HideOptions()
{
	for (UUW_DropdownOption* Option : DropdownOptions)
	{
		Option->SetVisibility(ESlateVisibility::Collapsed);
	}
}

bool UUW_LoadoutSlot::ValidateSlotDeselection()
{
	if (Btn_LoadoutSlot->IsHovered())
	{
		return false;
	}
	for (UUW_DropdownOption* Option : DropdownOptions)
	{
		if (Option->Btn_DropdownOption->IsHovered())
		{
			return false;		//something for this slot is selected
		}
	}
	return true;	//nothing in slot is selected	
}

void UUW_LoadoutSlot::UpdateSlot_Hover()
{
	SB_Slot->SetHeightOverride(SB_SlotHeightOverride_UnHovered * 4);
	Img_ItemIcon_Unhover->SetVisibility(ESlateVisibility::Hidden);
	Btn_LoadoutSlot->SetColorAndOpacity(FLinearColor::Black);
	Btn_LoadoutSlot->SetBackgroundColor(FLinearColor::White);
	SB_HoverIcon->SetHeightOverride(400);
	//ShowOptions();
	UE_LOG(LogTemp, Warning, TEXT("Hovered"));
	OnSlotHovered.Broadcast(DropdownOptions, SlotData);
	for (int32 i = 0; i < DropdownOptions.Num(); i++)
	{
		UUW_DropdownOption* Option = DropdownOptions[i];
		if (Option->Btn_DropdownOption->IsHovered())
		{
			OnSlotOptionUsed.Broadcast(SlotData, i);
		}
	}
}

void UUW_LoadoutSlot::UpdateSlot_UnHover()
{
	SB_Slot->SetHeightOverride(SB_SlotHeightOverride_UnHovered);
	Img_ItemIcon_Unhover->SetVisibility(ESlateVisibility::Visible);
	Btn_LoadoutSlot->SetColorAndOpacity(FLinearColor::White);
	Btn_LoadoutSlot->SetBackgroundColor(FLinearColor::Black);
	SB_HoverIcon->SetHeightOverride(0);
	HideOptions();
	UE_LOG(LogTemp, Warning, TEXT("Unhovered"));
	//OnSlotDeselected.Broadcast();
}

void UUW_LoadoutSlot::HandleSelectStatus()		//wrapper function for delegates
{
	if (ValidateSlotDeselection())
	{
		//UpdateSlot_UnHover();
	}
}
