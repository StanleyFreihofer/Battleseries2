#include "Core/UI/CustomizationUI/UW_Customization.h"
#include "Save/SaveSubsystem.h"
#include "Save/PlayerSave_Loadout.h"
#include "Vehicle_Base.h"
#include "Utilities/DataManagerSubsystem.h"
#include "Utilities/GameInstance_Base.h"
#include "Engine/DataTable.h"
#include "UObject/EnumProperty.h"
#include "Misc/EnumRange.h"
#include "Components/ScrollBox.h"       // For UScrollBox
#include "Components/VerticalBox.h"     // For UVerticalBox
#include "Components/Button.h"          // For UButton
#include "Components/TextBlock.h"       // For UTextBlock
#include "Components/PanelWidget.h"     // Base class for panels
#include "Components/Border.h"
#include "Core/UI/CustomizationUI/UW_LoadoutSlot.h"
#include "Core/UI/CustomizationUI/UW_LoadoutTypeButton.h"
#include "LoadoutPreviewStage.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Math/UnrealMathUtility.h"
#include "Utilities/HelperFunctions_Vehicle.h"
#include "Data/Runtime/CoreTypes.h"
#include "Core/Weapons/VehicleWeaponLogicComponent.h"
#include "Core/PlayerController_Base.h"
#include "Data/Weapons/VehicleWeapons/Data_VehicleWeapon.h"
#include "Data/Data_Optics.h"
#include "Data/Data_Camo.h"
#include "Data/Vehicles/VehicleDefaults.h"

/**
* currently alot of similar/same state stored on both the customization menu and the loadout stage... merge into 1 system?
* possibly loading ui class with too much non-UI logic?
**/

void UUW_Customization::NativeConstruct()
{
	Super::NativeConstruct();

	PC = Cast<APlayerController_Base>(GetOwningPlayer());
	CustomizationUIState.CurrentCustomizationMode = ECoreType::Vehicle;							//change this to be from customization default data

	Btn_VehicleMode->OnClicked.AddDynamic(this, &UUW_Customization::OnVehicleModeBtnClicked);
	Btn_WeaponMode->OnClicked.AddDynamic(this, &UUW_Customization::OnWeaponModeBtnClicked);
	Btn_ExitCustomization->OnClicked.AddDynamic(this, &UUW_Customization::ExitCustomization);

	//Init_Customization();			//dont autostart?
}

void UUW_Customization::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
}

void UUW_Customization::Init_Customization(ALoadoutPreviewStage* InputStageActor)
{
	//called on enter of customization
	switch (CustomizationUIState.CurrentCustomizationMode)
	{
		case ECoreType::Character:
		case ECoreType::Weapon:
			EnterLoadoutMode(true, false);
			break;
		case ECoreType::Vehicle:
			EnterVehicleMode(true, false);
			break;
	}
}

void UUW_Customization::EnterVehicleMode(bool OverrideCheck, bool BlendView)
{
	if (!OverrideCheck)		//used to ensure hitting the same button repeatedly wont result in reinitialization, entervehiclemode logic should only occur on enter into customization for first time or enter from other customization mode
	{
		if (CustomizationUIState.CurrentCustomizationMode == ECoreType::Vehicle)
		{
			return;
		}
	}

	ClearPreviousMode();
	
	CustomizationUIState.CurrentCustomizationMode = ECoreType::Vehicle;
	Build_VehicleTypeScrollbox();
	CustomizationUIState.TypeEnumIndex = 1;				//set currentvehicle type to 1 so function updateselectedvehicle will run (will only run as intended if currentvehicletype != inputvehicletype)

	PC->PreviewStageActor->PreviewStageState.CurrentStageMode = ECoreType::Vehicle;		//make function on stage side
	if (BlendView)
	{
		PC->SetViewTargetWithBlend(PC->PreviewStageActor->GetCurrentPreviewCameraActor(), 2.0f);
	}
	else
	{
		PC->SetViewTarget(PC->PreviewStageActor->GetCurrentPreviewCameraActor());
	}

	UpdateSelectedVehicleType(0);

	/**
	GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
	{
		//RefreshTypeScrollBox();
		UpdateSelectedVehicleType(0);
	});
	**/
}

