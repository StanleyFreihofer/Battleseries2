// coding in UE almost makes me wish for a nuclear winter

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Gadget_Base.generated.h"

struct FGadgetData;

UCLASS()
class BATTLESERIES2_API AGadget_Base : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGadget_Base();
	
	//Components
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	UStaticMeshComponent* GadgetMeshComponent = nullptr;
	
	
	
	const FGadgetData* GadgetData = nullptr;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
