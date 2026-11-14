#include "EnginePCH.h"
#include "ParticleSystemComponent.h"

#include "Asset/AssetManager.h"
#include "Render/Buffer.h"
#include "Render/RenderCommand.h"
#include "Render/RenderPacket.h"

UParticleSystemComponent::UParticleSystemComponent()
{
	QuadMesh = UAssetManager::GetAssetByPath<UStaticMesh>("ParticleQuad");
	Material = UAssetManager::GetAssetByPath<UMaterial>("ParticleSpriteMaterial");

	bCastShadow = false;
}

UParticleSystemComponent::~UParticleSystemComponent() = default;

void UParticleSystemComponent::SubmitToRenderQueue(FRenderQueue& RenderQueue, const FViewContext& ViewContext)
{
	if (!QuadMesh || !Material)
	{
		return;
	}

	BuildTestInstances(InstanceData);
	const uint32 InstanceCount = static_cast<uint32>(InstanceData.Num());
	if (InstanceCount == 0)
	{
		return;
	}

	if (!EnsureInstanceBuffer(InstanceCount))
	{
		return;
	}

	RenderCommand::UpdateBufferData(InstanceBuffer.get(), InstanceData.GetData(), sizeof(FParticleSpriteVertex) * InstanceCount);

	const FMatrix BillboardRotation = ViewContext.BuildBillboardMatrix(FVector::ZeroVector, 1.0f, 1.0f);
	const FVector Delta = GetWorldLocation() - ViewContext.CameraPosition;

	FRenderPacket Packet;
	Packet.Mesh = QuadMesh;
	Packet.Material = Material;
	Packet.Model = RenderQueue.StoreWorldMatrix(BillboardRotation);
	Packet.InstanceBuffer = InstanceBuffer.get();
	Packet.InstanceCount = InstanceCount;
	Packet.CameraToParticleDistance = Delta.Dot(Delta);
	RenderQueue.Add(Packet);
		
}

FBox UParticleSystemComponent::CalcLocalBounds() const
{
	const float Extent = 3.0f;
	return FBox{ FVector(-Extent, -Extent, -Extent), FVector(Extent, Extent, Extent) };
}

bool UParticleSystemComponent::EnsureInstanceBuffer(uint32 RequiredCount)
{
	if (InstanceBuffer && RequiredCount <= InstanceCapacity)
	{
		return true;
	}

	uint32 NewCapacity = FMath::Max(InstanceCapacity * 2, 64u);
	while (NewCapacity < RequiredCount)
	{
		NewCapacity *= 2;
	}

	InstanceBuffer = RenderCommand::CreateDynamicVertexBuffer(sizeof(FParticleSpriteVertex) * NewCapacity, sizeof(FParticleSpriteVertex));
	InstanceCapacity = InstanceBuffer ? NewCapacity : 0;
	return InstanceBuffer != nullptr;
}

void UParticleSystemComponent::BuildTestInstances(TArray<FParticleSpriteVertex>& OutInstances) const
{
	const FVector Origin = GetWorldLocation();
	OutInstances.Reset();

	const FParticleSpriteVertex Tests[4] =
	{
		//  Position                          RelTime  Size                Rotation  SubImage  Color
		{ Origin + FVector(0,  0.0f,  0.0f), 0.0f, FVector2(1.0f, 1.0f), 0.0f,     0.0f, FVector4(1, 1, 1, 1)   },
		{ Origin + FVector(0,  2.0f,  0.0f), 0.0f, FVector2(0.5f, 0.5f), 0.785f,   0.0f, FVector4(1, 0, 0, 1)   },
		{ Origin + FVector(0, -2.0f,  0.0f), 0.0f, FVector2(1.0f, 0.5f), 0.0f,     0.0f, FVector4(0, 1, 0, 1)   },
		{ Origin + FVector(0,  0.0f,  2.0f), 0.0f, FVector2(0.7f, 0.7f), 1.57f,    0.0f, FVector4(0, 0, 1, 0.5f)},
	};

	for (const FParticleSpriteVertex& Instance : Tests)
	{
		OutInstances.Add(Instance);
	}
}