void UUW_Customization::EnterLoadoutMode(bool OverrideCheck, bool BlendView)
{
	//character loadout (or kits)
	ClearPreviousMode();

	CustomizationUIState.CurrentCustomizationMode = ECoreType::Class;
	PC->PreviewStageActor->PreviewStageState.CurrentStageMode = ECoreType::Class;

	if (BlendView)
	{
		PC->SetViewTargetWithBlend(PC->PreviewStageActor->GetCurrentPreviewCameraActor(), 2.0f);
	}
	else
	{
		PC->SetViewTarget(PC->PreviewStageActor->GetCurrentPreviewCameraActor());
	}
}

void UUW_Customization::EnterItemMode()
{
	//enter into mode that allows 1 to customize an item within a loadout (guns/character weapon for example)
}

void UUW_Customization::ClearPreviousMode()
{
	//clear type buttons
	for (UUW_LoadoutTypeButton* TypeButton : CustomizationUIState.TypeButtons)
	{
		TypeButton->RemoveFromParent();
	}
	CustomizationUIState.TypeButtons.Empty();

	Clear_LoadoutPanel();
	ScrollBox->ClearChildren();
}

void UUW_Customization::Init_TypeScrollBox()
{

}

void UUW_Customization::Build_VehicleTypeScrollbox()				
{
	for (E_VehicleType VehicleType : TEnumRange<E_VehicleType>())		//change to data from customization defaults
	{
		UUW_LoadoutTypeButton* NewVehicleTypeButton = CreateWidget<UUW_LoadoutTypeButton>(GetWorld(), VehicleTypeButton);
		NewVehicleTypeButton->SetVehicleType((int32)VehicleType);
		NewVehicleTypeButton->OnLoadoutTypeClicked.AddDynamic(this, &UUW_Customization::UpdateSelectedVehicleType);			
		ScrollBox->AddChild(NewVehicleTypeButton);
		CustomizationUIState.TypeButtons.Add(NewVehicleTypeButton);
	}
}

