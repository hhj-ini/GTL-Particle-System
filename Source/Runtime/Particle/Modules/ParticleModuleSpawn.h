#pragma once
#include "ParticleModule.h"
#include "ParticleModuleSpawn.generated.h"

// 생성 빈도
UCLASS()
class UParticleModuleSpawn : public UParticleModule
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere)
	float Rate = 20.f;	// 초당 생성 수

	UPROPERTY(EditAnywhere)
	int32 BurstCount = 0;	// 시작할 때 한 번에 생성
};