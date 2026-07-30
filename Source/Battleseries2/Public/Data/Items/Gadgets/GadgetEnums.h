#pragma once

#include "CoreMinimal.h"
#include "GadgetEnums.generated.h"

UENUM(BlueprintType)
enum class EGadgetType : uint8
{
	Weapon		UMETA(DisplayName = "Weapon", ToolTip = "Gadget that is a weapon"),
	Vehicle		UMETA(DisplayName = "Vehicle", ToolTip = "Gadget that is a vehicle (EOD Bot, Drone, etc)"),
	Gadget		UMETA(DisplayName = "Gadget", ToolTip = "Every other type of gadget")
};