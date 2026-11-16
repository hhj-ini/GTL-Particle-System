#include "EnginePCH.h"
#include "ParticleEmitterInstances.h"
#include "ParticleEmitter.h"
#include "ParticleLODLevel.h"
#include "Modules/ParticleModule.h"
#include "Modules/ParticleModuleSpawn.h"
#include "Modules/ParticleModuleRequired.h"
#include "Components/ParticleSystemComponent.h"
#include <malloc.h>
#include <cstring>

namespace
{
	constexpr int32 MaxParticlesPerEmitter = 0xFFFF;
	int32 AlignUp(int32 Value, int32 Alignment)
	{
		return (Value + Alignment - 1) & ~(Alignment - 1);
	}
}

FParticleEmitterInstance::~FParticleEmitterInstance()
{
	FreeMemory();
}

void FParticleEmitterInstance::Init(UParticleEmitter* InTemplate, UParticleSystemComponent* InComponent)
{
	FreeMemory();
	SpriteTemplate = InTemplate;
	Component = InComponent;
	CurrentLODLevelIndex = 0;
	CurrentLODLevel = SpriteTemplate ? SpriteTemplate->GetLODLevel(0) : nullptr;
	if (!CurrentLODLevel)
	{
		return;
	}

	SpriteTemplate->CacheEmitterModuleInfo();
	ParticleSize = SpriteTemplate->ParticleSize;
	ParticleStride = AlignUp(ParticleSize, 16);
	PayloadOffset = sizeof(FBaseParticle);

	ParticleCounter = 0;
	SpawnFraction = 0.0f;
	EmitterTime = 0.0f;
	bBurstFired = false;

	Resize(FMath::Max(SpriteTemplate->InitialAllocationCount, 1));
}

void FParticleEmitterInstance::Tick(float DeltaTime)
{
	if (!CurrentLODLevel || !ParticleData || DeltaTime <= 0.0f) return;
	EmitterTime += DeltaTime;

	// 1. 나이 먹이기 + 속도 초기화 (모듈이 매 프레임 BaseVelocity 위에 더한다)
	for (int32 i = 0; i < ActiveParticles; ++i)
	{
		DECLARE_PARTICLE(Particle, ParticleData + ParticleStride * ParticleIndices[i]);
		Particle.RelativeTime += DeltaTime / Particle.LifeTime;
		Particle.Velocity = Particle.BaseVelocity;
	}

	// 2. 수명이 끝난 파티클 제거
	KillParticles();

	// 3. Update 모듈
	for (UParticleModule* Module : CurrentLODLevel->UpdateModules)
		Module->Update(this, static_cast<int32>(GetModuleOffset(Module)), DeltaTime);

	// 4. 위치·회전 적분
	for (int32 i = 0; i < ActiveParticles; ++i)
	{
		DECLARE_PARTICLE(Particle, ParticleData + ParticleStride * ParticleIndices[i]);
		Particle.Location += Particle.Velocity * DeltaTime;
		Particle.Rotation += Particle.RotationRate * DeltaTime;
	}

	// 5. 새 파티클 생성 (이번 프레임 안에서 태어난 시점만큼 PostSpawn이 앞당겨 준다)
	const FVector SpawnLocation = Component ? Component->GetWorldLocation() : FVector::ZeroVector;
	UParticleModuleSpawn* SpawnModule = CurrentLODLevel->SpawnModule;
	if (SpawnModule && !bBurstFired && SpawnModule->BurstCount > 0)
	{
		SpawnParticles(SpawnModule->BurstCount, 0.0f, 0.0f, SpawnLocation, FVector::ZeroVector);
		bBurstFired = true;
	}

	float StartTime = 0.0f;
	float Increment = 0.0f;
	const int32 SpawnCount = CalculateSpawnCount(DeltaTime, StartTime, Increment);
	if (SpawnCount > 0)
		SpawnParticles(SpawnCount, StartTime, Increment, SpawnLocation, FVector::ZeroVector);
}

void FParticleEmitterInstance::SpawnParticles(int32 Count, float StartTime, float Increment, const FVector& InitialLocation, const FVector& InitialVelocity)
{
	if (Count <= 0) return;

	if (ActiveParticles + Count > MaxActiveParticles)
	{
		Resize(FMath::Max(MaxActiveParticles * 2, ActiveParticles + Count));
		Count = FMath::Min(Count, MaxActiveParticles - ActiveParticles);
		if (Count <= 0) return;
	}

	for (int32 i = 0; i < Count; ++i)
	{
		const float SpawnTime = StartTime - static_cast<float>(i) * Increment;

		const int32 CurrentIndex = ParticleIndices[ActiveParticles];  // 첫 번째 빈 칸
		DECLARE_PARTICLE_PTR(Particle, ParticleData + ParticleStride * CurrentIndex);

		PreSpawn(Particle, InitialLocation, InitialVelocity);
		for (UParticleModule* Module : CurrentLODLevel->SpawnModules)
			Module->Spawn(this, static_cast<int32>(GetModuleOffset(Module)), SpawnTime, Particle);
		PostSpawn(Particle, 1.0f, SpawnTime);

		++ActiveParticles;
		++ParticleCounter;
	}
}

