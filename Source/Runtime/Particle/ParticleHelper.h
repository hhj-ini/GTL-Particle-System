#pragma once
#include "Math/Vector.h"
#include "Math/Color.h"
#include <random>

// CPU 시뮬레이션 데이터
struct FBaseParticle
{
	FVector Location;
	FVector Velocity;
	float RelativeTime;
	float LifeTime;
	FVector BaseVelocity;
	float Rotation;
	float RotationRate;
	FVector Size;
	FLinearColor Color;
};

// 16 배수로 맞춰줘야 함.
static_assert(sizeof(FBaseParticle) % 16 == 0);

struct FParticleDataContainer
{
	int32 MemBlockSize = 0;
	int32 ParticleDataNumBytes = 0;
	int32 ParticleIndicesNumShorts = 0;
	uint8* ParticleData = nullptr;
	uint16* ParticleIndices = nullptr;
};

// GPU에서 사용하는 데이터
struct FParticleSpriteVertex
{
	FVector Position;
	float RelativeTime;
	FVector2 Size;
	float Rotation;
	float SubImageIndex;
	FVector4 Color;
};

static_assert(sizeof(FParticleSpriteVertex) == 48);

#define DECLARE_PARTICLE(Name, Address)\
FBaseParticle& Name = *((FBaseParticle*)(Address));

#define DECLARE_PARTICLE_PTR(Name, Address)		\
	FBaseParticle* Name = (FBaseParticle*) (Address);

// UParticleModule::Spawn(Onwer, Offset, SpawnTime, ParticleBase)
#define SPAWN_INIT	\
const uint32 ParticleStride = Owner->ParticleStride;\
uint32 CurrentOffset = Offset;\
FBaseParticle& Particle = *(ParticleBase);\
(void)ParticleStride; (void)CurrentOffset;

//void UParticleModuleSizeScaleBySpeed::Update(FParticleEmitterInstance* Owner, int32 Offset, float DeltaTime)
#define BEGIN_UPDATE_LOOP																			\
{																									\
	int32& ActiveParticles = Owner->ActiveParticles;													\
	uint32 CurrentOffset = Offset;																	\
	uint8* ParticleData = Owner->ParticleData;      													\
	const uint32 ParticleStride = Owner->ParticleStride;												\
	uint16* ParticleIndices = Owner->ParticleIndices;												\
	for (int32 i = ActiveParticles -1; i >= 0; i--)													\
	{																								\
		const int32 CurrentIndex = ParticleIndices[i];												\
		uint8* ParticleBase = ParticleData + CurrentIndex * ParticleStride;			\
		FBaseParticle& Particle = *((FBaseParticle*) ParticleBase);									

#define END_UPDATE_LOOP																				\
		CurrentOffset = Offset;																		\
	}																								\
}

namespace ParticleRandom
{
	inline float Range(float Min, float Max)
	{
		if (!(Min < Max))
		{
			return Min;
		}

		static thread_local std::mt19937 Generator{ std::random_device{}() };
		return std::uniform_real_distribution<float>(Min, Max)(Generator);
	}

	inline FVector Range(const FVector& Min, const FVector& Max)
	{
		return FVector(Range(Min.X, Max.X), Range(Min.Y, Max.Y), Range(Min.Z, Max.Z));
	}
}