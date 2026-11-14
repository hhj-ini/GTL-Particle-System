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

private:
	bool EnsureInstanceBuffer(uint32 RequiredCount);

	void BuildTestInstances(TArray<FParticleSpriteVertex>& OutInstances) const;

private:
	UStaticMesh* QuadMesh = nullptr;
	UMaterial* Material = nullptr;

	TUniquePtr<FVertexBuffer> InstanceBuffer;
	uint32 InstanceCapacity = 0;
	TArray<FParticleSpriteVertex> InstanceData;


	//UPROPERTY()
	//UParticleSystem* Template = nullptr;

	TArray<FParticleEmitterInstance*> EmitterInstance;
	TArray<FDynamicEmitterDataBase*> EmitterRenderData;
};