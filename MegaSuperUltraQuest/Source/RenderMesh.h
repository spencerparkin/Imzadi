#pragma once

#include "Math/AxisAlignedBoundingBox.h"
#include "Math/Transform.h"
#include "AssetCache.h"

class Scene;
class Shader;
class Texture;

/**
 * These are the basic units of drawing in the game.  Anything that draws
 * is just a RenderMesh class instance.  This class is designed to facilitate
 * both static and dynamic geometry.  In the static case, the vertex buffer
 * never changes.  In the dynamic case, the vertex buffer can occationally
 * change.  In either case, the rendered mesh can be drawn using an object-space
 * to world-space transform that changes frame to frame.  A typical use-case
 * for dynamic render meshes may be to support the skinning of skeletal
 * bone hierarchies, or "rigs" as they're sometimes called.  There may be
 * other use-cases that involve mesh-deformations.
 * 
 * One draw-call is made per visible render mesh.  To keep things simple,
 * up to one texture is supported per render mesh.
 */
class RenderMesh : public Asset
{
public:
	RenderMesh();
	virtual ~RenderMesh();

	virtual bool Load(const rapidjson::Document& jsonDoc, AssetCache* assetCache) override;
	virtual bool Unload() override;

	const Collision::AxisAlignedBoundingBox& GetWorldBoundingBox() const { return this->boundingBox; }

	void Render(Scene* scene);

private:

	// TODO: Add vertex buffer.
	// TODO: Add index buffer.

	std::shared_ptr<Shader> shader;
	std::shared_ptr<Texture> texture;
	Collision::AxisAlignedBoundingBox boundingBox;
	Collision::Transform objectToWorld;
};