// coding in UE almost makes me wish for a nuclear winter

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneCaptureComponent2D.h"
#include "ScopeCaptureComponent.generated.h"

/**
 * 
 */
UCLASS()
class BATTLESERIES2_API UScopeCaptureComponent : public USceneCaptureComponent2D
{
	GENERATED_BODY()
	// render as the owning character's view, so bOwnerNoSee / bOnlyOwnerSee behave like the player camera
	virtual const AActor* GetViewOwner() const override { return GetOwner();};
	
};
