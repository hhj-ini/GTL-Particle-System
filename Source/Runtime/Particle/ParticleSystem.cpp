#include "EnginePCH.h"
#include "ParticleSystem.h"
#include "ParticleEmitter.h"

void UParticleSystem::UpdateAllModuleLists()
{
	for (UParticleEmitter* Emitter : Emitters)
	{
		if (Emitter)
		{
			Emitter->CacheEmitterModuleInfo();
		}
	}
}
