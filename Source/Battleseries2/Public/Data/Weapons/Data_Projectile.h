// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "NiagaraSystem.h"
#include "Data/Weapons/ProjectileEnums.h"
#include "Data_Projectile.generated.h"

/**
 * actor projectiles only
 */

USTRUCT(BlueprintType)
struct FProjectileClassificationData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ProjectileDisplayName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EProjectileType ProjectileType = EProjectileType::Bullet;
};

USTRUCT(BlueprintType)
struct FProjectileVisualData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UStaticMesh> ProjectileMesh = nullptr;

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
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Guidance")
	EProjectileGuidanceMethod BehaviorType = EProjectileGuidanceMethod::BallisticTrajectory; // The algorithm to use (Ballistic, Homing, WireGuided, etc.)

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
struct FProjectileData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FProjectileClassificationData ProjectileClassificationData = FProjectileClassificationData();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FProjectileVisualData ProjectileVisualData = FProjectileVisualData();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FProjectileBehaviorData ProjectileBehaviorData = FProjectileBehaviorData();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray <FProjectileFlightStage> ProjectileFlightPlan;

};