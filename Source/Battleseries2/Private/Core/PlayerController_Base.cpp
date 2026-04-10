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
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	// This tells Unreal: "Put it exactly where I said, even if it's inside a wall"
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	PreviewStageActor = GetWorld()->SpawnActor<ALoadoutPreviewStage>(CustDefaults->PreviewStageClass, CustDefaults->PreviewSpawnTransform, SpawnParams);
}

void APlayerController_Base::EnterCustomizationScreen()
{
	LastViewTarget = GetViewTarget();

	UDA_CustomizationDefaults* CustDefaults = GetDataSystem()->GetCustomizationDefaults();
	GetHUDSystem()->SpawnCustomizationUI(CustDefaults->CustomizationWidgetClass);
	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(GetHUDSystem()->CustomizationWidget->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);
	bShowMouseCursor = true;
	GetHUDSystem()->CustomizationWidget->Init_Customization(PreviewStageActor);		

	SetViewTarget(PreviewStageActor->GetCurrentPreviewCameraActor());
}

void APlayerController_Base::ExitCustomizationScreen()
{
	SetViewTarget(LastViewTarget.Get());
	GetHUDSystem()->RemoveCustomizationWidget();
	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);
	bShowMouseCursor = false;
}

void APlayerController_Base::EnterSpawnScreen()
{
}

UHUDSubsystem* APlayerController_Base::GetHUDSystem()
{
	return GetLocalPlayer()->GetSubsystem<UHUDSubsystem>();
}

UDataManagerSubsystem* APlayerController_Base::GetDataSystem()
{
	return GetGameInstance()->GetSubsystem<UDataManagerSubsystem>();
}

