#include "EnginePCH.h"
#include "ParticleEmitter.h"
#include "ParticleHelper.h"
#include "ParticleLODLevel.h"
#include "Modules/ParticleModule.h"

void UParticleEmitter::CacheEmitterModuleInfo()
{
    ParticleSize = sizeof(FBaseParticle);
    ModuleOffsetMap.Empty();

    UParticleLODLevel* LOD = GetLODLevel(0);
    if (!LOD)
    {
        return;
    }

    LOD->UpdateModuleLists();

    for (UParticleModule* Module : LOD->Modules)
    {
        if (!Module)
        {
            continue;
        }

        const uint32 Bytes = Module->RequiredBytes();
        if (Bytes == 0)
        {
            continue;
        }

        ModuleOffsetMap.FindOrAdd(Module) = static_cast<uint32>(ParticleSize);
        ParticleSize += static_cast<int32>(Bytes);
    }

}

UParticleLODLevel* UParticleEmitter::GetLODLevel(int32 Index) const
{
    return (Index >= 0 && Index < LODLevels.Num()) ? LODLevels[Index] : nullptr;
}
