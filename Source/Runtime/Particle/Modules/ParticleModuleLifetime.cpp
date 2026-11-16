#include "EnginePCH.h"
#include "ParticleModuleLifetime.h"
#include "Particle/ParticleHelper.h"
#include "Particle/ParticleEmitterInstances.h"

void UParticleModuleLifetime::Spawn(FParticleEmitterInstance* Owner, int32 Offset, float SpawnTime, FBaseParticle* ParticleBase)
{
	SPAWN_INIT;
	Particle.LifeTime = FMath::Max(ParticleRandom::Range(MinLifetime, MaxLifetime), 0.0001f);
}