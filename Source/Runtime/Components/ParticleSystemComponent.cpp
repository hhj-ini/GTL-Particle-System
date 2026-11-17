#include "EnginePCH.h"
#include "Components/ParticleSystemComponent.h"

#include "Asset/AssetManager.h"
#include "Render/Buffer.h"
#include "Render/RenderCommand.h"
#include "Render/RenderPacket.h"

#include "Particle/ParticleSystem.h"
#include "Particle/ParticleEmitter.h"
#include "Particle/ParticleLODLevel.h"
#include "Particle/ParticleEmitterInstances.h"
#include "Particle/Modules/ParticleModuleRequired.h"
#include "Particle/Modules/ParticleModuleSpawn.h"
#include "Particle/Modules/ParticleModuleLifetime.h"
#include "Particle/Modules/ParticleModuleVelocity.h"

#include "Particle/ParticleDynamicData.h"
#include "Particle/ParticleSystemSceneProxy.h"

namespace
{
	// 3단계 테스트용 시스템. 8단계에서 .particle 에셋 로드로 대체한다
	UParticleSystem* GetTestParticleSystem()
	{
		static UParticleSystem* System = nullptr;
		if (System) return System;

		System = NewObject<UParticleSystem>();
		UParticleEmitter* Emitter = NewObject<UParticleEmitter>(System);
		UParticleLODLevel* LOD = NewObject<UParticleLODLevel>(Emitter);

		UParticleModuleRequired* Required = NewObject<UParticleModuleRequired>(LOD);
		UParticleModuleSpawn* Spawn = NewObject<UParticleModuleSpawn>(LOD);
		Spawn->Rate = 20.0f;
		UParticleModuleLifetime* Lifetime = NewObject<UParticleModuleLifetime>(LOD);
		Lifetime->MinLifetime = 1.0f;
		Lifetime->MaxLifetime = 2.0f;
		UParticleModuleVelocity* Velocity = NewObject<UParticleModuleVelocity>(LOD);

		LOD->RequiredModule = Required;
		LOD->Modules.Add(Spawn);
		LOD->Modules.Add(Lifetime);
		LOD->Modules.Add(Velocity);

		Emitter->LODLevels.Add(LOD);
		System->Emitters.Add(Emitter);
		return System;
	}
}

UParticleSystemComponent::UParticleSystemComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	QuadMesh = UAssetManager::GetAssetByPath<UStaticMesh>("ParticleQuad");
	Material = UAssetManager::GetAssetByPath<UMaterial>("ParticleSpriteMaterial");
	bCastShadow = false;
}

UParticleSystemComponent::~UParticleSystemComponent()
{
	ClearEmitterInstances();
}

FPrimitiveSceneProxy* UParticleSystemComponent::CreateSceneProxy()
{
	return new FParticleSystemSceneProxy(this);
}

void UParticleSystemComponent::SetTemplate(UParticleSystem* NewTemplate)
{
	Template = NewTemplate;
	ClearEmitterInstances();   // 다음 Tick에서 다시 만든다
}

void UParticleSystemComponent::ClearEmitterInstances()
{
	for (FParticleEmitterInstance* Instance : EmitterInstances)
		delete Instance;
	EmitterInstances.Reset();

	if (FParticleSystemSceneProxy* Proxy = static_cast<FParticleSystemSceneProxy*>(GetSceneProxy()))
		Proxy->SetDynamicData(nullptr);
}

void UParticleSystemComponent::InitializeSystem()
{
	ClearEmitterInstances();
	if (!Template) Template = GetTestParticleSystem();
	if (!Template) return;

	for (UParticleEmitter* Emitter : Template->Emitters)
	{
		if (!Emitter) continue;
		FParticleEmitterInstance* Instance = new FParticleSpriteEmitterInstance();
		Instance->Init(Emitter, this);
		EmitterInstances.Add(Instance);
	}
}

void UParticleSystemComponent::TickComponent(float DeltaTime)
{
	Super::TickComponent(DeltaTime);

	// 생성자(CDO 포함)에서 NewObject를 부르지 않도록 첫 Tick에서 만든다
	if (EmitterInstances.IsEmpty())
		InitializeSystem();

	for (FParticleEmitterInstance* Instance : EmitterInstances)
		if (Instance) Instance->Tick(DeltaTime);

	UpdateParticleBounds();
	UpdateDynamicData();
}


void UParticleSystemComponent::UpdateDynamicData()
{
	// CreateSceneProxy가 항상 FParticleSystemSceneProxy를 만들기 때문에 static_cast가 안전하다
	FParticleSystemSceneProxy* Proxy = static_cast<FParticleSystemSceneProxy*>(GetSceneProxy());
	if (!Proxy) return;   // 아직 씬에 등록 전

	FParticleDynamicData* NewData = new FParticleDynamicData();
	for (int32 EmitterIndex = 0; EmitterIndex < EmitterInstances.Num(); ++EmitterIndex)
	{
		FParticleEmitterInstance* Instance = EmitterInstances[EmitterIndex];
		if (!Instance) continue;

		FDynamicEmitterDataBase* EmitterData = Instance->GetDynamicData(false);
		if (!EmitterData) continue;
		EmitterData->EmitterIndex = EmitterIndex;
		NewData->DynamicEmitterDataArray.Add(EmitterData);
	}
	Proxy->SetDynamicData(NewData);
}

void UParticleSystemComponent::UpdateParticleBounds()
{
	bool bHasParticles = false;
	FBox WorldBounds{};
	for (const FParticleEmitterInstance* Instance : EmitterInstances)
	{
		FBox EmitterBounds;
		if (!Instance || !Instance->GetParticleBounds(EmitterBounds)) continue;
		if (bHasParticles) WorldBounds.Expand(EmitterBounds);
		else { WorldBounds = EmitterBounds; bHasParticles = true; }
	}

	if (bHasParticles)
	{
		// 파티클은 월드 공간. 프록시는 로컬 바운드를 월드로 다시 변환하므로 역행렬로 로컬로 돌려 둔다
		LocalParticleBounds = WorldBounds.GetWorldAABB(GetWorldMatrix().Inverse());
	}
	else
	{
		LocalParticleBounds = FBox{ FVector(-0.5f, -0.5f, -0.5f), FVector(0.5f, 0.5f, 0.5f) };
	}

	OnTransformDirty();   // 이번 프레임 Scene.UpdateAllTransforms()에서 바운드가 갱신된다
}

FBox UParticleSystemComponent::CalcLocalBounds() const
{
	return LocalParticleBounds;
}