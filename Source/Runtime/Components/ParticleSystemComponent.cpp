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
	EmitterBuffers.clear();
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
	EmitterBuffers.resize(static_cast<size_t>(EmitterInstances.Num()));
}

void UParticleSystemComponent::TickComponent(float DeltaTime)
{
	Super::TickComponent(DeltaTime);

	// 생성자(CDO 포함)에서 NewObject를 부르지 않도록 첫 Tick에서 만든다
	if (EmitterInstances.IsEmpty())
		InitializeSystem();

	for (FParticleEmitterInstance* Instance : EmitterInstances)
		if (Instance) Instance->Tick(DeltaTime);
}

FBox UParticleSystemComponent::CalcLocalBounds() const
{
	// 테스트 시스템 기준: 속도 최대 ~2 × 수명 최대 2초 ≈ 4. 넉넉하게 잡는다.
	// 정확한 동적 바운드는 4단계(ReplayData)에서 활성 파티클로 계산한다
	const float Extent = 10.0f;
	return FBox{ FVector(-Extent, -Extent, -Extent), FVector(Extent, Extent, Extent) };
}

bool UParticleSystemComponent::EnsureInstanceBuffer(FEmitterGpuBuffer& GpuBuffer, uint32 RequiredCount)
{
	if (GpuBuffer.Buffer && RequiredCount <= GpuBuffer.Capacity) return true;

	uint32 NewCapacity = FMath::Max(GpuBuffer.Capacity * 2, 64u);
	while (NewCapacity < RequiredCount) NewCapacity *= 2;

	GpuBuffer.Buffer = RenderCommand::CreateDynamicVertexBuffer(
		sizeof(FParticleSpriteVertex) * NewCapacity, sizeof(FParticleSpriteVertex));
	GpuBuffer.Capacity = GpuBuffer.Buffer ? NewCapacity : 0;
	return GpuBuffer.Buffer != nullptr;
}

void UParticleSystemComponent::BuildSpriteInstances(const FParticleEmitterInstance& Instance,
	TArray<FParticleSpriteVertex>& OutInstances) const
{
	OutInstances.Reset();
	OutInstances.Reserve(static_cast<uint32>(Instance.ActiveParticles));

	for (int32 i = 0; i < Instance.ActiveParticles; ++i)
	{
		const FBaseParticle& Particle = *Instance.GetParticle(i);

		FParticleSpriteVertex Vertex{};
		Vertex.Position = Particle.Location;
		Vertex.RelativeTime = Particle.RelativeTime;
		Vertex.Size = FVector2(Particle.Size.X, Particle.Size.Y);
		Vertex.Rotation = Particle.Rotation;
		Vertex.SubImageIndex = 0.0f;
		Vertex.Color = FVector4(Particle.Color.R, Particle.Color.G, Particle.Color.B, Particle.Color.A);
		OutInstances.Add(Vertex);
	}
}

void UParticleSystemComponent::SubmitToRenderQueue(FRenderQueue& RenderQueue, const FViewContext& ViewContext)
{
	if (!QuadMesh) return;
	if (EmitterBuffers.size() != static_cast<size_t>(EmitterInstances.Num())) return;

	const FMatrix* BillboardRotation = nullptr;
	const FVector Delta = GetWorldLocation() - ViewContext.CameraPosition;

	for (int32 EmitterIndex = 0; EmitterIndex < EmitterInstances.Num(); ++EmitterIndex)
	{
		const FParticleEmitterInstance* Instance = EmitterInstances[EmitterIndex];
		if (!Instance || !Instance->CurrentLODLevel || Instance->ActiveParticles == 0) continue;

		const UParticleModuleRequired* Required = Instance->CurrentLODLevel->RequiredModule;
		UMaterial* EmitterMaterial = (Required && Required->Material) ? Required->Material : Material;
		if (!EmitterMaterial) continue;

		// 3단계: 시뮬레이션 메모리를 직접 읽는다. 4단계에서 ReplayData 복사본으로 바꾼다
		BuildSpriteInstances(*Instance, InstanceData);
		const uint32 InstanceCount = static_cast<uint32>(InstanceData.Num());

		FEmitterGpuBuffer& GpuBuffer = EmitterBuffers[static_cast<size_t>(EmitterIndex)];
		if (!EnsureInstanceBuffer(GpuBuffer, InstanceCount)) continue;
		RenderCommand::UpdateBufferData(GpuBuffer.Buffer.get(), InstanceData.GetData(),
			sizeof(FParticleSpriteVertex) * InstanceCount);

		if (!BillboardRotation)
			BillboardRotation = RenderQueue.StoreWorldMatrix(
				ViewContext.BuildBillboardMatrix(FVector::ZeroVector, 1.0f, 1.0f));

		FRenderPacket Packet;
		Packet.Mesh = QuadMesh;
		Packet.Material = EmitterMaterial;
		Packet.Model = BillboardRotation;
		Packet.InstanceBuffer = GpuBuffer.Buffer.get();
		Packet.InstanceCount = InstanceCount;
		Packet.CameraToParticleDistance = Delta.Dot(Delta);
		RenderQueue.Add(Packet);
	}
}