// coding in UE almost makes me wish for a nuclear winter


#include "Core/Gadgets/Gadget_Base.h"
#include "Components/SphereComponent.h"
#include "Components/BoxComponent.h"
#include "Data/Items/Gadgets/Data_Gadget.h"
#include "Utilities/BS2FunctionLibrary.h"
#include "Utilities/DataManagerSubsystem.h"

// Sets default values
AGadget_Base::AGadget_Base()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	GadgetMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GadgetMeshComponent"));
	SetRootComponent(GadgetMeshComponent); 
}

// Called every frame
void AGadget_Base::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called when the game starts or when spawned
void AGadget_Base::BeginPlay()
{
	Super::BeginPlay();
	
	check (!GadgetInstanceStartingData.GadgetID.IsNone());
	if (UBS2FunctionLibrary::GetDataSubsystem(this)->IsDataReady())
	{
		Init_GadgetData();
	}
	else
	{
		UBS2FunctionLibrary::GetDataSubsystem(this)->OnDataReady.AddDynamic(this, &AGadget_Base::Init_GadgetData);
	}
}

void AGadget_Base::Init_GadgetData()
{
	GadgetData = UBS2FunctionLibrary::GetDataSubsystem(this)->GetGadgetDataRow(GadgetInstanceStartingData.GadgetID);
	
	Init_Gadget();

}

void AGadget_Base::Init_Gadget()
{
	Init_GadgetMesh();
	if (GadgetData->GadgetInstanceData.bHasTriggerVolume)
	{
		Init_TriggerVolume();
	}
}

void AGadget_Base::Init_GadgetMesh()
{
	GadgetMeshComponent->SetStaticMesh(GadgetData->GadgetMesh.LoadSynchronous());
}

void AGadget_Base::Init_TriggerVolume()
{
	const FGadgetTriggerData& TriggerData = GadgetData->GadgetInstanceData.TriggerData;
	
	switch (TriggerData.ShapeType)
	{
		case ECustomCollisionShapeType::Sphere:
		{
			TObjectPtr<USphereComponent> Sphere = NewObject<USphereComponent>(this);
			Sphere->SetSphereRadius(TriggerData.SphereRadius);
			GadgetState.TriggerVolume = Sphere;
			break;
		}
		case ECustomCollisionShapeType::Box:
		{
			TObjectPtr<UBoxComponent> Box = NewObject<UBoxComponent>(this);
			Box->SetBoxExtent(TriggerData.BoxExtent);
			GadgetState.TriggerVolume = Box;
			break;
		}
	}
	
	GadgetState.TriggerVolume->SetupAttachment(GadgetMeshComponent);
	GadgetState.TriggerVolume->RegisterComponent();
}

void AGadget_Base::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
}

void AGadget_Base::OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
}



