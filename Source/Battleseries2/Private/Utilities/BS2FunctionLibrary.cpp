#include "Utilities/BS2FunctionLibrary.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Animation/AnimSingleNodeInstance.h"
#include "Data/Items/Weapons/WeaponDefaults.h"
#include "Data/Items/Weapons/WeaponTypes.h"
#include "Data/Items/Weapons/ProjectileTypes.h"
#include "Utilities/BS2FunctionLibrary.h"
#include "Utilities/DataManagerSubsystem.h"
#include "Utilities/ProjectilePoolSubsystem.h"
#include "Utilities/HUDSubsystem.h"
#include "Save/SaveSubsystem.h"
#include "Utilities/I_VehicleDataAccessor.h"
#include "Components/AudioComponent.h"
#include "Camera/CameraComponent.h"
#include "Data/Items/Gadgets/GadgetTypes.h"
#include "Data/Items/Weapons/Data_Projectile.h"
#include "Data/Vehicles/VehicleDefaults.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/KismetMathLibrary.h"
#include "Utilities/I_Damageable.h"

bool UBS2FunctionLibrary::PerformSphereTraceMulti(const UObject* WorldContextObject, const FTransform StartTransform, TArray<FHitResult>& OutHits, TArray<AActor*> ActorsToIgnore, float Radius, float Distance, bool Debug)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	FVector Startpoint = StartTransform.GetLocation();
	FVector GetRotationXVector = StartTransform.GetRotation().Rotator().Vector();
	FVector Endpoint = GetRotationXVector * Distance + Startpoint;

	FCollisionQueryParams Params;
	FCollisionShape SphereShape = FCollisionShape::MakeSphere(Radius);
	Params.AddIgnoredActors(ActorsToIgnore);
	EDrawDebugTrace::Type DebugTrace = Debug ? EDrawDebugTrace::ForOneFrame : EDrawDebugTrace::None;					//<--namespaced enum

	return UKismetSystemLibrary::SphereTraceMulti(
		WorldContextObject,
		Startpoint,
		Endpoint,
		Radius,
		UEngineTypes::ConvertToTraceType(ECC_Visibility),
		false,              // bTraceComplex
		ActorsToIgnore,
		DebugTrace,
		OutHits,
		true,               // bIgnoreSelf
		FLinearColor::Red,  // Trace Color
		FLinearColor::Green,// Hit Color
		0.0f                // Draw Time
	);
	return false;
}

bool UBS2FunctionLibrary::PerformLineTrace(const UObject* WorldContextObject, const FTransform& StartTransform, FHitResult& OutHit, TArray<AActor*> ActorsToIgnore, bool Debug)
{
	//if calling from any actor, WorldContextObject = this/self
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	FVector Startpoint = StartTransform.GetLocation();
	FVector GetRotationXVector = StartTransform.GetRotation().Rotator().Vector();
	FVector Endpoint = GetRotationXVector * 50000.0f + Startpoint;

	FCollisionQueryParams Params;
	Params.AddIgnoredActors(ActorsToIgnore);
	bool bDidHit = World->LineTraceSingleByChannel(OutHit, Startpoint, Endpoint, ECC_Visibility, Params);
	if (Debug)
	{
		DrawDebugLine(World, Startpoint, Endpoint, bDidHit ? FColor::Green : FColor::Red, false, 0.1f, 0, 0.02f);
	}

	return bDidHit;
}

float UBS2FunctionLibrary::GetDesiredAnimMontagePlayRate(UAnimMontage* AnimMontage, float DesiredAnimDuration)
{
	if (DesiredAnimDuration <= 0.f) { return 1.0f; }
	float AnimPlayRate = AnimMontage->GetPlayLength() / DesiredAnimDuration;
	return AnimPlayRate;
}

void UBS2FunctionLibrary::PlayAnimMontageAtDesiredDuration(UAnimInstance* AnimInstance, UAnimMontage* AnimMontage, float DesiredAnimDuration)
{
	float AnimPlayRate = GetDesiredAnimMontagePlayRate(AnimMontage, DesiredAnimDuration);
	AnimInstance->Montage_Play(AnimMontage, AnimPlayRate);
}

