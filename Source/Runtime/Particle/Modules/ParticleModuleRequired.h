#pragma once
#include "ParticleModule.h"
#include "ParticleModuleRequired.generated.h"

class UMaterial;

UCLASS()
class UParticleModuleRequired : public UParticleModule
{
	GENERATED_BODY()

public:
	UParticleModuleRequired();

	UMaterial* Material = nullptr;

	UPROPERTY(EditAnywhere)
	float EmitterDuration = 0.0f;

	UPROPERTY(EditAnywhere)
	int32 SubImagesHorizontal = 1;
	UPROPERTY(EditAnywhere)
	int32 SubImagesVertical = 1;
};