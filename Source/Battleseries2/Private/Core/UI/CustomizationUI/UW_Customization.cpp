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
#include "Components/CanvasPanelSlot.h"
#include "Core/UI/CustomizationUI/UW_LoadoutSlot.h"
#include "Core/UI/CustomizationUI/UW_LoadoutTypeButton.h"
#include "Core/UI/CustomizationUI/UW_DropdownOption.h"
#include "LoadoutPreviewStage.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Engine/GameViewportClient.h"
#include "Math/UnrealMathUtility.h"
#include "Utilities/HelperFunctions_Vehicle.h"
#include "Data/Core/CoreTypes.h"
#include "Core/Weapons/VehicleWeaponLogicComponent.h"
#include "Core/PlayerController_Base.h"
#include "Data/Weapons/WeaponEnums.h"
#include "Data/Weapons/Data_VehicleWeapon.h"
#include "Data/Data_Optics.h"
#include "Data/Data_Camo.h"
#include "Data/SoldierClassDefaults.h"
#include "Data/Vehicles/VehicleDefaults.h"
#include "Data/Data_Customization.h"

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

#pragma region CustomizationModes

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
}

void UUW_Customization::EnterLoadoutMode(bool OverrideCheck, bool BlendView)
{
	//character loadout (or kits)
	ClearPreviousMode();

	CustomizationUIState.CurrentCustomizationMode = ECoreType::Class;
	PC->PreviewStageActor->PreviewStageState.CurrentStageMode = ECoreType::Class;

	Build_ClassTypeScrollbox();

	if (BlendView)
	{
		PC->SetViewTargetWithBlend(PC->PreviewStageActor->GetCurrentPreviewCameraActor(), 2.0f);
	}
	else
	{
		PC->SetViewTarget(PC->PreviewStageActor->GetCurrentPreviewCameraActor());
	}

	UpdateSelectedClassType(0);
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

#pragma endregion

#pragma region ScrollBox

void UUW_Customization::Build_ClassTypeScrollbox()
{
	for (EClassType SoldierClass : TEnumRange<EClassType>())
	{
		UUW_LoadoutTypeButton* NewLoadoutTypeButton = CreateWidget<UUW_LoadoutTypeButton>(GetWorld(), GetData_UUWCustomization()->GetCustomizationDefaults()->LoadoutTypeWidgetClass);
		NewLoadoutTypeButton->SetSoldierClassType((int32)SoldierClass);
		NewLoadoutTypeButton->OnLoadoutTypeClicked.AddDynamic(this, &UUW_Customization::UpdateSelectedClassType);
		ScrollBox->AddChild(NewLoadoutTypeButton);
		CustomizationUIState.TypeButtons.Add(NewLoadoutTypeButton);
	}
}

void UUW_Customization::Build_VehicleTypeScrollbox()				
{
	TArray<EVehicleType>& AvailableVehicleTypes = GetData_UUWCustomization()->GetCustomizationDefaults()->CustomizableVehicleTypes;
	for (EVehicleType VehicleType : AvailableVehicleTypes)		
	{
		UUW_LoadoutTypeButton* NewLoadoutTypeButton = CreateWidget<UUW_LoadoutTypeButton>(GetWorld(), GetData_UUWCustomization()->GetCustomizationDefaults()->LoadoutTypeWidgetClass);
		NewLoadoutTypeButton->SetVehicleType((int32)VehicleType);
		NewLoadoutTypeButton->OnLoadoutTypeClicked.AddDynamic(this, &UUW_Customization::UpdateSelectedVehicleType);
		NewLoadoutTypeButton->OnLoadoutTypeHovered.AddDynamic(this, &UUW_Customization::ShowVehicleTypeOptions);

		ScrollBox->AddChild(NewLoadoutTypeButton);
		CustomizationUIState.TypeButtons.Add(NewLoadoutTypeButton);
	}
}

void UUW_Customization::ShowVehicleTypeOptions(int32 TypeEnumIndex)
{
	//builds/shows the dropdown below the scroll box when a vehicle type is hovered
	//if (CustomizationUIState.TypeEnumIndex == TypeEnumIndex)
	//{
		//return;
	//}

	VerticalBox_TypeDropdowns->ClearChildren();
	
	EVehicleType VehicleType = static_cast<EVehicleType>(CustomizationUIState.TypeEnumIndex);
	UDataTable* VehicleDataTable = GetData_UUWCustomization()->GetVehicleDataTable();
	TArray<FName> AllVehicleIDsOfType = FVehicleData::GetRowNamesOfType(VehicleDataTable, VehicleType);
	for (int32 i = 0; i < AllVehicleIDsOfType.Num(); ++i)
	{
		FName& VehicleID = AllVehicleIDsOfType[i];
		UUW_DropdownOption* NewVehicleOption = CreateWidget<UUW_DropdownOption>(this, GetData_UUWCustomization()->GetCustomizationDefaults()->DropdownOptionWidgetClass);
		NewVehicleOption->Init_DropdownOption(VehicleID, GetData_UUWCustomization()->GetVehicleDataRow(VehicleID)->Vehicle_DisplayName, i, ESlateVisibility::Visible);
		NewVehicleOption->OnOptionClicked.AddDynamic(this, &UUW_Customization::HandleVehicleSelected);
		VerticalBox_TypeDropdowns->AddChild(NewVehicleOption);
	}
}

void UUW_Customization::ClearVehicleTypeOptions()
{
	VerticalBox_TypeDropdowns->ClearChildren();
	//VerticalBox_TypeDropdowns->SetVisibility(ESlateVisibility::Collapsed);
}

void UUW_Customization::HandleVehicleSelected(UUW_DropdownOption* DropdownOption)
{
	FName& VehicleID = DropdownOption->ItemID;
	UpdateVehiclePreview(CustomizationUIState.TypeEnumIndex, VehicleID);
	ClearVehicleTypeOptions();
}

void UUW_Customization::RepopulateTypeScrollBox(int32 NumOfVisibleButtons)
{
	ScrollBox->ClearChildren();			//wipes the scrollbox clean
	FVector2D ViewportSize;
	GEngine->GameViewport->GetViewportSize(ViewportSize);
	UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(SizeBox_ScrollBox->Slot);
	float ScrollBoxWidth = CanvasSlot->GetSize().X;
	float ButtonWidth = ScrollBoxWidth / NumOfVisibleButtons;
	for (UUW_LoadoutTypeButton* Btn : CustomizationUIState.TypeButtons)
	{
		ScrollBox->AddChild(Btn);   // Adds them back in the NEW order we just made
		Btn->SizeBox->SetWidthOverride(ButtonWidth);

		if (Btn->TypeEnumIndex == CustomizationUIState.TypeEnumIndex)
		{
			Btn->SetButtonSelected();
		}
	}
}

void UUW_Customization::HandleRefreshTypeScrollBox(int32 NumOFVisibleButtons)
{
	auto& ButtonArray = CustomizationUIState.TypeButtons;
	bool bShouldScroll = false;

	switch (CustomizationUIState.CurrentCustomizationMode)
	{
		case ECoreType::Class:
			bShouldScroll = GetData_UUWCustomization()->GetCustomizationDefaults()->Scroll_SoldierClassMode;
			break;
		case ECoreType::Vehicle:
			bShouldScroll = GetData_UUWCustomization()->GetCustomizationDefaults()->Scroll_VehicleMode;
			break;
	}

	// 2. Apply Logic
	if (bShouldScroll)
	{
		// This function handles the array shuffling AND calls Repopulate
		RefreshTypeScrollBox(NumOFVisibleButtons);
	}
	else
	{
		// Fix the order to standard Enum order
		ButtonArray.Sort([](const UUW_LoadoutTypeButton& A, const UUW_LoadoutTypeButton& B) {
			return A.TypeEnumIndex < B.TypeEnumIndex;
			});

		// Explicitly call Repopulate with the FULL count so they all fit
		RepopulateTypeScrollBox(ButtonArray.Num());

		// Reset scroll so we don't start halfway through a static list
		ScrollBox->SetScrollOffset(0);
	}
}

void UUW_Customization::RefreshTypeScrollBox(int32 NumOfVisibleButtons)
{
	auto& ButtonArray = CustomizationUIState.TypeButtons;
	int32 NumOfButtons = ButtonArray.Num();
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
			//moving from front to back
			UUW_LoadoutTypeButton* FirstBtn = ButtonArray[0];
			ButtonArray.RemoveAt(0);		//removes the first button
			ButtonArray.Add(FirstBtn);		//sticks it on the end
			CurrentPos--;
		}
		else if (CurrentPos < 1)
		{
			//moving from back to front
			UUW_LoadoutTypeButton* LastBtn = ButtonArray.Last();
			ButtonArray.RemoveAt(ButtonArray.Num() - 1);	//removes the last button
			ButtonArray.Insert(LastBtn, 0);					//sticks it at the start
			CurrentPos++;
		}
	}

	RepopulateTypeScrollBox(NumOfVisibleButtons);
	int32 ButtonToSelect = NumOfVisibleButtons / 2;
	ButtonArray[ButtonToSelect]->SetButtonSelected();
	ScrollBox->SetScrollOffset(0);
}