float UBS2FunctionLibrary::GetDesiredAnimSequencePlayRate(UAnimSequence* AnimSequence, float DesiredAnimDuration)
{
	if (DesiredAnimDuration <= 0.f) { return 1.0f; }
	return AnimSequence->GetPlayLength() / DesiredAnimDuration;
}

void UBS2FunctionLibrary::PlayAnimSequenceAtDesiredDuration(USkeletalMeshComponent* MeshComp, UAnimSequence* AnimSequence, float DesiredAnimDuration, bool bLooping)
{
	MeshComp->PlayAnimation(AnimSequence, bLooping);
	UAnimSingleNodeInstance* SingleNode = MeshComp->GetSingleNodeInstance();
	SingleNode->SetPlayRate(GetDesiredAnimSequencePlayRate(AnimSequence, DesiredAnimDuration));
}

FTransform UBS2FunctionLibrary::GetSightOffset(UAnimInstance* AnimInstance, FTransform SightTransform, float CameraDistance, FTransform CameraTransform)
{
	FTransform MeshWorldTransform = AnimInstance->GetOwningComponent()->GetComponentTransform();
	CameraTransform.SetScale3D(FVector::OneVector);
	const FTransform MeshRelativeToCamera = UKismetMathLibrary::MakeRelativeTransform(MeshWorldTransform, CameraTransform);
	
	const FVector MeshRelativeLocation = MeshRelativeToCamera.GetLocation();
	const FRotator MeshRelativeRotation = MeshRelativeToCamera.Rotator();
	
	// Undo the sight's own rotation on its own translation offset —
	// "where would this offset be if the sight had no tilt of its own"
	const FRotator NegativeSightRotation = SightTransform.Rotator() * -1.0f;
	const FVector UnrotatedSightLocation = NegativeSightRotation.RotateVector(SightTransform.GetLocation());
	
	// Cancel out the mesh's offset from the camera and the sight's own offset,
	// then push forward by CameraDistance so the sight lands exactly that far in front of the camera
	FVector CombinedLocation = (MeshRelativeLocation * -1.0f) + (UnrotatedSightLocation * -1.0f);
	CombinedLocation += FVector(CameraDistance, 0.f, 0.f);
	
	// Convert that combined offset back into the weapon mesh's own local space
	const FRotator NegativeMeshRelativeRotation = MeshRelativeRotation * -1.0f;
	const FVector FinalLocation = NegativeMeshRelativeRotation.RotateVector(CombinedLocation);
	
	// Same idea for rotation: compose the mesh's camera-relative rotation with the sight's own rotation, then invert
	const FRotator ComposedRotation = UKismetMathLibrary::ComposeRotators(SightTransform.Rotator(), MeshRelativeRotation);
	const FRotator FinalRotation = ComposedRotation * -1.0f;
	
	return FTransform(FinalRotation, FinalLocation, FVector::OneVector);
}

void UBS2FunctionLibrary::ConvertNamesToVehicleTypes(const TArray<FName>& VehicleTypeNames, TArray<EVehicleType>& OutVehicleTypes)
{
	OutVehicleTypes.Empty();
	for (const FName& VehicleTypeName : VehicleTypeNames)
	{
		//FString VehicleTypeString = RowName.ToString();

		//EVehicleType VehicleType = EVehicleType::VE_None; //default/fallback
		
		int64 FoundEnum = StaticEnum<EVehicleType>()->GetValueByName(VehicleTypeName);
		if (FoundEnum != INDEX_NONE)
		{
			OutVehicleTypes.Add(static_cast<EVehicleType>(FoundEnum));
			//return FoundEnum;
		}
		
	}
}

FString UBS2FunctionLibrary::GetVehicleTypeLiteralString(EVehicleType VehicleType)
{
	// Use StaticEnum to look up the name
	UEnum* EnumPtr = StaticEnum<EVehicleType>();
	if (!EnumPtr)
	{
		return FString("Invalid");
	}

	// Get the literal enum name (e.g. "IFV", "Tank", etc.)
	return EnumPtr->GetNameStringByValue(static_cast<int64>(VehicleType));
}

