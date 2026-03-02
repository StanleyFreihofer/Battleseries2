#include "Core/UI/UW_Customization.h"
#include "Save/SaveSubsystem.h"
#include "Save/PlayerSave_Loadout.h"
#include "Vehicle_Base.h"
#include "Utilities/DataManagerSubsystem.h"
#include "Engine/DataTable.h"
#include "UObject/EnumProperty.h"
#include "Misc/EnumRange.h"
#include "Components/ScrollBox.h"       // For UScrollBox
#include "Components/VerticalBox.h"     // For UVerticalBox
#include "Components/Button.h"          // For UButton
#include "Components/TextBlock.h"       // For UTextBlock
#include "Components/PanelWidget.h"     // Base class for panels
#include "Components/Border.h"
#include "Core/UI/UW_LoadoutSlot.h"
#include "Core/UI/UW_VehicleTypeButton.h"
#include "LoadoutPreviewStage.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Math/UnrealMathUtility.h"
#include "Utilities/HelperFunctions_Vehicle.h"
#include "Data/Runtime/CoreTypes.h"
#include "Core/Weapons/VehicleWeaponLogicComponent.h"
#include "Data/Weapons/VehicleWeapons/Data_VehicleWeapon.h"
#include "Data/Data_Camo.h"
#include "Data/Vehicles/VehicleDefaults.h"

void UUW_Customization::NativeConstruct()
{
	Super::NativeConstruct();
	DataSubsystem = GetGameInstance()->GetSubsystem<UDataManagerSubsystem>();
	CustomizationUIState.CurrentCustomizationMode = ECoreType::Vehicle;

	//Init_Customization();			//dont autostart?
}

void UUW_Customization::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
}

void UUW_Customization::Init_Customization(ALoadoutPreviewStage* InputStageActor)
{
	SetPreviewStageActor(InputStageActor);
	switch (CustomizationUIState.CurrentCustomizationMode)
	{
		case ECoreType::Character:
		case ECoreType::Weapon:
			break;
		case ECoreType::Vehicle:
			Build_VehicleTypeScrollbox();
			CustomizationUIState.TypeEnumIndex = 1;				//set currentvehicle type to 1 so function updateselectedvehicle will run (will only run as intended if currentvehicletype != inputvehicletype)
			RefreshTypeScrollBox();
			UpdateSelectedVehicleType(0);			//default selected vehicle after customization widget is built is first one
			break;
	}
}

void UUW_Customization::SetPreviewStageActor(ALoadoutPreviewStage* InputStageActor)
{
	CustomizationUIState.PreviewStageActor = InputStageActor;
}

void UUW_Customization::Init_TypeScrollBox()
{

}

void UUW_Customization::Build_VehicleTypeScrollbox()				
{
	for (E_VehicleType VehicleType : TEnumRange<E_VehicleType>())
	{
		UUW_VehicleTypeButton* NewVehicleTypeButton = CreateWidget<UUW_VehicleTypeButton>(GetWorld(), VehicleTypeButton);
		NewVehicleTypeButton->SetVehicleType((int32)VehicleType);
		NewVehicleTypeButton->OnVehicleTypeClicked.AddDynamic(this, &UUW_Customization::UpdateSelectedVehicleType);			//bind the delegate 
		ScrollBox->AddChild(NewVehicleTypeButton);
		CustomizationUIState.TypeButtons.Add(NewVehicleTypeButton);
	}
}

void UUW_Customization::RepopulateTypeScrollBox()
{
	ScrollBox->ClearChildren();
	float ScrollBoxWidth = ScrollBox->GetTickSpaceGeometry().GetLocalSize().X;
	if (ScrollBoxWidth <= 0) ScrollBoxWidth = 1920.0f; // Fallback for first frame
	float TargetWidth = ScrollBoxWidth / 3.0f;
	for (UUW_VehicleTypeButton* Btn : CustomizationUIState.TypeButtons)
	{
		ScrollBox->AddChild(Btn);
		Btn->SizeBox->SetWidthOverride(TargetWidth);
	}
}

