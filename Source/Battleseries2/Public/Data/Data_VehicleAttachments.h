#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Data_VehicleAttachments.generated.h"

USTRUCT(BlueprintType)
struct FAttachmentCamoData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<int32, UMaterialInstance*> MaterialElementIndexMap;		//maps the MI with the material slot on the mesh 
};

USTRUCT(BlueprintType)
struct FVehicleAttachmentData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText AttachmentDisplayName = FText();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText AttachmentDescription = FText();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<USkeletalMesh> Attachment_SKM = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftClassPtr<UAnimInstance> Attachment_AnimClass = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UStaticMesh> Attachment_SM = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FName, FAttachmentCamoData> AvailableCamos;		//CamoID, MI for that attachment
};
