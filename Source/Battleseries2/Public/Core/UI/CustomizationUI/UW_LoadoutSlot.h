// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Engine/DataTable.h"
#include "Components/SizeBox.h"
#include "Components/Image.h"
#include "Data/Runtime/CoreTypes.h"
#include "Utilities/DataManagerSubsystem.h"
#include "UW_LoadoutSlot.generated.h"

struct FVehicleWeaponData;
class UUW_DropdownOption;
class UDataManagerSubsystem;

/**
 * 
 */

USTRUCT(BlueprintType)		//defines what kind of customization it is and what options are available for it
struct FCustomizationSlotConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText ItemContext = FText::GetEmpty();
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ItemSlot = -1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)			//at the core, what type of thing does this slot play a part in defining/customizaing
	ECoreItemType CoreItemType = ECoreItemType::CharacterWeapon;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)			//what are options are available
	TArray<FName> AvailableItems;

	FText BuildSlotLabel() const
	{
		FString Label;
		//context
		if (!ItemContext.IsEmpty())
		{
			Label += ItemContext.ToString();
			Label += TEXT(" ");
		}
		//slot
		if (ItemSlot == 0)
		{
			Label += TEXT("Primary");
			Label += TEXT(" ");
		}
		if (ItemSlot == 1)
		{
			Label += TEXT("Secondary");
			Label += TEXT(" ");
		}
		switch (CoreItemType)
		{
			case ECoreItemType::CharacterWeapon:
			case ECoreItemType::VehicleWeapon:
				Label += TEXT("Weapon");
				break;
			case ECoreItemType::CharacterGadget:
				Label += TEXT("Gadget");
				break;
			case ECoreItemType::VehicleOptic:
				Label += TEXT("Optic");
				break;
			case ECoreItemType::VehicleCountermeasure:
				Label += TEXT("Countermeasure");
				break;
			case ECoreItemType::VehicleUpgrade:
				Label += TEXT("Upgrade");
				break;
			case ECoreItemType::Camo:
				Label += TEXT("Camo");
				break;
		}

		return FText::FromString(Label);
	}
};

USTRUCT(BlueprintType)
struct FSlotData_Vehicle
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	int32 AssignedSeatIndex = -1;
};

USTRUCT(BlueprintType)
struct FSlotData
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	int32 SlotIndex = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FText SlotLabel = FText::GetEmpty();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SelectedOptionIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	struct FCustomizationSlotConfig SlotConfig = FCustomizationSlotConfig();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	struct FSlotData_Vehicle VehicleSlotData = FSlotData_Vehicle();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TArray<UUW_DropdownOption*> DropdownOptions;

};


DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnSlotSelectionChanged, int32, SeatIndex, FCustomizationSlotConfig, SlotConfig, FName, SelectedID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSlotOptionUsed, FSlotData, SlotData, int32, OptionIndex);		
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSlotDeselected);

UCLASS()
class BATTLESERIES2_API UUW_LoadoutSlot : public UUserWidget
{
	GENERATED_BODY()

public:

	//Designer Components
	UPROPERTY(meta = (BindWidget))
	class UButton* Btn_LoadoutSlot = nullptr;
	UPROPERTY(meta = (BindWidget))
	class USizeBox* SB_Slot = nullptr;
	UPROPERTY(meta = (BindWidget))
	class USizeBox* SB_HoverIcon = nullptr;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* T_SlotLabel = nullptr;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* T_SelectedOptionLabel = nullptr;
	UPROPERTY(meta = (BindWidget))
	class UImage* Img_ItemIcon_Unhover = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	class UVerticalBox* VerticalBox_OptionsList = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FSlotData SlotData = FSlotData();

	UPROPERTY(BlueprintAssignable)
	FOnSlotSelectionChanged OnSlotSelectionChanged;
	UPROPERTY(BlueprintAssignable)
	FOnSlotOptionUsed OnSlotOptionUsed;
	UPROPERTY(BlueprintAssignable)
	FOnSlotDeselected OnSlotDeselected;

	UPROPERTY(BlueprintReadWrite)
	float SB_SlotHeightOverride_UnHovered = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UUW_DropdownOption> DropdownOption;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TArray<UUW_DropdownOption*> DropdownOptions;




	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UFUNCTION(BlueprintCallable)
	void Init_LoadoutSlot_Vehicle(FCustomizationSlotConfig CustomizationSlotData, int32 InputSeatIndex);
	UFUNCTION(BlueprintCallable)
	void AddDropdownOption(FName NewOptionID, int32 OptionIndex);

	UFUNCTION(BlueprintCallable)
	void HandleSlotSelected();
	UFUNCTION(BlueprintCallable)
	void HandleOptionSelected(UUW_DropdownOption* ClickedOption);

	UFUNCTION(BlueprintCallable)
	int32 GetOptionIndexFromItemID(FName SelectedItemID);

	UFUNCTION(BlueprintCallable)
	void UpdateSlotSelection(int32 OptionIndex);
	UFUNCTION(BlueprintCallable)
	void ShowOptions();
	UFUNCTION(BlueprintCallable)
	void HideOptions();
	UFUNCTION(BlueprintCallable)
	bool ValidateSlotDeselection();
	UFUNCTION(BlueprintCallable)
	void UpdateSlot_Hover();
	UFUNCTION(BlueprintCallable)
	void UpdateSlot_Unhover();
	UFUNCTION(BlueprintCallable)
	void HandleHoverStatus();

private:
	UDataManagerSubsystem* DataManager;
	

};
