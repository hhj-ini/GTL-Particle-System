#pragma once
#include "UObject/Object.h"
#include "UObject/Class.h"
#include "Containers/Array.h"
#include "Containers/Map.h"
#include "ParticleEmitter.generated.h"

class UParticleLODLevel;
class UParticleModule;

UCLASS()
class UParticleEmitter : public UObject
{
	GENERATED_BODY()

public:

	void CacheEmitterModuleInfo();
	UParticleLODLevel* GetLODLevel(int32 Index) const;

	UPROPERTY(EditAnywhere)
	int32 InitialAllocationCount = 64;

	TArray<UParticleLODLevel*> LODLevels;

	int32 ParticleSize = 0;
	TMap<UParticleModule*, uint32> ModuleOffsetMap;
};