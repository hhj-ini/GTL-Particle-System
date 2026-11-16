#pragma once
#include "UObject/Object.h"
#include "UObject/Class.h"
#include "Containers/Array.h"
#include "ParticleLODLevel.generated.h"

class UParticleModule;
class UParticleModuleRequired;
class UParticleModuleSpawn;
class UParticleModuleTypeDataBase;


UCLASS()
class UParticleLODLevel : public UObject
{
	GENERATED_BODY()

public:
	void UpdateModuleLists();

	int32 Level = 0;
	bool bEnabled = true;

	UParticleModuleRequired* RequiredModule = nullptr;
	TArray<UParticleModule*> Modules;

	UParticleModuleTypeDataBase* TypeDataModule = nullptr;

	UParticleModuleSpawn* SpawnModule = nullptr;
	TArray<UParticleModule*> SpawnModules;
	TArray<UParticleModule*> UpdateModules;
};