void FParticleEmitterInstance::KillParticles()
{
	for (int32 i = ActiveParticles - 1; i >= 0; --i)
	{
		const int32 CurrentIndex = ParticleIndices[i];
		const FBaseParticle* Particle = reinterpret_cast<const FBaseParticle*>(ParticleData + ParticleStride * CurrentIndex);
		if (Particle->RelativeTime >= 1.0f)
		{
			ParticleIndices[i] = ParticleIndices[ActiveParticles - 1];
			ParticleIndices[ActiveParticles - 1] = static_cast<uint16>(CurrentIndex);
			--ActiveParticles;
		}
	}
}

void FParticleEmitterInstance::KillParticle(int32 Index)
{	// swap and pop
	if (Index < 0 || Index >= ActiveParticles)
	{
		return;
	}
	const uint16 KilledIndex = ParticleIndices[Index];
	ParticleIndices[Index] = ParticleIndices[ActiveParticles - 1];
	ParticleIndices[ActiveParticles - 1] = KilledIndex;
	--ActiveParticles;
}

bool FParticleEmitterInstance::Resize(int32 NewMax)
{
	NewMax = FMath::Min(NewMax, MaxParticlesPerEmitter);
	if (NewMax <= MaxActiveParticles) return true;

	uint8* NewData = static_cast<uint8*>(_aligned_malloc(static_cast<size_t>(ParticleStride) * NewMax, 16));
	if (!NewData) return false;
	uint16* NewIndices = new uint16[NewMax];

	if (ParticleData)    std::memcpy(NewData, ParticleData, static_cast<size_t>(ParticleStride) * MaxActiveParticles);
	if (ParticleIndices) std::memcpy(NewIndices, ParticleIndices, sizeof(uint16) * MaxActiveParticles);
	for (int32 i = MaxActiveParticles; i < NewMax; ++i)
		NewIndices[i] = static_cast<uint16>(i);   // 새로 생긴 칸은 빈 칸

	_aligned_free(ParticleData);
	delete[] ParticleIndices;
	ParticleData = NewData;
	ParticleIndices = NewIndices;
	MaxActiveParticles = NewMax;
	return true;
}

void FParticleEmitterInstance::FreeMemory()
{
	_aligned_free(ParticleData);
	ParticleData = nullptr;
	delete[] ParticleIndices;
	ParticleIndices = nullptr;
	MaxActiveParticles = 0;
	ActiveParticles = 0;
}

int32 FParticleEmitterInstance::CalculateSpawnCount(float DeltaTime, float& OutStartTime, float& OutIncrement)
{
	OutStartTime = 0.0f;
	OutIncrement = 0.0f;

	const UParticleModuleRequired* Required = CurrentLODLevel->RequiredModule;
	if (Required && Required->EmitterDuration > 0.0f && EmitterTime > Required->EmitterDuration)
		return 0;

	const UParticleModuleSpawn* SpawnModule = CurrentLODLevel->SpawnModule;
	const float Rate = SpawnModule ? FMath::Max(SpawnModule->Rate, 0.0f) : 0.0f;
	if (Rate <= 0.0f) return 0;

	// UE와 같은 방식: 소수 부분을 다음 프레임으로 넘겨 프레임레이트와 무관하게 Rate를 맞춘다
	const float NewLeftover = SpawnFraction + DeltaTime * Rate;
	const int32 Number = static_cast<int32>(std::floor(NewLeftover));
	OutIncrement = 1.0f / Rate;
	OutStartTime = DeltaTime + SpawnFraction * OutIncrement - OutIncrement; // 첫 파티클이 태어난 뒤 흐른 시간
	SpawnFraction = NewLeftover - static_cast<float>(Number);
	return Number;
}

uint32 FParticleEmitterInstance::GetModuleOffset(UParticleModule* Module) const
{
	if (!SpriteTemplate) return 0;
	const uint32* Found = SpriteTemplate->ModuleOffsetMap.Find(Module);
	return Found ? *Found : 0;
}

void FParticleEmitterInstance::PreSpawn(FBaseParticle* Particle, const FVector& InitialLocation, const FVector& InitialVelocity)
{
	std::memset(Particle, 0, static_cast<size_t>(ParticleStride));   // Payload까지 0으로
	Particle->Location = InitialLocation;
	Particle->Velocity = InitialVelocity;
	Particle->BaseVelocity = InitialVelocity;
	Particle->Size = FVector(1.0f, 1.0f, 1.0f);
	Particle->Color = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);
	Particle->LifeTime = 1.0f;   // Lifetime 모듈이 없을 때의 기본값
}

void FParticleEmitterInstance::PostSpawn(FBaseParticle* Particle, float Interp, float SpawnTime)
{
	// 프레임 도중에 태어났으면 그만큼 나이와 위치를 앞당긴다
	Particle->RelativeTime = SpawnTime / Particle->LifeTime;
	Particle->Location += Particle->Velocity * SpawnTime;
}
