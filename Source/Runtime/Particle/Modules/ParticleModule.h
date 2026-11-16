#pragma once
#include "UObject/Object.h"
#include "UObject/Class.h"
#include "ParticleModule.generated.h"

struct FParticleEmitterInstance;
struct FBaseParticle;

UCLASS()
class UParticleModule : public UObject
{
	GENERATED_BODY()
public:
	
	// 파티클이 생길 때 한번 호출됨. 
	virtual void Spawn(FParticleEmitterInstance* Owner, int32 Offset, float SpawnTime, FBaseParticle* ParticleBase){}

	virtual void Update(FParticleEmitterInstance* Owner, int32 Offset, float DeltaTime){}

	virtual uint32 RequiredBytes() const { return 0; }

	bool IsSpawnModule() const { return bSpawnModule; }
	bool IsUpdateModule() const { return bUpdateModule; }

	UPROPERTY(EditAnywhere)
	bool bEnabled = true;

	bool bSpawnModule = false;
	bool bUpdateModule = false;
};