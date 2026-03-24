#pragma once

#include "CoreMinimal.h"
#include "PlayerEnums.generated.h"

/**
* enums for the player/controller
* remember the lifetime of the controller precedes the life of a character it possesses
**/

UENUM(BlueprintType)                //only matters within the context of a level/match (not main menu for example)
enum class EPlayerLifeState : uint8
{
    InSpawnScreen,   // Looking at the map, picking a class
    Deploying,       // The 2-second "zoom-in" camera transition
    Alive,           // Currently possessing a Soldier or Vehicle
    Dead,            // Just died, watching the ragdoll/killcam
    InSpectator      // Just watching the match (no intent to spawn)
};
