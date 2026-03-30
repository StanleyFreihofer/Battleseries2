// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/UI/CustomizationUI/UW_VehicleTypeButton.h"
#include "Utilities/DataManagerSubsystem.h"
#include "Data/Vehicles/VehicleDefaults.h"

void UUW_VehicleTypeButton::NativeConstruct()
{
	Super::NativeConstruct();
	DataSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UDataManagerSubsystem>();
	
	if (!isBound)
	{
		Btn_VehicleType->OnClicked.AddDynamic(this, &UUW_VehicleTypeButton::HandleButtonClicked);
		isBound = true;
	}
};

void UUW_VehicleTypeButton::SetVehicleType(int32 NewType)
{
	TypeEnumIndex = NewType;
    DataSubsystem = GetGameInstance()->GetSubsystem<UDataManagerSubsystem>();
	FText DisplayNameText = DataSubsystem->GetVehicleDefaults()->VehicleTypeDefintions.Find(static_cast<E_VehicleType>(NewType))->DisplayName;
    T_VehicleTypeName->SetText(DisplayNameText);
}

void UUW_VehicleTypeButton::HandleButtonClicked()
{
	//SetButtonSelected();
	OnVehicleTypeClicked.Broadcast(TypeEnumIndex);
}

void UUW_VehicleTypeButton::SetButtonDeselected()
{
	Btn_VehicleType->SetColorAndOpacity(FLinearColor::White);
	Btn_VehicleType->SetBackgroundColor(FLinearColor::Black);
	//T_VehicleTypeName->SetColorAndOpacity(FLinearColor::White);
}

void UUW_VehicleTypeButton::SetButtonSelected()
{
	Btn_VehicleType->SetColorAndOpacity(FLinearColor::Black);
	Btn_VehicleType->SetBackgroundColor(FLinearColor::White);
	//T_VehicleTypeName->SetColorAndOpacity(FLinearColor::Black);
}

