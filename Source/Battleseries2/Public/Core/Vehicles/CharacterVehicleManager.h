// coding in UE almost makes me wish for a nuclear winter

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/Characters/CharacterTypes.h"
#include "CharacterVehicleManager.generated.h"


UCLASS(Blueprintable, BlueprintType)
class BATTLESERIES2_API UCharacterVehicleManager : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCharacterVehicleManager();
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "State")
	FCharacterVehicleState CharacterVehicleState = FCharacterVehicleState();
	
	
	
	UFUNCTION(BlueprintCallable)
	void ManageinVehicleStatus(AVehicle_Base* Vehicle, bool In_Vehicle);
	
#pragma region EnterExitVehicle
	UFUNCTION(BlueprintCallable)
	void AttemptEnterVehicle(AVehicle_Base* Vehicle);
	UFUNCTION(BlueprintCallable)
	void StartEnterVehicle(AVehicle_Base* Vehicle, int32 SeatIndex);
	UFUNCTION(BlueprintCallable)
	void CharacterEnterVehicle();
	UFUNCTION(BlueprintCallable)
	void EnterVehicle_LocalPlayer();
	UFUNCTION(BlueprintCallable)
	void PhysicallyEnterVehicle();
	UFUNCTION(BlueprintCallable)
	void CharacterExitVehicle();
	UFUNCTION(BlueprintCallable)
	void ExitVehicle_LocalPlayer();
	UFUNCTION(BlueprintCallable)
	void PhysicallyExitVehicle();
	UFUNCTION(BlueprintCallable)
	void StartEnterSeat();
	UFUNCTION(BlueprintCallable)
	void StartExitSeat(int32 SeatIndex);
	UFUNCTION(BlueprintCallable)
	void ChangeSeat(int32 SeatIndex);
	UFUNCTION(BlueprintCallable)
	void CharacterEnterSeat(const FCharacterSeatContext& SeatContext);
	UFUNCTION(BlueprintCallable)
	void HandleEnterSeat_Gunner(const FCharacterSeatContext& SeatContext);
	UFUNCTION(BlueprintCallable)
	void CharacterExitSeat(int32 SeatIndex);
	UFUNCTION(BlueprintCallable)
	FVector CalculateSafeExitLocation(AActor* Vehicle);
	
	UFUNCTION(BlueprintCallable)
	void AttemptChangeSeat();
#pragma endregion	
	UFUNCTION(BlueprintCallable)
	void UpdateCSI(int32 NewCSI);
	UFUNCTION(BlueprintCallable)
	void UpdateVehicleHUD(TSubclassOf<UUserWidget> HUDClass);
	UFUNCTION(BlueprintCallable)
	void UpdateUI_EnterSeat();
	UFUNCTION(BlueprintCallable)
	void UpdateUI_EnterSeat_Turrets();
	UFUNCTION(BlueprintCallable)
	void OnVehicleHealthChanged();
	UFUNCTION(BlueprintCallable)
	void UpdateRangefinder_WindowedVehicle();
	
	UFUNCTION(BlueprintCallable)
	void HandleViewMethod(int32 SeatIndex);
	UFUNCTION(BlueprintCallable)
	void HandleViewMethod_Default(int32 SeatIndex);
	UFUNCTION(BlueprintCallable)
	void ToggleCurrentViewMode();
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	ACharacter_Base* GetOwnerCharacter();
	
	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (BlueprintThreadSafe))
	int32& GetCSI();
	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool GetInVehicle();
	UFUNCTION(BlueprintCallable, BlueprintPure)
	const FVehicleTypeSeatDefinition& GetCurrentSeatDefinition();
	UFUNCTION(BlueprintCallable, BlueprintPure)
	const FSeatData& GetCurrentSeatData();
	UFUNCTION(BlueprintCallable, BlueprintPure)
	FSeatWeaponSystemState& GetCurrentSeatWeaponSystem();
	UFUNCTION(BlueprintCallable, BlueprintPure)
	USkeletalMeshComponent* GetActiveCharacterAttachTarget();
	UFUNCTION(BlueprintCallable, BlueprintPure)
	FName GetIKAttachSocket(bool Foot, bool Left);
	UFUNCTION(BlueprintCallable, BlueprintPure)
	FTransform GetIKTransform(bool Foot, bool Left);
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	AVehicle_Base* GetCurrentVehicle();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