void UUW_Customization::RepopulateTypeScrollBox()
{
	ScrollBox->ClearChildren();
	//float ScrollBoxWidth = 1920.0f;//ScrollBox->GetTickSpaceGeometry().GetLocalSize().X;
	//if (ScrollBoxWidth <= 0) ScrollBoxWidth = 1920.0f; // Fallback for first frame
	//float TargetWidth = ScrollBoxWidth / 3.0f;
	for (UUW_LoadoutTypeButton* Btn : CustomizationUIState.TypeButtons)
	{
		ScrollBox->AddChild(Btn);
		Btn->SizeBox->SetWidthOverride(360.0f);
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
			UUW_LoadoutTypeButton* FirstBtn = ButtonArray[0];
			ButtonArray.RemoveAt(0);
			ButtonArray.Add(FirstBtn);
			CurrentPos--;
		}
		else if (CurrentPos < 1)
		{
			UUW_LoadoutTypeButton* LastBtn = ButtonArray.Last();
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
	FName VehicleID = GetData_UUWCustomization()->GetFirstVehicleIDOfType(VehicleType);
	const FVehicleData& VehicleData = *GetData_UUWCustomization()->GetVehicleDataRow(VehicleID);

	TMap<int32, FAvailableItems*> CustomizableSeatOptions;			//option list to be built out
	for (int32 SI = 0; SI < VehicleData.Seats.Num(); SI++)
	{
		const FSeatData& SeatData = VehicleData.Seats[SI];
		if (SeatData.SeatRole == E_SeatRole::DriverGunner || SeatData.SeatRole == E_SeatRole::Gunner)
		{
			Build_VehicleLoadoutData_Weapon(SeatData, TypeEnumIndex, SI);
			Build_VehicleLoadoutData_Optic(SeatData, TypeEnumIndex, SI);
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

void UUW_Customization::Build_VehicleLoadoutData_Optic(const FSeatData& SeatData, int32 TypeEnumIndex, int32 SeatIndex)
{
	if (SeatData.AvailableItems.AvailableOptics.Num() > 1)
	{
		TArray<FName> Optics;
		for (const FName& Optic : SeatData.AvailableItems.AvailableOptics)
		{
			Optics.Add(Optic);
		}
		FCustomizationSlotConfig NewSlotData = Build_LoadoutSlotData(Optics, ECoreItemType::VehicleOptic, -1, SeatData.SeatName);
		UUW_LoadoutSlot* NewSlotWidget = CreateNewLoadoutSlot();
		NewSlotWidget->Init_LoadoutSlot_Vehicle(NewSlotData, SeatIndex);
		LoadCurrentSave_VehicleOptic(NewSlotWidget, SeatIndex, TypeEnumIndex);
	}
}

void UUW_Customization::Build_VehicleLoadoutData_Camo(const FVehicleData& VehicleData, int32 TypeEnumIndex)
{
	if (VehicleData.AvailableCamos.Num() > 1)
	{
		TArray<FName> Camos;
		VehicleData.AvailableCamos.GetKeys(Camos);
		FCustomizationSlotConfig NewSlotData = Build_LoadoutSlotData(Camos, ECoreItemType::Camo, -1, FText());
		UUW_LoadoutSlot* NewSlotWidget = CreateNewLoadoutSlot();
		NewSlotWidget->Init_LoadoutSlot_Vehicle(NewSlotData, 0);		//<--THIS SHOULDNT NEED A SEAT INDEX
		LoadCurrentSave_VehicleCamo(NewSlotWidget, TypeEnumIndex);
	}
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

void UUW_Customization::LoadCurrentSave_VehicleOptic(UUW_LoadoutSlot* NewSlotWidget, int32 SeatIndex, int32 TypeEnumIndex)
{
	E_VehicleType VehicleType = static_cast<E_VehicleType>(TypeEnumIndex);
	USaveSubsystem* SaveSubsystem = GetGameInstance()->GetSubsystem<USaveSubsystem>();
	FName SavedSelection = NAME_None;
	int32 OptionIndex = 0;
	const FSavedSeatLoadout& SavedSeat = SaveSubsystem->GetSeatLoadout(VehicleType, SeatIndex);
	if (!SavedSeat.Optic.IsNone())
	{
		SavedSelection = SavedSeat.Optic;
		OptionIndex = NewSlotWidget->GetOptionIndexFromItemID(SavedSelection);
		NewSlotWidget->UpdateSlotSelection(OptionIndex);
	}
	else
	{
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
		case ECoreItemType::VehicleOptic:
			FillDetailsPanel_Optic(NewSlotData.SlotConfig.AvailableItems[OptionIndex]);
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
	if (WeaponID.IsNone())
	{
		T_ItemName->SetText(FText::FromString("None"));
		return;
	}
	UE_LOG(LogTemp, Error, TEXT("[UW_Customization::FillDetailsPanel_VehicleWeapon] WeaponID = %s"), *WeaponID.ToString());
	const FVehicleWeaponData* VehicleWeaponData = GetData_UUWCustomization()->GetVehicleWeaponDataRow(WeaponID);
	T_ItemName->SetText(VehicleWeaponData->WeaponData.WeaponClassification.WeaponDisplayName);
	T_Description->SetText(VehicleWeaponData->WeaponData.WeaponClassification.WeaponDescription);
}

void UUW_Customization::FillDetailsPanel_Optic(FName OpticID)
{
	const FOpticData* OpticData = GetData_UUWCustomization()->GetOpticDataRow(OpticID);
	T_ItemName->SetText(OpticData->OpticDisplayName);
	T_Description->SetText(OpticData->OpticDescription);
}

void UUW_Customization::FillDetailsPanel_Camo(FName CamoID)
{
	const FCamoData* CamoData = GetData_UUWCustomization()->GetCamoDataRow(CamoID);
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
	
	if (PC->PreviewStageActor->PreviewStageState.CurrentVehicle)
	{
		PC->PreviewStageActor->PreviewStageState.CurrentVehicle->ClearEntireLoadoutFromVehicle();
	}

	//create new VehicleInstanceData (id, loadout, preview)

	USaveSubsystem* SaveSubsystem = GetGameInstance()->GetSubsystem<USaveSubsystem>();
	FString TypeName = StaticEnum<E_VehicleType>()->GetNameStringByValue((int64)VehicleType);
	UDataTable* VehicleDataTable = GetData_UUWCustomization()->GetVehicleDataTable();
	TArray<FName> AllVehicleIDsOfType = FVehicleData::GetRowNamesOfType(VehicleDataTable, VehicleType);
	FName RowName = FName(*StaticEnum<E_VehicleType>()->GetNameStringByValue((int64)VehicleType));

	FVehicleStartingData NewVehicleStartingData;
	NewVehicleStartingData.PreviewVehicle = true;
	NewVehicleStartingData.StartingVehicleLoadout = SaveSubsystem->GetVehicleLoadout(VehicleType);
	NewVehicleStartingData.VehicleID = AllVehicleIDsOfType[0];

	FTransform PreviewTransform = GetData_UUWCustomization()->GetVehicleDataRow(NewVehicleStartingData.VehicleID)->CustomizationPosition;
	UE_LOG(LogTemp, Warning, TEXT("location: %s"), *PreviewTransform.GetLocation().ToString());

	PC->PreviewStageActor->SetupNewPreviewVehicle(PreviewTransform, NewVehicleStartingData);		//<--default selection
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

		for (UUW_LoadoutTypeButton* VehicleTypeButtonInQuestion : CustomizationUIState.TypeButtons)		//make this a function (handle typeselection or some shi)
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
			PC->PreviewStageActor->PreviewStageState.CurrentVehicle->VehicleWeaponLogicComponent->ClearWeaponSlotFromSeat(SeatIndex, SlotConfig.ItemSlot);
			UE_LOG(LogTemp, Error, TEXT("[UW_Customization::HandleSlotSelectionChanged] ApplyLoadoutToSeat will be fired here"));
			PC->PreviewStageActor->PreviewStageState.CurrentVehicle->VehicleWeaponLogicComponent->ApplyWeaponAtIndexToSeat(SeatIndex, SlotConfig.ItemSlot, SelectedItemID);
			break;
		case ECoreItemType::VehicleOptic:
			SaveSubsystem->SetLoadoutOpticChoice_Vehicle(CurrentVehicleType, SeatIndex, SelectedItemID);
			break;
		case ECoreItemType::Camo:
			SaveSubsystem->SetLoadoutCamoChoice_Vehicle(CurrentVehicleType, SelectedItemID);
			PC->PreviewStageActor->PreviewStageState.CurrentVehicle->ApplyCamoToVehicle(SelectedItemID);
			break;
	}
}

void UUW_Customization::ExitCustomization()
{
	PC->ExitCustomizationScreen();
}

void UUW_Customization::OnVehicleModeBtnClicked()
{
	EnterVehicleMode(false, true);
}

void UUW_Customization::OnWeaponModeBtnClicked()
{
	EnterLoadoutMode(false, true);
}

UDataManagerSubsystem* UUW_Customization::GetData_UUWCustomization()
{
	return GetGameInstance()->GetSubsystem<UDataManagerSubsystem>();;
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
		PC->PreviewStageActor->ZoomPreview(WheelDelta);
	}
	return FReply::Handled();
}


FReply UUW_Customization::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	CustomizationUIState.isRotating = false;
	return FReply::Handled();
}

FReply UUW_Customization::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	CustomizationUIState.isRotating = true;
	CustomizationUIState.LastMousePosition = InMouseEvent.GetScreenSpacePosition();
	return FReply::Handled();
}