#pragma region Subsystems

UDataManagerSubsystem* UBS2FunctionLibrary::GetDataSubsystem(const UObject* WorldContextObject)
{
	return WorldContextObject->GetWorld()->GetGameInstance()->GetSubsystem<UDataManagerSubsystem>();
}

UHUDSubsystem* UBS2FunctionLibrary::GetHUDSubsystem(const UObject* WorldContextObject)
{
	return WorldContextObject->GetWorld()->GetFirstLocalPlayerFromController()->GetSubsystem<UHUDSubsystem>();
}

USaveSubsystem* UBS2FunctionLibrary::GetSaveSubsystem(const UObject* WorldContextObject)
{
	return WorldContextObject->GetWorld()->GetGameInstance()->GetSubsystem<USaveSubsystem>();
}

UProjectilePoolSubsystem* UBS2FunctionLibrary::GetProjectileSystem(const UObject* WorldContextObject)
{
	return WorldContextObject->GetWorld()->GetSubsystem<UProjectilePoolSubsystem>();
}

IVehicleDataAccessor* UBS2FunctionLibrary::GetVehicleAccessor(AActor* TargetActor)
{
	return Cast<IVehicleDataAccessor>(TargetActor);
}

#pragma endregion

UCameraComponent* UBS2FunctionLibrary::CreateAndAttachCamera(UObject* Owner, USceneComponent* AttachTarget, FName SocketName)
{
	UCameraComponent* Cam = NewObject<UCameraComponent>(Owner);
	Cam->SetAutoActivate(false);
	Cam->SetActive(false);
	//Cam->bCameraMeshHiddenInGame = false;
	Cam->SetupAttachment(AttachTarget, SocketName);
	Cam->RegisterComponent();
	return Cam;
}

#pragma region WeaponFunctions

UAudioComponent* UBS2FunctionLibrary::CreateWAC(const UObject* WorldContextObject, AActor* Owner, USceneComponent* AttachTarget)
{
	UAudioComponent* NewAudioComp = NewObject<UAudioComponent>(Owner);
	NewAudioComp->SetupAttachment(AttachTarget);
	NewAudioComp->RegisterComponent();
	TSoftObjectPtr<UDA_WeaponDefaults> WeaponDefaults = GetDataSubsystem(WorldContextObject)->WeaponDefaultsDAAsset;
	NewAudioComp->SetSound(WeaponDefaults->WeaponDefaults.DefaultWeaponMetaSound.LoadSynchronous());
	NewAudioComp->bAutoActivate = false;
	return NewAudioComp;
}

float UBS2FunctionLibrary::GetFireRate(float RateOfFire)
{
	RateOfFire = 60 / RateOfFire;
	return RateOfFire;
}


bool UBS2FunctionLibrary::PerformWeaponSphereTrace(const UObject* WorldContextObject, const FTransform& StartTransform, FHitResult& OutHit, TArray<AActor*> ActorsToIgnore, float Radius, bool Debug)
{
	//if calling from any actor, WorldContextObject = this/self
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	FVector Startpoint = StartTransform.GetLocation();
	FVector GetRotationXVector = StartTransform.GetRotation().Rotator().Vector();
	FVector Endpoint = GetRotationXVector * 1000000.0f + Startpoint;			//(1000000 = 6 miles)

	FCollisionQueryParams Params;
	FCollisionShape SphereShape = FCollisionShape::MakeSphere(Radius);
	Params.AddIgnoredActors(ActorsToIgnore);
	EDrawDebugTrace::Type DebugTrace = Debug ? EDrawDebugTrace::ForOneFrame : EDrawDebugTrace::None;

	return UKismetSystemLibrary::SphereTraceSingle(
		WorldContextObject,
		Startpoint,
		Endpoint,
		Radius,
		UEngineTypes::ConvertToTraceType(ECC_Visibility),
		false,              // bTraceComplex
		ActorsToIgnore,
		DebugTrace,
		OutHit,
		true,               // bIgnoreSelf
		FLinearColor::Red,  // Trace Color
		FLinearColor::Green,// Hit Color
		0.0f                // Draw Time
	);

	//return bDidHit;
}