void UUW_Customization::RefreshTypeScrollBox()
{
	auto& ButtonArray = CustomizationUIState.TypeButtons;

	int32 CurrentPos = -1;
	for (int32 i = 0; i < ButtonArray.Num(); i++)
	{
		if (ButtonArray[i]->TypeEnumIndex == CustomizationUIState.TypeEnumIndex)
		{
			CurrentPos = i;
			break;
		}
	}

	while (CurrentPos != 1)
	{
		if (CurrentPos > 1)
		{
			UUW_VehicleTypeButton* FirstBtn = ButtonArray[0];
			ButtonArray.RemoveAt(0);
			ButtonArray.Add(FirstBtn);
			CurrentPos--;
		}
		else if (CurrentPos < 1)
		{
			UUW_VehicleTypeButton* LastBtn = ButtonArray.Last();
			ButtonArray.RemoveAt(ButtonArray.Num() - 1);
			ButtonArray.Insert(LastBtn, 0);
			CurrentPos++;
		}
	}

	RepopulateTypeScrollBox();
	ButtonArray[1]->SetButtonSelected();
	ScrollBox->SetScrollOffset(0);
}

void UUW_Customization::Build_VehicleLoadoutPanel(int32 TypeEnumIndex)
{
	E_VehicleType VehicleType = static_cast<E_VehicleType>(TypeEnumIndex);
	FName VehicleID = DataSubsystem->GetFirstVehicleIDOfType(VehicleType);
	const FVehicleData& VehicleData = *DataSubsystem->GetVehicleDataRow(VehicleID);

	TMap<int32, FAvailableItems*> CustomizableSeatOptions;			//option list to be built out
	for (int32 SI = 0; SI < VehicleData.Seats.Num(); SI++)
	{
		const FSeatData& SeatData = VehicleData.Seats[SI];
		if (SeatData.SeatRole == E_SeatRole::DriverGunner || SeatData.SeatRole == E_SeatRole::Gunner)
		{
			Build_VehicleLoadoutData_Weapon(SeatData, TypeEnumIndex, SI);
			//optic
			//countermeasure
			//upgrade
		}
	}
	Build_VehicleLoadoutData_Camo(VehicleData, TypeEnumIndex);
}

FCustomizationSlotConfig UUW_Customization::Build_LoadoutSlotData(TArray<FName> ItemIDs, ECoreItemType ItemType, int32 ItemSlot, FText Context)
{
	FCustomizationSlotConfig NewSlotData;
	NewSlotData.AvailableItems = ItemIDs;
	NewSlotData.ItemContext = Context;
	NewSlotData.CoreItemType = ItemType;
	NewSlotData.ItemSlot = ItemSlot;

	return NewSlotData;
}

void UUW_Customization::Build_VehicleLoadoutData_Weapon(const FSeatData& SeatData, int32 TypeEnumIndex, int32 SeatIndex)
{
	//weapons (weapon slots)	
	for (int32 i = 0; i < SeatData.AvailableItems.AvailableWeaponSlots.Num(); i++)
	{
		const FWeaponSlotChoices& WeaponSlot = SeatData.AvailableItems.AvailableWeaponSlots[i];
		if (WeaponSlot.WeaponChoices.Num() > 1)			//if weapon choices > 1, we have multiple options/customization
		{
			TArray<FName> Weapons;
			WeaponSlot.WeaponChoices.GetKeys(Weapons);
			FCustomizationSlotConfig NewSlotData = Build_LoadoutSlotData(Weapons, ECoreItemType::VehicleWeapon, i, SeatData.SeatName);
			UUW_LoadoutSlot* NewSlotWidget = CreateNewLoadoutSlot();
			NewSlotWidget->Init_LoadoutSlot_Vehicle(NewSlotData, SeatIndex);
			LoadCurrentSave_VehicleWeaponSlot(NewSlotWidget, SeatIndex, i, TypeEnumIndex);
		}
	}
}

void UUW_Customization::Build_VehicleLoadoutData_Camo(const FVehicleData& VehicleData, int32 TypeEnumIndex)
{
	TArray<FName> Camos;
	VehicleData.AvailableCamos.GetKeys(Camos);
	FCustomizationSlotConfig NewSlotData = Build_LoadoutSlotData(Camos, ECoreItemType::Camo, -1, FText());
	UUW_LoadoutSlot* NewSlotWidget = CreateNewLoadoutSlot();
	NewSlotWidget->Init_LoadoutSlot_Vehicle(NewSlotData, 0);		//<--THIS SHOULDNT NEED A SEAT INDEX
	LoadCurrentSave_VehicleCamo(NewSlotWidget, TypeEnumIndex);
}

