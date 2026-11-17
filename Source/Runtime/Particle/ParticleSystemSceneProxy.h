#pragma once
#include "Engine/PrimitiveSceneProxy.h"
#include "Particle/ParticleHelper.h"
#include "Containers/Array.h"
#include <vector>

class FVertexBuffer;
class UParticleSystemComponent;
struct FParticleDynamicData;

class FParticleSystemSceneProxy : public FPrimitiveSceneProxy
{
public:
	explicit FParticleSystemSceneProxy(UParticleSystemComponent* InComponent);
	~FParticleSystemSceneProxy() override;

	void UpdateRenderState() override;
	bool HasDynamicMeshElements() const override { return true; }
	void GetDynamicMeshElements(FRenderQueue& Queue, const FViewContext& View) override;

	// 컴포넌트가 매 Tick 새 스냅샷을 넘긴다. 소유권이 프록시로 넘어온다 (nullptr이면 비움)
	void SetDynamicData(FParticleDynamicData* NewDynamicData);

	UStaticMesh* GetQuadMesh() const { return QuadMesh; }
	UMaterial* GetDefaultMaterial() const { return DefaultMaterial; }
	TArray<FParticleSpriteVertex>& GetScratchVertices() { return ScratchVertices; }
	// EmitterIndex 슬롯의 인스턴스 버퍼를 필요하면 키우고 데이터를 올린다
	FVertexBuffer* UploadInstanceBuffer(int32 EmitterIndex, const FParticleSpriteVertex* Vertices, uint32 Count);

private:
	struct FEmitterGpuBuffer
	{
		TUniquePtr<FVertexBuffer> Buffer;
		uint32 Capacity = 0;
	};

	FParticleDynamicData* DynamicData = nullptr;
	UStaticMesh* QuadMesh = nullptr;
	UMaterial* DefaultMaterial = nullptr;

	std::vector<FEmitterGpuBuffer> EmitterBuffers;   // TUniquePtr라서 복사가 안됨. std::vector 사용
	TArray<FParticleSpriteVertex> ScratchVertices;   // 이미터마다 재사용하는 임시 배열
};