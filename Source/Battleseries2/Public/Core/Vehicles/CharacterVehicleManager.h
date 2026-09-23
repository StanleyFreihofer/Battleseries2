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
	UFUNCTION(BlueprintCallable)
	void UpdateSeatList(TArray<ACharacter_Base*> Characters);
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
	void CharacterEnterSeat(const FCharacterSeatContext& SeatContext);
	UFUNCTION(BlueprintCallable)
	void HandleEnterSeat_Gunner(const FCharacterSeatContext& SeatContext);
	UFUNCTION(BlueprintCallable)
	void CharacterExitSeat(const FCharacterSeatContext& SeatContext);
	UFUNCTION(BlueprintCallable)
	FVector CalculateSafeExitLocation(AActor* Vehicle);
	UFUNCTION(BlueprintCallable)
	void UpdateSeatIndexes(int32 NewLSI, int32 NewCSI, int32 NewNSI);
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
	ACharacter_Base* GetOwnerCharacter();
	
	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (BlueprintThreadSafe))
	int32& GetCSI();
	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool& GetInVehicle();
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	AVehicle_Base* GetCurrentVehicle();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
