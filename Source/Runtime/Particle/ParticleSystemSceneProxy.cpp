#include "EnginePCH.h"
#include "Particle/ParticleSystemSceneProxy.h"
#include "Particle/ParticleDynamicData.h"
#include "Components/ParticleSystemComponent.h"
#include "Render/Buffer.h"
#include "Render/RenderCommand.h"
#include "Render/RenderPacket.h"

FParticleSystemSceneProxy::FParticleSystemSceneProxy(UParticleSystemComponent* InComponent)
	: FPrimitiveSceneProxy(InComponent)
	, QuadMesh(InComponent->GetQuadMesh())
	, DefaultMaterial(InComponent->GetMaterial(0))
{
	bCastsShadow = false;
}

FParticleSystemSceneProxy::~FParticleSystemSceneProxy()
{
	delete DynamicData;
}

void FParticleSystemSceneProxy::UpdateRenderState()
{
	// 메시 캐시가 없는 프록시. 가시성/그림자 플래그만 갱신한다
	UpdateCommonFlags();
	bCastsShadow = false;
	DefaultMaterial = GetComponent()->GetMaterial(0);
}

void FParticleSystemSceneProxy::SetDynamicData(FParticleDynamicData* NewDynamicData)
{
	if (DynamicData == NewDynamicData) return;
	delete DynamicData;
	DynamicData = NewDynamicData;
}

FVertexBuffer* FParticleSystemSceneProxy::UploadInstanceBuffer(int32 EmitterIndex, const FParticleSpriteVertex* Vertices, uint32 Count)
{
	if (EmitterIndex < 0 || Count == 0) return nullptr;
	if (EmitterBuffers.size() <= static_cast<size_t>(EmitterIndex))
		EmitterBuffers.resize(static_cast<size_t>(EmitterIndex) + 1);

	FEmitterGpuBuffer& Gpu = EmitterBuffers[static_cast<size_t>(EmitterIndex)];
	if (!Gpu.Buffer || Count > Gpu.Capacity)
	{
		uint32 NewCapacity = FMath::Max(Gpu.Capacity * 2, 64u);
		while (NewCapacity < Count) NewCapacity *= 2;

		Gpu.Buffer = RenderCommand::CreateDynamicVertexBuffer(
			sizeof(FParticleSpriteVertex) * NewCapacity, sizeof(FParticleSpriteVertex));
		Gpu.Capacity = Gpu.Buffer ? NewCapacity : 0;
		if (!Gpu.Buffer) return nullptr;
	}

	RenderCommand::UpdateBufferData(Gpu.Buffer.get(), Vertices, sizeof(FParticleSpriteVertex) * Count);
	return Gpu.Buffer.get();
}

void FParticleSystemSceneProxy::GetDynamicMeshElements(FRenderQueue& Queue, const FViewContext& View)
{
	if (!DynamicData || DynamicData->DynamicEmitterDataArray.IsEmpty()) return;

	// 모든 이미터가 같은 카메라 회전을 쓰므로 뷰마다 한 번만 저장
	const FMatrix* BillboardRotation = Queue.StoreWorldMatrix(
		View.BuildBillboardMatrix(FVector::ZeroVector, 1.0f, 1.0f));

	for (const FDynamicEmitterDataBase* EmitterData : DynamicData->DynamicEmitterDataArray)
	{
		if (EmitterData && EmitterData->bValid)
			EmitterData->GetDynamicMeshElementsEmitter(*this, Queue, View, BillboardRotation);
	}
}