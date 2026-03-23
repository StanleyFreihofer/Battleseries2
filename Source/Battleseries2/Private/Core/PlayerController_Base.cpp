// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/PlayerController_Base.h"
#include "Blueprint/UserWidget.h"
#include "Utilities/HUDSubsystem.h"
#include "Utilities/DataManagerSubsystem.h"
#include "Data/Data_Customization.h"

void APlayerController_Base::BeginPlay()
{
	Super::BeginPlay();
	Init_CustomizationScreen();
}

void APlayerController_Base::Init_CustomizationScreen()
{
	UDA_CustomizationDefaults* CustDefaults = GetDataSystem()->GetCustomizationDefaults();
	PreviewStageActor = GetWorld()->SpawnActor<ALoadoutPreviewStage>(CustDefaults->PreviewStageClass, CustDefaults->PreviewSpawnTransform);
}

void APlayerController_Base::EnterCustomizationScreen()
{
	LastViewTarget = GetViewTarget();
	SetViewTarget(PreviewStageActor);
	UDA_CustomizationDefaults* CustDefaults = GetDataSystem()->GetCustomizationDefaults();
	GetHUDSystem()->SpawnCustomizationUI(CustDefaults->CustomizationWidgetClass);
	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(GetHUDSystem()->CustomizationWidget->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);
	bShowMouseCursor = true;
	GetHUDSystem()->CustomizationWidget->Init_Customization(PreviewStageActor);
}

void APlayerController_Base::ExitCustomizationScreen()
{
	SetViewTarget(LastViewTarget.Get());
	GetHUDSystem()->RemoveCustomizationWidget();
	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);
	bShowMouseCursor = false;
}

UHUDSubsystem* APlayerController_Base::GetHUDSystem()
{
	return GetLocalPlayer()->GetSubsystem<UHUDSubsystem>();
}

UDataManagerSubsystem* APlayerController_Base::GetDataSystem()
{
	return GetGameInstance()->GetSubsystem<UDataManagerSubsystem>();
}