#pragma endregion

#pragma region LoadoutPanel

#pragma region BuildLoadoutPanels

void UUW_Customization::Build_SoldierClassLoadoutPanel(int32 TypeEnumIndex)
{
	//soldier class
	EClassType SoldierClassType = static_cast<EClassType>(TypeEnumIndex);
	EWeaponType DefaultWeaponCategory = GetData_UUWCustomization()->GetSoldierClassDefaults()->SoldierClassDefinitions.Find(SoldierClassType)->AvailableWeaponCategories[0];

	//Build_SoldierClassLoadoutData_WeaponData
	const TArray<FName>& WeaponTypeIDs = GetData_UUWCustomization()->GetAllInfantryWeaponIDsOfType(DefaultWeaponCategory);
	FCustomizationSlotConfig NewSlotData = Build_LoadoutSlotData(WeaponTypeIDs, ECoreItemType::CharacterWeapon, 0, FText::GetEmpty());
	UUW_LoadoutSlot* NewSlotWidget = CreateNewLoadoutSlot();
	NewSlotWidget->Init_LoadoutSlot_Vehicle(NewSlotData, -1);			//CHANGE THIS

	LoadCurrentSave_InfantryWeaponSlot(NewSlotWidget, 0, TypeEnumIndex);
}

void UUW_Customization::Build_VehicleLoadoutPanel(int32 TypeEnumIndex)
{
	EVehicleType VehicleType = static_cast<EVehicleType>(TypeEnumIndex);
	FName VehicleID = GetData_UUWCustomization()->GetFirstVehicleIDOfType(VehicleType);
	const FVehicleData& VehicleData = *GetData_UUWCustomization()->GetVehicleDataRow(VehicleID);

	//TMap<int32, FAvailableItems*> CustomizableSeatOptions;			//option list to be built out
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
		const FVehicleWeaponSlotChoices& WeaponSlot = SeatData.AvailableItems.AvailableWeaponSlots[i];
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

#pragma endregion

UUW_LoadoutSlot* UUW_Customization::CreateNewLoadoutSlot()
{
	UUW_LoadoutSlot* NewSlotWidget = CreateWidget<UUW_LoadoutSlot>(GetWorld(), GetData_UUWCustomization()->GetCustomizationDefaults()->LoadoutSlotWidgetClass);
	VerticalBox_LoadoutPanel->AddChild(NewSlotWidget);
	CustomizationUIState.LoadoutSlots.Add(NewSlotWidget);
	NewSlotWidget->OnSlotSelectionChanged.AddDynamic(this, &UUW_Customization::HandleSlotSelectionChanged);
	NewSlotWidget->OnSlotHovered.AddDynamic(this, &UUW_Customization::HandleSlotHovered);
	NewSlotWidget->OnSlotOptionUsed.AddDynamic(this, &UUW_Customization::UpdateDetailsPanel);
	NewSlotWidget->OnSlotDeselected.AddDynamic(this, &UUW_Customization::HideDetailsPanel);
	return NewSlotWidget;
}

#pragma region LoadoutPanel_LoadSaves

void UUW_Customization::LoadCurrentSave_InfantryWeaponSlot(UUW_LoadoutSlot* NewSlotWidget, int32 WeaponIndex, int32 TypeEnumIndex)
{
	EClassType InfantryClassType = static_cast<EClassType>(TypeEnumIndex);
	USaveSubsystem* SaveSubsystem = GetGameInstance()->GetSubsystem<USaveSubsystem>();

	const FPlayerLoadoutConfig_Class& SavedClassLoadout = SaveSubsystem->GetClassLoadout(InfantryClassType);
	if (SavedClassLoadout.Weapons.IsValidIndex(WeaponIndex))
	{
		FName SavedSelection = NAME_None;
		int32 OptionIndex = 0;
		SavedSelection = SavedClassLoadout.Weapons[WeaponIndex];
		OptionIndex = NewSlotWidget->GetOptionIndexFromItemID(SavedSelection);
		NewSlotWidget->UpdateSlotSelection(OptionIndex);
	}
	else
	{
		NewSlotWidget->UpdateSlotSelection(0);
	}
}

void UUW_Customization::LoadCurrentSave_VehicleWeaponSlot(UUW_LoadoutSlot* NewSlotWidget, int32 SeatIndex, int32 WeaponIndex, int32 TypeEnumIndex)
{
	//DONT UPDATE PREVIEW HERE, THIS IS FOR SLOTS ONLY, PREVIEW USES APPLYLOADOUTTOSEAT ON VEHICLE CLASS
	EVehicleType VehicleType = static_cast<EVehicleType>(TypeEnumIndex);
	USaveSubsystem* SaveSubsystem = GetGameInstance()->GetSubsystem<USaveSubsystem>();
	const FSavedSeatLoadout& SavedSeat = SaveSubsystem->GetSeatLoadout(VehicleType, SeatIndex);

	if (SavedSeat.Weapons.IsValidIndex(WeaponIndex))
	{
		FName SavedSelection = NAME_None;
		int32 OptionIndex = 0;
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
	EVehicleType VehicleType = static_cast<EVehicleType>(TypeEnumIndex);
	USaveSubsystem* SaveSubsystem = GetGameInstance()->GetSubsystem<USaveSubsystem>();

	const FSavedSeatLoadout& SavedSeat = SaveSubsystem->GetSeatLoadout(VehicleType, SeatIndex);
	if (!SavedSeat.Optic.IsNone())
	{
		FName SavedSelection = NAME_None;
		int32 OptionIndex = 0;
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
	EVehicleType VehicleType = static_cast<EVehicleType>(TypeEnumIndex);
	USaveSubsystem* SaveSubsystem = GetGameInstance()->GetSubsystem<USaveSubsystem>();
	const FPlayerLoadoutConfig_Vehicle& SavedVehicleLoadout = SaveSubsystem->GetVehicleLoadout(VehicleType);

	if (SavedVehicleLoadout.VehicleCamo.IsValid())
	{
		FName SavedSelection = NAME_None;
		int32 OptionIndex = 0;
		SavedSelection = SavedVehicleLoadout.VehicleCamo;
		OptionIndex = NewSlotWidget->GetOptionIndexFromItemID(SavedSelection);
		NewSlotWidget->UpdateSlotSelection(OptionIndex);
	}
	else
	{
		//NewSlotWidget->UpdateSlotSelection(0);
	}
}

#pragma endregion

void UUW_Customization::Clear_LoadoutPanel()
{
	VerticalBox_LoadoutPanel->ClearChildren();
	for (UUW_LoadoutSlot* SlotWidget : CustomizationUIState.LoadoutSlots)
	{
		SlotWidget->HideOptions();
		SlotWidget->RemoveFromParent();
	}

	CustomizationUIState.LoadoutSlots.Empty();
}

#pragma endregion

#pragma region DetailsPanel

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

#pragma endregion

void UUW_Customization::UpdateVehiclePreview(int32 TypeEnumIndex, FName VehicleID)
{
	USaveSubsystem* SaveSubsystem = GetGameInstance()->GetSubsystem<USaveSubsystem>();
	EVehicleType VehicleType = static_cast<EVehicleType>(TypeEnumIndex);
	//if you wanna add functionality for selecting specific vehicle for preview, create function add it here probably
	//What happens when you make a static DataTable in a class ?
		//It becomes one shared copy for the whole game, not per object.
		//You must define it in exactly one.cpp file.
		//Everyone who uses it is really pointing to the same thing.
		//eg. vehicle_base owns vehicle data table
		//and now this ui hooks into vehicle base to access said shit i guess


	//1. clear soontobe previous vehicle's loadout
	//EVehicleType VehicleType = CustomizationUIState.CustomizationUIState_Vehicle.CurrentVehicleType;
	
	if (PC->PreviewStageActor->PreviewStageState.CurrentVehicle)
	{
		PC->PreviewStageActor->PreviewStageState.CurrentVehicle->ClearEntireLoadoutFromVehicle();
	}

	//create new VehicleInstanceData (id, loadout, preview)
	FVehicleStartingData NewVehicleStartingData;
	NewVehicleStartingData.PreviewVehicle = true;

	if (VehicleID == NAME_None)
	{
		FString TypeName = StaticEnum<EVehicleType>()->GetNameStringByValue((int64)VehicleType);
		UDataTable* VehicleDataTable = GetData_UUWCustomization()->GetVehicleDataTable();
		TArray<FName> AllVehicleIDsOfType = FVehicleData::GetRowNamesOfType(VehicleDataTable, VehicleType);
		FName RowName = FName(*StaticEnum<EVehicleType>()->GetNameStringByValue((int64)VehicleType));

		NewVehicleStartingData.VehicleID = AllVehicleIDsOfType[0];
	}
	else
	{
		NewVehicleStartingData.VehicleID = VehicleID;
	}

	NewVehicleStartingData.StartingVehicleLoadout = SaveSubsystem->GetVehicleLoadout(VehicleType);

	FTransform PreviewTransform = GetData_UUWCustomization()->GetVehicleDataRow(NewVehicleStartingData.VehicleID)->CustomizationPosition;
	UE_LOG(LogTemp, Warning, TEXT("location: %s"), *PreviewTransform.GetLocation().ToString());

	PC->PreviewStageActor->SetupNewPreviewVehicle(PreviewTransform, NewVehicleStartingData);		//<--default selection

	T_PreviewName->SetText(GetData_UUWCustomization()->GetVehicleDataRow(NewVehicleStartingData.VehicleID)->Vehicle_DisplayName);
}

#pragma region TypeUpdates

void UUW_Customization::UpdateSelectedClassType(int32 TypeEnumIndex)
{
	if (TypeEnumIndex != CustomizationUIState.TypeEnumIndex)
	{
		CustomizationUIState.TypeEnumIndex = TypeEnumIndex;

		Clear_LoadoutPanel();
		Build_SoldierClassLoadoutPanel(TypeEnumIndex);

		HandleTypeSelection(TypeEnumIndex);

		HandleRefreshTypeScrollBox(GetData_UUWCustomization()->GetCustomizationDefaults()->NumOfViewableTypeButtons_SoldierClassMode);
	}
}

void UUW_Customization::UpdateSelectedVehicleType(int32 TypeEnumIndex)
{
	if (TypeEnumIndex != CustomizationUIState.TypeEnumIndex)
	{
		CustomizationUIState.TypeEnumIndex = TypeEnumIndex;

		Clear_LoadoutPanel();

		Build_VehicleLoadoutPanel(CustomizationUIState.TypeEnumIndex);
		UpdateVehiclePreview(TypeEnumIndex, NAME_None);

		//CustomizationUIState.TypeButtons[TypeEnumIndex]->OnLoadoutTypeHovered.AddDynamic(this, &UUW_Customization::ShowVehicleTypeOptions);

		HandleTypeSelection(TypeEnumIndex);

		HandleRefreshTypeScrollBox(GetData_UUWCustomization()->GetCustomizationDefaults()->NumOfViewableTypeButtons_VehicleMode);

		//ClearVehicleTypeOptions();
	}
}

void UUW_Customization::HandleTypeSelection(int32 TypeEnumIndex)
{
	for (UUW_LoadoutTypeButton* TypeButtonInQuestion : CustomizationUIState.TypeButtons)		
	{
		if (TypeButtonInQuestion->TypeEnumIndex != TypeEnumIndex)
		{
			TypeButtonInQuestion->SetButtonDeselected();
		}
	}
	//ClearVehicleTypeOptions();
}

#pragma endregion

#pragma region SlotUpdates

void UUW_Customization::HandleSlotSelectionChanged(int32 SeatIndex, FCustomizationSlotConfig SlotConfig, FName SelectedItemID)			//<--shouldnt this just be called update loadout
{
	USaveSubsystem* SaveSubsystem = GetGameInstance()->GetSubsystem<USaveSubsystem>();

	switch (SlotConfig.CoreItemType)
	{
		case ECoreItemType::CharacterWeapon:
		{
			EClassType CurrentClassType = static_cast<EClassType>(CustomizationUIState.TypeEnumIndex);
			SaveSubsystem->SetLoadoutWeaponChoice_Infantry(CurrentClassType, SlotConfig.ItemSlot, SelectedItemID);
			PC->PreviewStageActor->UpdateWeaponPreview(SelectedItemID);
			break;
		}
		case ECoreItemType::VehicleWeapon:
		{
			EVehicleType CurrentVehicleType = static_cast<EVehicleType>(CustomizationUIState.TypeEnumIndex);
			SaveSubsystem->SetLoadoutWeaponChoice_Vehicle(CurrentVehicleType, SeatIndex, SlotConfig.ItemSlot, SelectedItemID);
			PC->PreviewStageActor->PreviewStageState.CurrentVehicle->VehicleWeaponLogicComponent->ClearWeaponSlotFromSeat(SeatIndex, SlotConfig.ItemSlot);
			PC->PreviewStageActor->PreviewStageState.CurrentVehicle->VehicleWeaponLogicComponent->ApplyWeaponAtIndexToSeat(SeatIndex, SlotConfig.ItemSlot, SelectedItemID);
			break;
		}
		case ECoreItemType::VehicleOptic:
		{
			EVehicleType CurrentVehicleType = static_cast<EVehicleType>(CustomizationUIState.TypeEnumIndex);
			SaveSubsystem->SetLoadoutOpticChoice_Vehicle(CurrentVehicleType, SeatIndex, SelectedItemID);
			break;
		}
		case ECoreItemType::Camo:
		{
			EVehicleType CurrentVehicleType = static_cast<EVehicleType>(CustomizationUIState.TypeEnumIndex);
			SaveSubsystem->SetLoadoutCamoChoice_Vehicle(CurrentVehicleType, SelectedItemID);
			PC->PreviewStageActor->PreviewStageState.CurrentVehicle->ApplyCamoToVehicle(SelectedItemID);
			break;
		}
	}
}

void UUW_Customization::HandleSlotHovered(const TArray<UUW_DropdownOption*>& Options, FSlotData Data)
{
	VerticalBox_Dropdowns->ClearChildren();

	for (UUW_LoadoutSlot* SlotToCheck : CustomizationUIState.LoadoutSlots)
	{
		if (SlotToCheck->ValidateSlotDeselection())
		{
			SlotToCheck->UpdateSlot_UnHover();
		}
	}

	for (UUW_DropdownOption* Option : Options)
	{
		VerticalBox_Dropdowns->AddChild(Option);
		Option->SetVisibility(ESlateVisibility::Visible);
	}
}

#pragma endregion

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
		switch (CustomizationUIState.CurrentCustomizationMode)
		{
			case ECoreType::Class:
				UpdateSelectedClassType(TargetTypeIndex);
				break;
			case ECoreType::Vehicle:
				UpdateSelectedVehicleType(TargetTypeIndex);
				break;
		}

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

