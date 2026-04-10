
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Data/Vehicles/Data_Vehicle.h"
#include "UW_Customization.generated.h"

class UUW_VehicleTypeButton;
class UUW_LoadoutSlot;
class ALoadoutPreviewStage;
class UDataManagerSubsystem;
class APlayerController_Base;
enum class ECoreItemType : uint8;
enum class ECoreType : uint8;
enum class EGenericItemContext : uint8;
struct FCustomizationSlotState;
struct FCustomizationSlotConfig;
struct FSlotData;
struct FBaseWeaponData;

USTRUCT(BlueprintType)
struct FCustomizationUIState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)		//the mode/generic category of thing we are customizing
	ECoreType CurrentCustomizationMode = ECoreType::Vehicle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)		//in the type scrollbox, designates the index within that enum of thing we are currently customizing
	int32 TypeEnumIndex = -1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TArray<UUW_VehicleTypeButton*> TypeButtons;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TArray<UUW_LoadoutSlot*> LoadoutSlots;		//each of the loadout slots

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	bool isRotating = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FVector2D LastMousePosition = FVector2D();
};


UCLASS()
class BATTLESERIES2_API UUW_Customization : public UUserWidget
{
	GENERATED_BODY()

public:

	//Designer Components
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (BindWidget))
	class UScrollBox* ScrollBox = nullptr;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (BindWidget))
	class USizeBox* SizeBox_ScrollBox = nullptr;
	UPROPERTY(meta = (BindWidget))
	class UVerticalBox* VerticalBox_LoadoutPanel = nullptr;
	UPROPERTY(meta = (BindWidget))
	UButton* Btn_ExitCustomization = nullptr;
	UPROPERTY(meta = (BindWidget))
	UButton* Btn_WeaponMode = nullptr;
	UPROPERTY(meta = (BindWidget))
	UButton* Btn_VehicleMode = nullptr;
	UPROPERTY(meta = (BindWidget))
	UButton* Btn_CharacterMode = nullptr;

	//details panel
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (BindWidget))
	class UBorder* Border_DetailsPanel = nullptr;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* T_ItemName = nullptr;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* T_Description = nullptr;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (BindWidget))
	class UImage* Img_Icon = nullptr;

	UPROPERTY()
	FTimerHandle DetailsPanel_FadeTimerHandle = FTimerHandle();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UUW_VehicleTypeButton> VehicleTypeButton = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UUW_LoadoutSlot> LoadoutSlot = nullptr;




	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FCustomizationUIState CustomizationUIState = FCustomizationUIState();


	// Constructor
	//UUW_Customization(const FObjectInitializer & ObjectInitializer);
	
	/** Called when the underlying slate widget is constructed */
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UFUNCTION(BlueprintCallable)
	void Init_Customization(ALoadoutPreviewStage* InputStageActor);
	UFUNCTION(BlueprintCallable)
	void EnterVehicleMode(bool OverrideCheck, bool BlendView);
	UFUNCTION(BlueprintCallable)
	void EnterWeaponMode(bool OverrideCheck, bool BlendView);
	UFUNCTION(BlueprintCallable)
	void Init_TypeScrollBox();
	UFUNCTION(BlueprintCallable)
	void Build_VehicleTypeScrollbox();
	UFUNCTION(BlueprintCallable)
	void RepopulateTypeScrollBox();
	UFUNCTION(BlueprintCallable)
	void RefreshTypeScrollBox();

	UFUNCTION(BlueprintCallable)
	void Build_VehicleLoadoutPanel(int32 TypeEnumIndex);
	UFUNCTION(BlueprintCallable)
	FCustomizationSlotConfig Build_LoadoutSlotData(TArray<FName> ItemIDs, ECoreItemType ItemType, int32 ItemSlot, FText Context);
	UFUNCTION(BlueprintCallable)
	void Build_VehicleLoadoutData_Weapon(const FSeatData& SeatData, int32 TypeEnumIndex, int32 SeatIndex);
	UFUNCTION(BlueprintCallable)
	void Build_VehicleLoadoutData_Optic(const FSeatData& SeatData, int32 TypeEnumIndex, int32 SeatIndex);
	UFUNCTION(BlueprintCallable)
	void Build_VehicleLoadoutData_Camo(const FVehicleData& VehicleData, int32 TypeEnumIndex);
	UFUNCTION(BlueprintCallable)
	UUW_LoadoutSlot* CreateNewLoadoutSlot();
	UFUNCTION(BlueprintCallable)
	void LoadCurrentSave_VehicleWeaponSlot(UUW_LoadoutSlot* NewSlotWidget, int32 SeatIndex, int32 WeaponIndex, int32 TypeEnumIndex);
	UFUNCTION(BlueprintCallable)
	void LoadCurrentSave_VehicleOptic(UUW_LoadoutSlot* NewSlotWidget, int32 SeatIndex, int32 TypeEnumIndex);
	UFUNCTION(BlueprintCallable)
	void LoadCurrentSave_VehicleCamo(UUW_LoadoutSlot* NewSlotWidget, int32 TypeEnumIndex);

	UFUNCTION(BlueprintCallable)
	void Clear_LoadoutPanel();
	UFUNCTION(BlueprintCallable)
	void FadeInOutDetailsPanel(bool FadeOut);
	UFUNCTION(BlueprintCallable)
	void UpdateDetailsPanel(FSlotData NewSlotData, int32 OptionIndex);
	UFUNCTION(BlueprintCallable)
	void HideDetailsPanel();
	UFUNCTION(BlueprintCallable)
	void FillDetailsPanel_VehicleWeapon(FName WeaponID);
	UFUNCTION(BlueprintCallable)
	void FillDetailsPanel_Optic(FName OpticID);
	UFUNCTION(BlueprintCallable)
	void FillDetailsPanel_Camo(FName CamoID);

	UFUNCTION(BlueprintCallable)
	void UpdateVehiclePreview(int32 TypeEnumIndex);

	UFUNCTION(BlueprintCallable)
	void UpdateSelectedVehicleType(int32 TypeEnumIndex);

	UFUNCTION(BlueprintCallable)
	void HandleSlotSelectionChanged(int32 SeatIndex, FCustomizationSlotConfig SlotConfig, FName SelectedItemID);

	UFUNCTION(BlueprintCallable)
	void ExitCustomization();

	UFUNCTION()
	void OnVehicleModeBtnClicked();
	UFUNCTION()
	void OnWeaponModeBtnClicked();

	UFUNCTION()
	UDataManagerSubsystem* GetData_UUWCustomization();

protected:
	virtual FReply NativeOnMouseWheel(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	//virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

private:
	//UDataManagerSubsystem* DataSubsystem;
	APlayerController_Base* PC;

};