FTransform UBS2FunctionLibrary::GetMuzzleTransform(FName MuzzleSocketName, TWeakObjectPtr<USkeletalMeshComponent> SocketMesh)
{
	FTransform SocketTransform;
	SocketTransform = SocketMesh->GetSocketTransform(MuzzleSocketName, RTS_World);
	return SocketTransform;
}

FVector UBS2FunctionLibrary::CalculateAimDirection(FHitResult TraceData, FVector MuzzleLocation)
{
	FVector TargetPoint = TraceData.bBlockingHit ? TraceData.ImpactPoint : TraceData.TraceEnd;
	FVector RawAimDirection = (TargetPoint - MuzzleLocation).GetSafeNormal();
	return RawAimDirection;
}

FVector UBS2FunctionLibrary::GetAimDirectionFromMuzzle(FHitResult TraceData, FName MuzzleSocketName, TWeakObjectPtr<USkeletalMeshComponent> WeaponMesh)
{
	FVector MuzzleLocation = GetMuzzleTransform(MuzzleSocketName, WeaponMesh).GetLocation();
	FVector AimDirection = CalculateAimDirection(TraceData, MuzzleLocation);
	return AimDirection;
}

FVector UBS2FunctionLibrary::GetAimDirectionFromMuzzle_BP(FHitResult TraceData, FName MuzzleSocketName, USkeletalMeshComponent* WeaponMesh)
{
	return GetAimDirectionFromMuzzle(TraceData, MuzzleSocketName, TWeakObjectPtr<USkeletalMeshComponent>(WeaponMesh));
}

FSimProjectile_Runtime UBS2FunctionLibrary::CreateSimProjectile(FName MunitionID, class APlayerState* InstigatorPlayerState, FVector MuzzleLocation, float MuzzleSpeed, float GravityScale, FVector AimDirection, UProjectilePoolSubsystem* ProjectileSubsystem)
{
	FSimProjectile_Runtime NewSimulatedProjectile = FSimProjectile_Runtime();
	NewSimulatedProjectile.BaseProjectileState.MunitionID = MunitionID;
	NewSimulatedProjectile.BaseProjectileState.FireOrigin = MuzzleLocation;
	NewSimulatedProjectile.CurrentLocation = MuzzleLocation;
	NewSimulatedProjectile.CurrentVelocity = AimDirection * MuzzleSpeed;
	NewSimulatedProjectile.GravityScale = GravityScale;
	ProjectileSubsystem->AddNewSimProjectile(NewSimulatedProjectile);
	return NewSimulatedProjectile;
}

void UBS2FunctionLibrary::CalculateReload(int32 MagSize, int32 CAM, int32 CRA, int32& OutCAM, int32& OutCRA)
{
	//need max reserve ammo input?
	int32 BulletsFiredFromMag = MagSize - CAM;
	int32 BulletsToLoad = FMath::Min(BulletsFiredFromMag, CRA);
	OutCRA = CRA - BulletsToLoad;
	OutCAM = CAM + BulletsToLoad;
}

int32 UBS2FunctionLibrary::UpdateCurrentAmmoInMag(FWeaponState& CurrentWeapon, int32 AmmoDelta, int32 MagSize)
{
	//can be used for firing or resupplying logic
	CurrentWeapon.CurrentAmmoinMag = FMath::Clamp(CurrentWeapon.CurrentAmmoinMag + AmmoDelta, 0, MagSize);
	if (CurrentWeapon.CurrentAmmoinMag == 0)
	{
		CurrentWeapon.canFire = false;
	}
	UE_LOG(LogTemp, Warning, TEXT("[BS2FunctionLibrary::UpdateCurrentAmmoInMag] CAM = %d"), CurrentWeapon.CurrentAmmoinMag);
	return CurrentWeapon.CurrentAmmoinMag;
}

