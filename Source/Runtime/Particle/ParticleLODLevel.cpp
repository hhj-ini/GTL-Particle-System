#include "EnginePCH.h"
#include "ParticleLODLevel.h"
#include "Modules/ParticleModule.h"
#include "Modules/ParticleModuleSpawn.h"

void UParticleLODLevel::UpdateModuleLists()
{
	SpawnModule = nullptr;
	SpawnModules.Reset();
	UpdateModules.Reset();

	for (UParticleModule* Module : Modules)
	{
		if (!Module || !Module->bEnabled) continue;

		if (UParticleModuleSpawn* Spawn = Cast<UParticleModuleSpawn>(Module))
		{
			SpawnModule = Spawn;
			continue;
		}

		if (Module->IsSpawnModule())
		{
			SpawnModules.Add(Module);
		}
		if (Module->IsUpdateModule())
		{
			UpdateModules.Add(Module);
		}
	}

}
