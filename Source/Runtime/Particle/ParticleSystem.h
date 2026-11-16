#pragma once
#include "UObject/Object.h"
#include "UObject/Class.h"
#include "Containers/Array.h"
#include "ParticleSystem.generated.h"

class UParticleEmitter;

UCLASS()
class UParticleSystem : public UObject
{
	GENERATED_BODY()

public:
	void UpdateAllModuleLists();

	TArray<UParticleEmitter*> Emitters;
};