UUW_LoadoutSlot* UUW_Customization::CreateNewLoadoutSlot()
{
	UUW_LoadoutSlot* NewSlotWidget = CreateWidget<UUW_LoadoutSlot>(GetWorld(), LoadoutSlot);
	VerticalBox_LoadoutPanel->AddChild(NewSlotWidget);
	CustomizationUIState.LoadoutSlots.Add(NewSlotWidget);
	NewSlotWidget->OnSlotSelectionChanged.AddDynamic(this, &UUW_Customization::HandleSlotSelectionChanged);
	NewSlotWidget->OnSlotOptionUsed.AddDynamic(this, &UUW_Customization::UpdateDetailsPanel);
	NewSlotWidget->OnSlotDeselected.AddDynamic(this, &UUW_Customization::HideDetailsPanel);
	return NewSlotWidget;
}

void UUW_Customization::LoadCurrentSave_VehicleWeaponSlot(UUW_LoadoutSlot* NewSlotWidget, int32 SeatIndex, int32 WeaponIndex, int32 TypeEnumIndex)
{
	//DONT UPDATE PREVIEW HERE, THIS IS FOR SLOTS ONLY, PREVIEW USES APPLYLOADOUTTOSEAT ON VEHICLE CLASS
	E_VehicleType VehicleType = static_cast<E_VehicleType>(TypeEnumIndex);
	USaveSubsystem* SaveSubsystem = GetGameInstance()->GetSubsystem<USaveSubsystem>();
	FName SavedSelection = NAME_None;
	int32 OptionIndex = 0;
	const FSavedSeatLoadout& SavedSeat = SaveSubsystem->GetSeatLoadout(VehicleType, SeatIndex);

	if (SavedSeat.Weapons.IsValidIndex(WeaponIndex))
	{
		SavedSelection = SavedSeat.Weapons[WeaponIndex];
		OptionIndex = NewSlotWidget->GetOptionIndexFromItemID(SavedSelection);
		NewSlotWidget->UpdateSlotSelection(OptionIndex);
	}
	else            //no save for this weapon slot on this seat of this vehicle type
	{
		UE_LOG(LogTemp, Error, TEXT("[UW_Customization::LoadCurrentSavesForWeaponSlot] Load default options into loadout slot"));
		NewSlotWidget->UpdateSlotSelection(0);
	}
}

void UUW_Customization::LoadCurrentSave_VehicleCamo(UUW_LoadoutSlot* NewSlotWidget, int32 TypeEnumIndex)
{
	E_VehicleType VehicleType = static_cast<E_VehicleType>(TypeEnumIndex);
	USaveSubsystem* SaveSubsystem = GetGameInstance()->GetSubsystem<USaveSubsystem>();
	FName SavedSelection = NAME_None;
	int32 OptionIndex = 0;
	const FPlayerLoadoutConfig_Vehicle& SavedVehicleLoadout = SaveSubsystem->GetVehicleLoadout(VehicleType);

	if (SavedVehicleLoadout.VehicleCamo.IsValid())
	{
		SavedSelection = SavedVehicleLoadout.VehicleCamo;
		OptionIndex = NewSlotWidget->GetOptionIndexFromItemID(SavedSelection);
		NewSlotWidget->UpdateSlotSelection(OptionIndex);
	}
	else
	{
		//NewSlotWidget->UpdateSlotSelection(0);
	}
}

void UUW_Customization::Clear_LoadoutPanel()
{
	VerticalBox_LoadoutPanel->ClearChildren();
	for (UUW_LoadoutSlot* SlotWidget : CustomizationUIState.LoadoutSlots)
	{
		SlotWidget->RemoveFromParent();
	}
	CustomizationUIState.LoadoutSlots.Empty();
}

