// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/UI/CustomizationUI/UW_LoadoutTypeButton.h"
#include "Utilities/DataManagerSubsystem.h"
#include "Data/Vehicles/VehicleDefaults.h"
#include "Data/SoldierClassDefaults.h"

void UUW_LoadoutTypeButton::NativeConstruct()
{
	Super::NativeConstruct();
	DataSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UDataManagerSubsystem>();
	
	if (!isBound)
	{
		Btn_LoadoutType->OnClicked.AddDynamic(this, &UUW_LoadoutTypeButton::HandleButtonClicked);
		isBound = true;
	}
};

void UUW_LoadoutTypeButton::SetSoldierClassType(int32 NewType)
{
	TypeEnumIndex = NewType;
	DataSubsystem = GetGameInstance()->GetSubsystem<UDataManagerSubsystem>();
	FText DisplayNameText = DataSubsystem->GetSoldierClassDefaults()->SoldierClassDefinitions.Find(static_cast<EClassType>(NewType))->DisplayName;
	T_LoadoutTypeName->SetText(DisplayNameText);
}

void UUW_LoadoutTypeButton::SetVehicleType(int32 NewType)
{
	TypeEnumIndex = NewType;
    DataSubsystem = GetGameInstance()->GetSubsystem<UDataManagerSubsystem>();
	FText DisplayNameText = DataSubsystem->GetVehicleDefaults()->VehicleTypeDefintions.Find(static_cast<EVehicleType>(NewType))->DisplayName;
	UE_LOG(LogTemp, Warning, TEXT("[UUW_LoadoutTypeButton::SetVehicleType] %s"), *DisplayNameText.ToString());
    T_LoadoutTypeName->SetText(DisplayNameText);
}

void UUW_LoadoutTypeButton::HandleButtonClicked()
{
	//SetButtonSelected();
	OnLoadoutTypeClicked.Broadcast(TypeEnumIndex);
}

void UUW_LoadoutTypeButton::SetButtonDeselected()
{
	Btn_LoadoutType->SetColorAndOpacity(FLinearColor::White);
	Btn_LoadoutType->SetBackgroundColor(FLinearColor::Black);
	//T_VehicleTypeName->SetColorAndOpacity(FLinearColor::White);
}

void UUW_LoadoutTypeButton::SetButtonSelected()
{
	Btn_LoadoutType->SetColorAndOpacity(FLinearColor::Black);
	Btn_LoadoutType->SetBackgroundColor(FLinearColor::White);
	//T_VehicleTypeName->SetColorAndOpacity(FLinearColor::Black);
}

