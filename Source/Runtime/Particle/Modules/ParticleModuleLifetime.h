#pragma once
#include "Particle/Modules/ParticleModule.h"
#include "ParticleModuleLifetime.generated.h"

UCLASS()
class UParticleModuleLifetime : public UParticleModule
{
	GENERATED_BODY()

public:
	UParticleModuleLifetime() { bSpawnModule = true; }
	virtual void Spawn(FParticleEmitterInstance* Owner, int32 Offset, float SpawnTime, FBaseParticle* ParticleBase) override;

	UPROPERTY(EditAnywhere)
	float MinLifetime = 1.0f;
	UPROPERTY(EditAnywhere)
	float MaxLifetime = 2.0f;
};