int32 UBS2FunctionLibrary::UpdateCurrentReserveAmmo(FWeaponState& CurrentWeapon, int32 CRADelta, int32 MRA)
{
	CurrentWeapon.CurrentReserveAmmo = FMath::Clamp(CurrentWeapon.CurrentReserveAmmo + CRADelta, 0, MRA);
	return CurrentWeapon.CurrentReserveAmmo;
}

void UBS2FunctionLibrary::HandleIfWeaponCanFire(FWeaponState& CurrentWeapon)
{
	if (CurrentWeapon.CurrentAmmoinMag <= 0 || !CurrentWeapon.isEquipped || CurrentWeapon.isReloading)
	{
		CurrentWeapon.canFire = false;
		return;
	}
	CurrentWeapon.canFire = true;
}

void UBS2FunctionLibrary::UpdateWeaponIndex(TArray<FWeaponState> Weapons, int32 InCurrentWeaponIndex, int32& OutNewWeaponIndex)
{
	OutNewWeaponIndex = (InCurrentWeaponIndex + 1) % Weapons.Num();
}

void UBS2FunctionLibrary::UpdateWACData(TWeakObjectPtr<UAudioComponent> WAC, float RPM, FWeaponAudioData WeaponAudioData)
{
	WAC.Get()->SetFloatParameter(FName("Data_RPM"), RPM);
	if (!WeaponAudioData.GunshotLoop.IsEmpty())
	{
		UpdateAudioCompArrayParameter(WAC, WeaponAudioData.GunshotLoop, FName("Data_FireLoopAudio"));
	}
	if (!WeaponAudioData.MechanicalPunch.IsEmpty())
	{
		UpdateAudioCompArrayParameter(WAC, WeaponAudioData.MechanicalPunch, FName("Data_Punch"));
	}
	if (!WeaponAudioData.MetalCycle.IsEmpty())
	{
		UpdateAudioCompArrayParameter(WAC, WeaponAudioData.MetalCycle, FName("Data_Metal"));
	}
	if (!WeaponAudioData.InteriorTail.IsEmpty())
	{
		UpdateAudioCompArrayParameter(WAC, WeaponAudioData.InteriorTail, FName("Data_Tail"));
	}
}

void UBS2FunctionLibrary::StartWAC(TWeakObjectPtr<UAudioComponent> WAC, int32 AvailableShots)
{
	WAC.Get()->Activate();
	WAC.Get()->SetIntParameter(FName("State_AvailableShots"), AvailableShots);
	WAC.Get()->SetTriggerParameter(FName("Event_StartFire"));
}

int32 UBS2FunctionLibrary::GetMaxMagSize(bool canRoundbeChambered, int32 BaseMagSize)
{
	return canRoundbeChambered ? BaseMagSize + 1 : BaseMagSize;
}

bool UBS2FunctionLibrary::GetIfWeaponCanReload(FWeaponState Weapon, bool canRoundbeChambered, int32 BaseMagSize)
{
	if (Weapon.CurrentReserveAmmo > 0 && Weapon.CurrentAmmoinMag < GetMaxMagSize(canRoundbeChambered, BaseMagSize) && !Weapon.isReloading)
	{
		return true;
	}
	return false;
}

#pragma endregion 

void UBS2FunctionLibrary::UpdateGadgetInventory(FGadgetState& Gadget, int32 InventoryDelta, int32 MaxInventoryCount)
{
	Gadget.CurrentInventory = FMath::Clamp(Gadget.CurrentInventory + InventoryDelta, 0, MaxInventoryCount);
}

void UBS2FunctionLibrary::HandleUpdateOptic(float inDefaultFOV, float inOpticMagnfication, float& OutOpticFOV, FPostProcessSettings inPostProcessData, FPostProcessSettings& OutPostProcessSettings, float& OutPostProcessWeight)
{
	//Handles both Post Process (thermal, night vision, etc.) and FOV changes for optics
	if (inPostProcessData.WeightedBlendables.Array.Num() > 0)
	{
		OutPostProcessSettings = inPostProcessData;
		OutPostProcessWeight = 1.0f;
	}
	else
	{
		OutPostProcessSettings = FPostProcessSettings();
		OutPostProcessWeight = 0.0f;
	}
	OutOpticFOV = inDefaultFOV / inOpticMagnfication;
}

