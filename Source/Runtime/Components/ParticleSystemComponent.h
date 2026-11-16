#pragma once

#include "UObject/Object.h"
#include "UObject/Class.h"
#include "SceneComponent.h"
#include "PrimitiveComponent.h"
#include "Containers/Array.h"
#include "Particle/ParticleHelper.h"
#include "ParticleSystemComponent.generated.h"

// Forward declarations.
class FVertexBuffer;
class UStaticMesh;
class UMaterial;
class UParticleSystem;
struct FParticleEmitterInstance;
struct FDynamicEmitterDataBase;

UCLASS()
class UParticleSystemComponent : public UPrimitiveComponent
{
	GENERATED_BODY()

public:

	UParticleSystemComponent();
	virtual ~UParticleSystemComponent() override;

	virtual void TickComponent(float DeltaTime) override;

	virtual void SubmitToRenderQueue(FRenderQueue& RenderQueue, const FViewContext& ViewContext) override;

	virtual int32 GetNumMaterials() const override { return 1; }
	virtual UMaterial* GetMaterial(int32 Slot) const override { return Slot == 0 ? Material : nullptr; }
	virtual void SetMaterial(int32 SlotIndex, UMaterial* InMaterial) override
	{
		if (SlotIndex == 0)
		{
			Material = InMaterial;
		}
	}

	virtual FBox CalcLocalBounds() const override;

	void SetTemplate(UParticleSystem* NewTemplate);
	UParticleSystem* GetTemplate() const { return Template; }
	void InitializeSystem();

private:
	void ClearEmitterInstances();
	void BuildSpriteInstances(const FParticleEmitterInstance& Instance, TArray<FParticleSpriteVertex>& OutInstances) const;

	struct FEmitterGpuBuffer
	{
		TUniquePtr<FVertexBuffer> Buffer;
		uint32 Capacity = 0;
	};
	
	static bool EnsureInstanceBuffer(FEmitterGpuBuffer& GpuBuffer, uint32 RequiredCount);


private:
	UStaticMesh* QuadMesh = nullptr;
	UMaterial* Material = nullptr;

	UParticleSystem* Template = nullptr;

	TArray<FParticleEmitterInstance*> EmitterInstances;
	TArray<FDynamicEmitterDataBase*> EmitterRenderData;

	std::vector<FEmitterGpuBuffer> EmitterBuffers;
	TArray<FParticleSpriteVertex> InstanceData;
};