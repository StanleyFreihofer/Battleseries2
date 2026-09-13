// coding in UE almost makes me wish for a nuclear winter


#include "Core/Gadgets/Gadget_Base.h"

// Sets default values
AGadget_Base::AGadget_Base()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AGadget_Base::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AGadget_Base::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

