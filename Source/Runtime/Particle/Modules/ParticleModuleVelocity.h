#pragma once
#include "Particle/Modules/ParticleModule.h"
#include "Math/Vector.h"
#include "ParticleModuleVelocity.generated.h"

UCLASS()
class UParticleModuleVelocity : public UParticleModule
{
	GENERATED_BODY()

public:
	UParticleModuleVelocity() { bSpawnModule = true; }
	virtual void Spawn(FParticleEmitterInstance* Owner, int32 Offset, float SpawnTime, FBaseParticle* ParticleBase) override;

	UPROPERTY(EditAnywhere)
	FVector StartVelocityMin = FVector(-0.5f, -0.5f, 1.0f);
	UPROPERTY(EditAnywhere)
	FVector StartVelocityMax = FVector(0.5f, 0.5f, 2.0f);
};