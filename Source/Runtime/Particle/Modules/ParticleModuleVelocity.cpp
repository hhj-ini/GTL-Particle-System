#include "EnginePCH.h"
#include "Particle/Modules/ParticleModuleVelocity.h"
#include "Particle/ParticleEmitterInstances.h"

void UParticleModuleVelocity::Spawn(FParticleEmitterInstance* Owner, int32 Offset, float SpawnTime, FBaseParticle* ParticleBase)
{
	SPAWN_INIT;
	const FVector Velocity = ParticleRandom::Range(StartVelocityMin, StartVelocityMax);
	Particle.Velocity += Velocity;
	Particle.BaseVelocity += Velocity;
}