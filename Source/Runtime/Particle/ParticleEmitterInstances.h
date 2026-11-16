#pragma once
#include "ParticleHelper.h"

class UParticleEmitter;
class UParticleLODLevel;
class UParticleModule;
class UParticleSystemComponent;

struct FParticleEmitterInstance
{
	FParticleEmitterInstance() = default;

	virtual ~FParticleEmitterInstance();

	FParticleEmitterInstance(const FParticleEmitterInstance&) = delete;
	FParticleEmitterInstance& operator= (const FParticleEmitterInstance&) = delete;

	virtual void Init(UParticleEmitter* InTemplate, UParticleSystemComponent* InComponent);
	virtual void Tick(float DeltaTime);

	void SpawnParticles(int32 Count, float StartTime, float Increment, const FVector& InitialLocation, const FVector& InitialVelocity);
	void KillParticles();
	void KillParticle(int32 Index);
	void KillAllParticles() { ActiveParticles = 0; }

	const FBaseParticle* GetParticle(int32 ActiveIndex) const
	{
		return reinterpret_cast<const FBaseParticle*>(ParticleData + ParticleStride * ParticleIndices[ActiveIndex]);
	}



	UParticleEmitter* SpriteTemplate = nullptr;
	UParticleSystemComponent* Component = nullptr;
	int32 CurrentLODLevelIndex = 0;
	UParticleLODLevel* CurrentLODLevel = nullptr;

	uint8* ParticleData = nullptr;		// MaxActiveParticles * ParticleStride
	uint16* ParticleIndices = nullptr;

	uint8* InstanceData = nullptr;
	int32 InstancePayloadSize = 0;
	int32 PayloadOffset = 0;			// Payload 가 시작하는 위치

	int32 ParticleSize = 0;
	int32 ParticleStride = 0;
	int32 ActiveParticles = 0;
	uint32 ParticleCounter = 0;
	int32 MaxActiveParticles = 0;
	float SpawnFraction = 0.0f;
	float EmitterTime = 0.0f;
	bool bBurstFired = false;

protected:

	bool Resize(int32 NewMaxActiveParticles);
	void FreeMemory();
	int32 CalculateSpawnCount(float DeltaTime, float& OutStartTime, float& OutIncrement);
	uint32 GetModuleOffset(UParticleModule* Module) const;

	virtual void PreSpawn(FBaseParticle* Particle, const FVector& InitialLocation, const FVector& InitialVelocity);
	virtual void PostSpawn(FBaseParticle* Particle, float Interp, float SpawnTime);
	

};

struct FParticleSpriteEmitterInstance : public FParticleEmitterInstance
{
	// 4단계에서 GetDynamicData()가 여기에 추가된다
};
