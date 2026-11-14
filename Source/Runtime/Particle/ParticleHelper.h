#pragma once
#include "Math/Vector.h"
#include "Math/Color.h"

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

struct FParticleDataContainer
{
	int32 MemBlockSize;
	int32 ParticleDataNumBytes;
	int32 ParticleIndicesNumShorts;
	uint8* ParticleData;
	uint16* ParticleIndices;
};

// 16 배수로 맞춰줘야 함.
static_assert(sizeof(FBaseParticle) % 16 == 0);