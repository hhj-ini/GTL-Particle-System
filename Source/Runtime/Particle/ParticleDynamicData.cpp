#include "EnginePCH.h"
#include "Particle/ParticleDynamicData.h"
#include "Particle/ParticleSystemSceneProxy.h"
#include "Render/RenderPacket.h"

void FDynamicSpriteEmitterData::GetDynamicMeshElementsEmitter(FParticleSystemSceneProxy& Proxy, FRenderQueue& Queue,
	const FViewContext& View, const FMatrix* BillboardRotation) const
{
	const int32 Count = Source.ActiveParticleCount;
	UStaticMesh* QuadMesh = Proxy.GetQuadMesh();
	UMaterial* Material = Source.Material ? Source.Material : Proxy.GetDefaultMaterial();
	if (Count <= 0 || !QuadMesh || !Material || !BillboardRotation) return;

	// ReplayData → GPU 인스턴스 데이터
	TArray<FParticleSpriteVertex>& Vertices = Proxy.GetScratchVertices();
	Vertices.Reset();
	Vertices.Reserve(static_cast<uint32>(Count));

	const uint8* ParticleData = Source.DataContainer.ParticleData;
	const uint16* ParticleIndices = Source.DataContainer.ParticleIndices;
	for (int32 i = 0; i < Count; ++i)
	{
		const FBaseParticle& Particle =
			*reinterpret_cast<const FBaseParticle*>(ParticleData + Source.ParticleStride * ParticleIndices[i]);

		FParticleSpriteVertex Vertex{};
		Vertex.Position = Particle.Location;
		Vertex.RelativeTime = Particle.RelativeTime;
		Vertex.Size = FVector2(Particle.Size.X, Particle.Size.Y);
		Vertex.Rotation = Particle.Rotation;
		Vertex.SubImageIndex = 0.0f;   // 6단계 SubUV
		Vertex.Color = FVector4(Particle.Color.R, Particle.Color.G, Particle.Color.B, Particle.Color.A);
		Vertices.Add(Vertex);
	}

	FVertexBuffer* InstanceBuffer = Proxy.UploadInstanceBuffer(EmitterIndex, Vertices.GetData(), static_cast<uint32>(Count));
	if (!InstanceBuffer) return;

	const FVector Delta = Proxy.GetBounds().Center - View.CameraPosition;

	FRenderPacket Packet;
	Packet.Mesh = QuadMesh;
	Packet.Material = Material;
	Packet.Model = BillboardRotation;   // Proxy는 넣지 않는다 (아래 설명)
	Packet.InstanceBuffer = InstanceBuffer;
	Packet.InstanceCount = static_cast<uint32>(Count);
	Packet.CameraToParticleDistance = Delta.Dot(Delta);
	Queue.Add(Packet);
}