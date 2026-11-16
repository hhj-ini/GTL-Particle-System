#include "EnginePCH.h"
#include "ParticleModuleRequired.h"
#include "Asset/AssetManager.h"

UParticleModuleRequired::UParticleModuleRequired()
{
	Material = UAssetManager::GetAssetByPath<UMaterial>("ParticleSpriteMaterial");
}
