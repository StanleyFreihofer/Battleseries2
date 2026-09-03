#pragma once

#include "CoreMinimal.h"
#include "ItemStructs.generated.h"

USTRUCT(BlueprintType)
struct FHeldItemAnimData_Base
{
	GENERATED_BODY()
	
	// Equip / Unequip
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Actions | Montages")
    TSoftObjectPtr<UAnimMontage> EquipMontage = nullptr;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Actions | Montages")
    TSoftObjectPtr<UAnimMontage> UnequipMontage = nullptr;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Actions | Montages")
    TSoftObjectPtr<UAnimMontage> InitialEquipMontage = nullptr;

    // Movement | Blendspaces
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement | Blendspaces")
    TSoftObjectPtr<UBlendSpace> MovementBlendspace = nullptr;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement | Blendspaces")
    TSoftObjectPtr<UBlendSpace> ProneBlendspace = nullptr;

    // Movement
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
    TSoftObjectPtr<UAnimSequence> FallingLoop = nullptr;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
    TSoftObjectPtr<UAnimSequence> Idle = nullptr;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
    TSoftObjectPtr<UAnimSequence> JumpLoop = nullptr;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
    TSoftObjectPtr<UAnimSequence> SprintLoop = nullptr;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
    TSoftObjectPtr<UAnimSequence> SlideLoop = nullptr;

    // Transitions | Sequences — movement-state enter/exit, unrelated to firing/reloading
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transitions | Sequences")
    TSoftObjectPtr<UAnimSequence> TacSprintExit = nullptr;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transitions | Sequences")
    TSoftObjectPtr<UAnimSequence> TacSprintEnter = nullptr;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transitions | Sequences")
    TSoftObjectPtr<UAnimSequence> SprintEnter = nullptr;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transitions | Sequences")
    TSoftObjectPtr<UAnimSequence> SprintExit = nullptr;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transitions | Sequences")
    TSoftObjectPtr<UAnimSequence> TacSprintLoopAdditive = nullptr;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transitions | Sequences")
    TSoftObjectPtr<UAnimSequence> SlideEnterAdditive = nullptr;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transitions | Sequences")
    TSoftObjectPtr<UAnimSequence> JumpEnterAdditive = nullptr;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transitions | Sequences")
    TSoftObjectPtr<UAnimSequence> JumpExitAdditive = nullptr;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transitions | Sequences")
    TSoftObjectPtr<UAnimSequence> TacSprintExitIdleAdditive = nullptr;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transitions | Sequences")
    TSoftObjectPtr<UAnimSequence> ProneEnterAdditive = nullptr;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transitions | Sequences")
    TSoftObjectPtr<UAnimSequence> ProneExitAdditive = nullptr;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transitions | Sequences")
    TSoftObjectPtr<UAnimSequence> SlidingExit = nullptr;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transitions | Sequences")
    TSoftObjectPtr<UAnimSequence> TacSprintEnterIdle = nullptr;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transitions | Sequences")
    TSoftObjectPtr<UAnimSequence> CrouchEnter = nullptr;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transitions | Sequences")
    TSoftObjectPtr<UAnimSequence> CrouchExit = nullptr;
};