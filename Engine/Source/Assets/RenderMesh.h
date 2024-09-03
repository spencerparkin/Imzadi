#pragma once

#include "Math/AxisAlignedBoundingBox.h"
#include "Math/Transform.h"
#include "Scene.h"
#include "AssetCache.h"
#include "Shader.h"
#include "Texture.h"
#include "Buffer.h"
#include <d3dcommon.h>

namespace Imzadi
{
	/**
	 * This is everything that defines a renderable mesh without the
	 * particulars of an instance of such.
	 */
	class IMZADI_API RenderMeshAsset : public Asset
	{
	public:
		RenderMeshAsset();
		virtual ~RenderMeshAsset();

		virtual bool Load(const rapidjson::Document& jsonDoc, AssetCache* assetCache) override;
		virtual bool Unload() override;
		virtual bool MakeRenderInstance(Reference<RenderObject>& renderObject) override;

		D3D_PRIMITIVE_TOPOLOGY GetPrimType() const { return this->primType; }
		Shader* GetShader() { return this->mainPassShader.Get(); }
		Shader* GetShadowShader() { return this->shadowPassShader.Get(); }
		Buffer* GetVertexBuffer() { return this->vertexBuffer.Get(); }
		Buffer* GetIndexBuffer() { return this->indexBuffer.Get(); }
		Texture* GetTexture() { return this->texture.Get(); }
		const Transform& GetObjectToWorldTransform() const { return this->objectToWorld; }
		double GetLODRadius() const { return this->lodRadius; }
		const std::string& GetName() const { return this->name; }

		typedef std::unordered_map<std::string, Transform> PortMap;

		/**
		 * Get the port transform by the given name.  These are port-space
		 * to object-space transforms.  Typically these are concatinated with
		 * the object-to-world transform to get a port-to-world transform.
		 * In any case, these are just named sub-spaces on the render-mesh.
		 * Most meshes will probably not make use of this feature, but some will.
		 * You can think of them as attachment points on the mesh.
		 * 
		 * @param[in] portName This is the name of the desired port.
		 * @param[out] portToObject This is the returned transform.
		 * @return True is returned if the given port exists; false, otherwise, and the given transform is left untouched.
		 */
		bool GetPort(const std::string& portName, Transform& portToObject) const;

		/**
		 * Get read-only access to this mesh's map of port names to port transforms.
		 */
		const PortMap& GetPortMap() const { return this->portMap; }

	protected:
		D3D_PRIMITIVE_TOPOLOGY primType;
		Reference<Buffer> vertexBuffer;
		Reference<Buffer> indexBuffer;
		Reference<Shader> mainPassShader;
		Reference<Shader> shadowPassShader;
		Reference<Texture> texture;
		Reference<RenderMeshAsset> nextLOD;
		AxisAlignedBoundingBox objectSpaceBoundingBox;
		Transform objectToWorld;
		double lodRadius;
		PortMap portMap;
		std::string name;
	};
}