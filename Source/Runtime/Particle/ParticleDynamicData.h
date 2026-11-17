#pragma once
#include "Particle/ParticleHelper.h"
#include "Containers/Array.h"

class UMaterial;
class FRenderQueue;
struct FViewContext;
struct FMatrix;
class FParticleSystemSceneProxy;

enum class EDynamicEmitterType : uint8
{
	Unknown,
	Sprite,
	Mesh,   // Mesh Emitter 때 사용
};

// 시뮬레이션 상태의 스냅샷. Tick이 끝난 시점의 활성 파티클을 복사해 둔다
struct FDynamicEmitterReplayDataBase
{
	virtual ~FDynamicEmitterReplayDataBase() = default;

	EDynamicEmitterType EmitterType = EDynamicEmitterType::Unknown;
	int32 ActiveParticleCount = 0;
	int32 ParticleStride = 0;
	FParticleDataContainer DataContainer;
};

struct FDynamicSpriteEmitterReplayDataBase : public FDynamicEmitterReplayDataBase
{
	UMaterial* Material = nullptr;
	int32 SubImagesHorizontal = 1;
	int32 SubImagesVertical = 1;
};

struct FDynamicSpriteEmitterReplayData : public FDynamicSpriteEmitterReplayDataBase
{
};

// 렌더러가 보는 이미터 단위 데이터. ReplayData를 들고 있고 패킷을 만든다
struct FDynamicEmitterDataBase
{
	virtual ~FDynamicEmitterDataBase() = default;

	virtual const FDynamicEmitterReplayDataBase& GetSource() const = 0;
	virtual void GetDynamicMeshElementsEmitter(FParticleSystemSceneProxy& Proxy, FRenderQueue& Queue,
		const FViewContext& View, const FMatrix* BillboardRotation) const = 0;

	int32 EmitterIndex = 0;   // 프록시의 이미터별 GPU 버퍼 슬롯
	bool bValid = false;
};

struct FDynamicSpriteEmitterDataBase : public FDynamicEmitterDataBase
{
	// 5단계 SortSpriteParticles가 여기에 들어간다
};

struct FDynamicSpriteEmitterData : public FDynamicSpriteEmitterDataBase
{
	const FDynamicEmitterReplayDataBase& GetSource() const override { return Source; }
	void GetDynamicMeshElementsEmitter(FParticleSystemSceneProxy& Proxy, FRenderQueue& Queue,
		const FViewContext& View, const FMatrix* BillboardRotation) const override;

	FDynamicSpriteEmitterReplayData Source;
};

// 컴포넌트 하나 분량. 프록시가 소유한다
struct FParticleDynamicData
{
	FParticleDynamicData() = default;
	~FParticleDynamicData()
	{
		for (FDynamicEmitterDataBase* Data : DynamicEmitterDataArray)
			delete Data;
	}
	FParticleDynamicData(const FParticleDynamicData&) = delete;
	FParticleDynamicData& operator=(const FParticleDynamicData&) = delete;

	TArray<FDynamicEmitterDataBase*> DynamicEmitterDataArray;
};