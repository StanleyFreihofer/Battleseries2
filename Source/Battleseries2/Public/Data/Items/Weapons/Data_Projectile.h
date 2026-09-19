// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "NiagaraSystem.h"
#include "Data/Items/Weapons/ProjectileEnums.h"
#include "Data/Core/Combat/Data_Combat.h"
#include "Data_Projectile.generated.h"

/**
 * munitions/projectiles
 */

USTRUCT(BlueprintType)
struct FMunitionVisualData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UStaticMesh> MunitionMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UNiagaraSystem> RocketExhaust = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector ExhaustLocationOffset = FVector();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UNiagaraSystem> ImpactVFX = nullptr;

	//vfx
	//tracer?
	//UI?
	//audio?
};

USTRUCT(BlueprintType)
struct FProjectileFunctionalityData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool UseProjectilePOV = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool ControlProjectile = false;
};

USTRUCT(BlueprintType)
struct FProjectileBehaviorData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EProjectileLimit ProjectileLimit = EProjectileLimit::NoLimits;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float InitialLifeSpan = 0.0f;
};

USTRUCT(BlueprintType)
struct FFlightParameters
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float InitialSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float MaxSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Acceleration = 0.0f; 

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float GravityScale = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float PitchForce = 0.0f;
};

USTRUCT(BlueprintType)
struct FProjectileFlightStage
{
	GENERATED_BODY()

	// --- BEHAVIOR ---
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Guidance", meta = (ToolTip = "The algorithm to use (Ballistic, Homing, WireGuided, etc.)"))
	EProjectileGuidanceMethod BehaviorType = EProjectileGuidanceMethod::BallisticTrajectory;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Guidance")
	FFlightParameters GuidanceParams = FFlightParameters(); // e.g., Thrust duration, Turn Rate, Max Speed

	// --- PRIMARY TRANSITION ---
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transition")
	ETransitionCondition PrimaryTransitionCondition = ETransitionCondition(); // When to move to the next stage

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transition")
	float RequiredValue = 0.0f; // e.g., 5.0 seconds, 100 meters, 0.8 lock quality

	// --- CONTINGENCY (FAILURE) ---
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Contingency")
	EContingencyType FailureTrigger = EContingencyType::None; // If Lock is lost, Fuel is empty, etc.

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Contingency")
	int32 ContingencyStageIndex = INDEX_NONE; // The index to jump to on failure (e.g., jump to Self-Destruct stage)
};

USTRUCT(BlueprintType)
struct FActorProjectileData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EProjectileType ProjectileType = EProjectileType::Bullet;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FProjectileBehaviorData ProjectileBehaviorData = FProjectileBehaviorData();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray <FProjectileFlightStage> ProjectileFlightPlan;
};

USTRUCT(BlueprintType)
struct FBaseMunitionDamageData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "weapons 'raw' damage without any falloff"))
	float BaseDamage = 0.0f;				
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<EArmorType, float> ArmorDamageMultipliers;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UCurveFloat> DamageDropoffCurve = nullptr;
	
	//penetration

	float GetDamageAtDistance(float Distance) const
	{
		if (DamageDropoffCurve)
		{
			// If the curve exists, it defines the Max/Min behavior entirely
			return DamageDropoffCurve->GetFloatValue(Distance);
		}
		return BaseDamage;
	}
	
	float CalculateFinalBaseDamage(float Distance, EArmorType ArmorTypeHit) const
	{
		const float Damage = GetDamageAtDistance(Distance);
		const float ArmorDmgMultiplier = ArmorDamageMultipliers.FindRef(ArmorTypeHit);
		return Damage * ArmorDmgMultiplier;
	}
	
	//radial/splash damage?
};

USTRUCT(BlueprintType)
struct FExplosiveDamageData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "full BaseDamage is dealt to anything within this radius of the blast origin"))
	float InnerRadius = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "damage falls off between InnerRadius and this — nothing takes damage beyond it"))
	float OuterRadius = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "damage dealt exactly at OuterRadius, before it drops to zero"))
	float MinimumDamage = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "1.0 = linear falloff between the radii; higher = sharper dropoff near InnerRadius"))
	float DamageFalloffExponent = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "multiplies BaseDamage specifically when this munition directly impacts a target, rather than just being in blast range"))
	float DirectHitDamageMultiplier = 1.0f;
};

USTRUCT(BlueprintType)
struct FMunitionDamageData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EDamageCategory DamageCategory = EDamageCategory::Ballistic;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBaseMunitionDamageData BaseDamageData = FBaseMunitionDamageData();
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "DamageCategory == EDamageCategory::Explosive", EditConditionHides))
	FExplosiveDamageData ExplosiveDamageData = FExplosiveDamageData();
};

USTRUCT(BlueprintType)
struct FProjectileData : public FTableRowBase
{
	GENERATED_BODY()
		
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText MunitionDisplayName = FText::GetEmpty();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)	
	EMunitionType MunitionType = EMunitionType::SimProjectile;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FMunitionVisualData MunitionVisualData = FMunitionVisualData();
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FMunitionDamageData MunitionDamageData = FMunitionDamageData();
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditConition = "MunitionType == EMunitionType::ActorProjectile", EditConditionHides))
	FActorProjectileData ActorProjectileData = FActorProjectileData();

};