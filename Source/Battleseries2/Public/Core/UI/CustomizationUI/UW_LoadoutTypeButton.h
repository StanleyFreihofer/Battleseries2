// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h" 
#include "Components/SizeBox.h"
#include "Data/Vehicles/Data_Vehicle.h"
#include "UW_LoadoutTypeButton.generated.h"

class UDataManagerSubsystem;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLoadoutTypeClicked, int32, TypeIndex);

/**
 * 
 */
UCLASS()
class BATTLESERIES2_API UUW_LoadoutTypeButton : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	UButton* Btn_LoadoutType = nullptr;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* T_LoadoutTypeName = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	USizeBox* SizeBox = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TypeEnumIndex = -1;

	UPROPERTY(BlueprintAssignable)
	FOnLoadoutTypeClicked OnLoadoutTypeClicked;

	UFUNCTION(BlueprintCallable)
	void SetSoldierClassType(int32 NewType);
	UFUNCTION(BlueprintCallable)
	void SetVehicleType(int32 NewType);

	UFUNCTION(BlueprintCallable)
	void SetButtonDeselected();
	UFUNCTION(BlueprintCallable)
	void SetButtonSelected();

	UFUNCTION()
	void HandleButtonClicked();

	virtual void NativeConstruct() override;

private:
	UDataManagerSubsystem* DataSubsystem;
	bool isBound = false;
};