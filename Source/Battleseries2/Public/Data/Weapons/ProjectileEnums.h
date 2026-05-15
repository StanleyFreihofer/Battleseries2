#pragma once

#include "CoreMinimal.h"
#include "ProjectileEnums.generated.h"

//rename stuff to munition?

UENUM(BlueprintType)
enum class EProjectileType : uint8
{
	Bullet					UMETA(DisplayName = "Bullet"),		
	Pellet					UMETA(DisplayName = "Pellet"),		
	Shell					UMETA(DisplayName = "Shell"),	
	Rocket					UMETA(DisplayName = "Rocket"),		
	Missile					UMETA(DisplayName = "Missile"),	
	Bomb					UMETA(DisplayName = "Bomb")		
};

UENUM(BlueprintType)
enum class EProjectileLimit : uint8
{
	NoLimits				UMETA(DisplayName = "Default/No Limits"),
	LimitedTime				UMETA(DisplayName = "Limited Life Span/Time"),
	LimitedRange			UMETA(DisplayName = "Limited Range")
};

UENUM(BlueprintType)
enum class EDetonationType : uint8
{
	DetOnImpact							UMETA(DisplayName = "Detonate On Impact"),
	Airbust								UMETA(DisplayName = "Airburst"),
	Delay								UMETA(DisplayName = "Delay")
};

UENUM(BlueprintType)				//(What CAUSES the munition to change stage)
enum class ETransitionCondition : uint8
{
	OnImpact					UMETA(DisplayName = "On Impact"),
	LimitedTime					UMETA(DisplayName = "Limited Time"),
	LimitedRange				UMETA(DisplayName = "Limited Range", ToolTip = "Range FROM an initial location"),
	RangeToTarget				UMETA(DisplayName = "Range To Target", ToolTip = "Range TO TARGET from current location"),
	Proximity2D					UMETA(DisplayName = "Proximity 2D", ToolTip = "if directly above target"),
	TargetAcquired				UMETA(DisplayName = "Target Acquired")
};

UENUM(BlueprintType)				//(What FAILURE causes the munition to change stage)
enum class EContingencyType : uint8
{
	None						UMETA(DisplayName = "None"),
	LockLost					UMETA(DisplayName = "Lock Lost"),
	OutOfFuel					UMETA(DisplayName = "Out Of Fuel"),
	MaxLifeSpanExceeded			UMETA(DisplayName = "Life Span Exceeded")
};

UENUM(BlueprintType)							//what the munition does
enum class EProjectileGuidanceMethod : uint8
{
	BallisticTrajectory         UMETA(DisplayName = "Default/Ballistic"),			//embedded actor data i guess
	PitchToAltitude				UMETA(DisplayName = "Pitch To Altitude"),
	GuideToTarget				UMETA(DisplayName = "Guide To Target/Homing/Lock On"),		
	ManualGuideToPoint          UMETA(DisplayName = "Manual Guide to Point", ToolTip = "Guides to a vector point via manual guidance/wire guided (TOW)"),
	AutoGuideToPoint			UMETA(DisplayName = "Auto Guide to Point", ToolTip = "Guides to a point via auto/self-guidance"),
	FullControl					UMETA(DisplayName = "Player Controlled"),
	Drop						UMETA(DisplayName = "Drop", ToolTip = "Dumb drop, uses sim physics on projectile mesh, projectile movement comp turned off"),

	// Terminal Behavior
	SelfDestruct                UMETA(DisplayName = "Self-Destruct")
};