void UBS2FunctionLibrary::UpdateOpticIndex(int32 TotalOptics, int32& CurrentOpticIndex)
{
	int32 NewOpticIndex = (CurrentOpticIndex + 1) % TotalOptics;
	CurrentOpticIndex = NewOpticIndex;
}

void UBS2FunctionLibrary::UpdateAudioCompArrayParameter(TWeakObjectPtr<UAudioComponent> AC, TArray<TSoftObjectPtr<USoundWave>> AudioList, FName ParameterName)
{
	TArray<UObject*> LoadedWaves;
	for (const TSoftObjectPtr<USoundWave>& SoftWave : AudioList)
	{
		TObjectPtr<USoundWave> Wave = SoftWave.LoadSynchronous();
		LoadedWaves.Add(Wave);
	}
	AC.Get()->SetObjectArrayParameter(ParameterName, LoadedWaves);
}

void UBS2FunctionLibrary::HandleApplyDamage(FBaseProjectileState BaseMunitionState, FVector CurrentLocation, FHitResult HitResult)
{
	const FMunitionDamageData& MunitionDamageData = GetDataSubsystem(HitResult.GetActor())->GetProjectileDataRow(BaseMunitionState.MunitionID)->MunitionDamageData;
	float Distance = FVector::Dist(BaseMunitionState.FireOrigin, CurrentLocation);
	FVector Direction = (CurrentLocation - BaseMunitionState.FireOrigin).GetSafeNormal();
	TObjectPtr<AActor> HitActor = HitResult.GetActor();
	
	EArmorType TargetArmorType = EArmorType::Infantry;
	float HitzoneMultiplier = 1.0f;
	
	if (HitActor->GetClass()->ImplementsInterface(UDamageable::StaticClass()))
	{
		TargetArmorType = IDamageable::Execute_GetArmorType(HitActor);
		HitzoneMultiplier = IDamageable::Execute_GetHitZoneMultiplier(HitActor, HitResult.BoneName, HitResult.ImpactPoint);
		
		const UEnum* EnumPtr = StaticEnum<EArmorType>();
		FString EnumString = EnumPtr->GetDisplayNameTextByValue((int64)TargetArmorType).ToString();
		UE_LOG(LogTemp, Warning, TEXT("[BS2FunctionLibrary::HandleApplyDamage] ArmoryType = %s, HitzoneMultiplier = %f"), *EnumString, HitzoneMultiplier);
	}
	
	float BaseDamage = MunitionDamageData.BaseDamageData.CalculateFinalBaseDamage(Distance, TargetArmorType);				
	float FinalDamage = BaseDamage * HitzoneMultiplier;
	
	switch (MunitionDamageData.DamageCategory)
	{
		case EDamageCategory::Ballistic:
		//BaseMunitionState.InstigatorPlayerState.Get()->GetOwningController()
		//BaseMunitionState.InstigatorPlayerState.Get()->GetPawn()
			UGameplayStatics::ApplyPointDamage(HitResult.GetActor(), FinalDamage, Direction, HitResult, nullptr, nullptr, UDamageType::StaticClass());
			break;
		case EDamageCategory::Explosive:
			break;
	}
}

bool UBS2FunctionLibrary::TakeDmg(float Damage, float& CurrentHealth)
{
	//damage should be final damage/damage after all modifiers to it have been calculated.
	Damage = FMath::Abs(Damage);
	UE_LOG(LogTemp, Warning, TEXT("[BS2FunctionLibrary::TakeDmg] Damage = %f"), Damage);

	bool HealthDepleted = false;
	CurrentHealth = FMath::Clamp(CurrentHealth - Damage, 0.0f, CurrentHealth);

	UE_LOG(LogTemp, Warning, TEXT("[BS2FunctionLibrary::TakeDmg] CurrentHealth = %f"), CurrentHealth);
	if (CurrentHealth == 0.0f)
	{
		HealthDepleted = true;
	}
	return HealthDepleted;
}