void UUW_Customization::FadeInOutDetailsPanel(bool FadeOut)
{
	float FadeDuration = 0.75f; //total fade time in sec
	int32 FadeSteps = 30;	//how many steps to divide the fade into
	const float StepTime = FadeDuration / FadeSteps;
	float CurrentStep = FadeOut ? static_cast<float>(FadeSteps) : 0.0f;
	float StepDirection = FadeOut ? -1.0f : 1.0f;
	GetWorld()->GetTimerManager().SetTimer
	(
		DetailsPanel_FadeTimerHandle, [this, CurrentStep, FadeSteps, StepDirection, FadeOut]() mutable
		{
			CurrentStep += StepDirection;
			float Alpha1 = FMath::Clamp(CurrentStep / FadeSteps, 0.0f, 1.0f);
			float Alpha2 = FMath::Clamp(CurrentStep / FadeSteps, 0.0f, 0.2f);
			FLinearColor Color1 = Border_DetailsPanel->GetContentColorAndOpacity();
			FLinearColor Color2 = Border_DetailsPanel->GetBrushColor();
			Color1.A = Alpha1;
			Color2.A = Alpha2;
			Border_DetailsPanel->SetContentColorAndOpacity(Color1);
			Border_DetailsPanel->SetBrushColor(Color2);
			if ((FadeOut && Alpha1 <= 0.0f) || (!FadeOut && Alpha1 >= 1.0f))
			{
				GetWorld()->GetTimerManager().ClearTimer(DetailsPanel_FadeTimerHandle);
			}
		}, StepTime, true
	);
}

void UUW_Customization::UpdateDetailsPanel(FSlotData NewSlotData, int32 OptionIndex)
{
	switch (NewSlotData.SlotConfig.CoreItemType)
	{
		case ECoreItemType::CharacterWeapon:
			break;
		case ECoreItemType::VehicleWeapon:
			FillDetailsPanel_VehicleWeapon(NewSlotData.SlotConfig.AvailableItems[OptionIndex]);
			break;
		case ECoreItemType::Camo:
			FillDetailsPanel_Camo(NewSlotData.SlotConfig.AvailableItems[OptionIndex]);
			break;

	}
	if (Border_DetailsPanel->GetContentColorAndOpacity().A == 0 || Border_DetailsPanel->GetBrushColor().A == 0)
	{
		FadeInOutDetailsPanel(false);
	}
}

void UUW_Customization::HideDetailsPanel()
{
	FadeInOutDetailsPanel(true);
}

void UUW_Customization::FillDetailsPanel_VehicleWeapon(FName WeaponID)
{
	//add contingency for none
	if (WeaponID.IsNone())
	{
		T_ItemName->SetText(FText::FromString("None"));
		return;
	}
	UE_LOG(LogTemp, Error, TEXT("[UW_Customization::FillDetailsPanel_VehicleWeapon] WeaponID = %s"), *WeaponID.ToString());
	const FVehicleWeaponData* VehicleWeaponData = DataSubsystem->GetVehicleWeaponDataRow(WeaponID);
	T_ItemName->SetText(VehicleWeaponData->WeaponData.WeaponClassification.WeaponDisplayName);
}

void UUW_Customization::FillDetailsPanel_Camo(FName CamoID)
{
	const FCamoData* CamoData = DataSubsystem->GetCamoDataRow(CamoID);
	T_ItemName->SetText(CamoData->CamoDisplayName);
	T_Description->SetText(CamoData->CamoDescription);
	Img_Icon->SetBrushFromTexture(CamoData->CamoIcon.LoadSynchronous());
}

void UUW_Customization::UpdateVehiclePreview(int32 TypeEnumIndex)
{
	E_VehicleType VehicleType = static_cast<E_VehicleType>(TypeEnumIndex);
	//if you wanna add functionality for selecting specific vehicle for preview, create function add it here probably
	//What happens when you make a static DataTable in a class ?
		//It becomes one shared copy for the whole game, not per object.
		//You must define it in exactly one.cpp file.
		//Everyone who uses it is really pointing to the same thing.
		//eg. vehicle_base owns vehicle data table
		//and now this ui hooks into vehicle base to access said shit i guess


	//1. clear soontobe previous vehicle's loadout
	//E_VehicleType VehicleType = CustomizationUIState.CustomizationUIState_Vehicle.CurrentVehicleType;
	if (CustomizationUIState.PreviewStageActor->CurrentVehicle)
	{
		CustomizationUIState.PreviewStageActor->CurrentVehicle->ClearEntireLoadoutFromVehicle();
	}

	//create new VehicleInstanceData (id, loadout, preview)

	USaveSubsystem* SaveSubsystem = GetGameInstance()->GetSubsystem<USaveSubsystem>();
	FString TypeName = StaticEnum<E_VehicleType>()->GetNameStringByValue((int64)VehicleType);
	UDataTable* VehicleDataTable = DataSubsystem->GetVehicleDataTable();
	TArray<FName> AllVehicleIDsOfType = FVehicleData::GetRowNamesOfType(VehicleDataTable, VehicleType);
	FName RowName = FName(*StaticEnum<E_VehicleType>()->GetNameStringByValue((int64)VehicleType));

	FVehicleStartingData NewVehicleStartingData;
	NewVehicleStartingData.PreviewVehicle = true;
	NewVehicleStartingData.StartingVehicleLoadout = SaveSubsystem->GetVehicleLoadout(VehicleType);
	NewVehicleStartingData.VehicleID = AllVehicleIDsOfType[0];

	FTransform PreviewTransform = DataSubsystem->GetVehicleDataRow(NewVehicleStartingData.VehicleID)->CustomizationPosition;
	UE_LOG(LogTemp, Warning, TEXT("location: %s"), *PreviewTransform.GetLocation().ToString());

	//CustomizationUIState.PreviewStageActor->CurrentVehicle->VehicleWeaponLogicComponent->OnVWSCleared.AddDynamic(this, &UUW_Customization::UpdateVehiclePreview);
	CustomizationUIState.PreviewStageActor->SetupNewPreviewVehicle(PreviewTransform, NewVehicleStartingData);		//<--default selection
}

