#pragma once

#include "CoreMinimal.h"
#include "ProjectileEnums.generated.h"

//rename stuff to munition?

UENUM(BlueprintType)
enum class EProjectileType : uint8
{
	Bullet					UMETA(DisplayName = "Bullet"),		//sim		init velocity set by weapon muzzle velocity
	Pellet					UMETA(DisplayName = "Pellet"),		//sim		init velocity set by weapon muzzle velocity
	Shell					UMETA(DisplayName = "Shell"),		//sim		init velocity set by weapon muzzle velocity
	Rocket					UMETA(DisplayName = "Rocket"),		//actor		init velocity comes from self
	Missile					UMETA(DisplayName = "Missile"),		//actor		init velocity comes from self
	Bomb					UMETA(DisplayName = "Bomb")			//actor
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
	GuideToTarget				UMETA(DisplayName = "Guide To Target/Homing/Lock On"),		//(and wire guided)
	PitchToAltitude				UMETA(DisplayName = "Pitch To Altitude"),
	WireGuided                  UMETA(DisplayName = "Player Controlled: Wire Guided"),
	FullControl					UMETA(DisplayName = "Player Controlled: Full Control"),
	//drop

	// Terminal Behavior
	SelfDestruct                UMETA(DisplayName = "Self-Destruct")
};