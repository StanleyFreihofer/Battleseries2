// coding in UE almost makes me wish for a nuclear winter

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Data/Items/Gadgets/GadgetTypes.h"
#include "Gadget_Base.generated.h"

struct FGadgetData;

USTRUCT(BlueprintType)
struct FGadgetInstanceStartingData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName GadgetID = NAME_None;
	
	//UPROPERTY(EditAnywhere, BlueprintReadWrite)
	//bool bAutoUse = false;
};

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
	
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = "true"), Category = "__Instance")
	FGadgetInstanceStartingData GadgetInstanceStartingData = FGadgetInstanceStartingData();
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FGadgetInstanceState GadgetState = FGadgetInstanceState();
	
	const FGadgetData* GadgetData = nullptr;


	
	UFUNCTION(BlueprintCallable)
	void Init_GadgetData();
	UFUNCTION(BlueprintCallable)
	void Init_Gadget();
	UFUNCTION(BlueprintCallable)
	void Init_GadgetMesh();
	UFUNCTION(BlueprintCallable)
	void Init_TriggerVolume();
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	UFUNCTION()
	void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