void UUW_Customization::UpdateSelectedVehicleType(int32 TypeEnumIndex)
{
	if (TypeEnumIndex != CustomizationUIState.TypeEnumIndex)
	{
		CustomizationUIState.TypeEnumIndex = TypeEnumIndex;
		auto& ButtonArray = CustomizationUIState.TypeButtons;

		Clear_LoadoutPanel();
		Build_VehicleLoadoutPanel(CustomizationUIState.TypeEnumIndex);
		UpdateVehiclePreview(TypeEnumIndex);

		for (UUW_VehicleTypeButton* VehicleTypeButtonInQuestion : CustomizationUIState.TypeButtons)		//make this a function (handle typeselection or some shi)
		{
			if (VehicleTypeButtonInQuestion->TypeEnumIndex != TypeEnumIndex)
			{
				VehicleTypeButtonInQuestion->SetButtonDeselected();
			}
		}

		RefreshTypeScrollBox();
	}
}

void UUW_Customization::HandleSlotSelectionChanged(int32 SeatIndex, FCustomizationSlotConfig SlotConfig, FName SelectedItemID)			//<--shouldnt this just be called update loadout
{
	USaveSubsystem* SaveSubsystem = GetGameInstance()->GetSubsystem<USaveSubsystem>();
	E_VehicleType CurrentVehicleType = static_cast<E_VehicleType>(CustomizationUIState.TypeEnumIndex);
	switch (SlotConfig.CoreItemType)
	{
		case ECoreItemType::VehicleWeapon:
			SaveSubsystem->SetLoadoutWeaponChoice_Vehicle(CurrentVehicleType, SeatIndex, SlotConfig.ItemSlot, SelectedItemID);
			CustomizationUIState.PreviewStageActor->CurrentVehicle->VehicleWeaponLogicComponent->ClearWeaponSlotFromSeat(SeatIndex, SlotConfig.ItemSlot);
			UE_LOG(LogTemp, Error, TEXT("[UW_Customization::HandleSlotSelectionChanged] ApplyLoadoutToSeat will be fired here"));
			CustomizationUIState.PreviewStageActor->CurrentVehicle->VehicleWeaponLogicComponent->ApplyWeaponAtIndexToSeat(SeatIndex, SlotConfig.ItemSlot, SelectedItemID);
			break;
		case ECoreItemType::Camo:
			//need vehicle/weapon/character mode state
			SaveSubsystem->SetLoadoutCamoChoice_Vehicle(CurrentVehicleType, SelectedItemID);
			CustomizationUIState.PreviewStageActor->CurrentVehicle->ApplyCamoToVehicle(SelectedItemID);
			break;
	}
}

FReply UUW_Customization::NativeOnMouseWheel(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	float WheelDelta = InMouseEvent.GetWheelDelta();
	if (SizeBox_ScrollBox->IsHovered())
	{
		auto& ButtonArray = CustomizationUIState.TypeButtons;
		float Delta = InMouseEvent.GetWheelDelta();
		int32 TargetTypeIndex = (Delta > 0) ? ButtonArray[0]->TypeEnumIndex : ButtonArray[2]->TypeEnumIndex;
		UpdateSelectedVehicleType(TargetTypeIndex);
	}
	else
	{
		CustomizationUIState.PreviewStageActor->ZoomPreview(WheelDelta);
	}
	return FReply::Handled();
}

