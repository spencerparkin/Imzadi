#pragma once

#include "AssetCache.h"

namespace Imzadi
{
	class Skeleton;
	class BareBuffer;

	/**
	 * This class specifies how each vertex in a bind-pose vertex buffer is weighted
	 * to one or more named bones of a skeleton.  "Vertex X is bound to bone Y by
	 * percentage Z", and so forth.
	 */
	class IMZADI_API SkinWeights : public Asset
	{
	public:
		SkinWeights();
		virtual ~SkinWeights();

		virtual bool Load(const rapidjson::Document& jsonDoc, std::string& error, AssetCache* assetCache) override;
		virtual bool Unload() override;
		virtual bool Save(rapidjson::Document& jsonDoc, std::string& error) const override;

		void Clear();

		/**
		 * Do our best to calculate a reasonable set of skin-weights for the given
		 * vertex buffer and skeleton.
		 *
		 * Note that we expect the positional part of each vertex to be a set of 3 floats.
		 *
		 * @param[in] skeleton We weight vertices against the bones in this skeleton.
		 * @param[in] bindPoseVertexBuffer The vertices of this buffer are weighted.
		 * @param[in] elementStride This is the stride, in bytes, from one vertex to the next in the given buffer.
		 * @param[in] vertexOffset This is the offset, in bytes, from the start of a vertex to the positional part of a vertex.
		 * @param[in] radius Bone space origins further than this radius to a vertex are not weighted to that vertex, unless this would mean the vertex never gets weighted to any bone.
		 * @return True is returned if successful; false, otherwise.
		 */
		bool AutoSkin(const Skeleton* skeleton, const BareBuffer* bindPoseVertexBuffer, uint32_t elementStride, uint32_t vertexOffset, double radius);

		struct BoneWeight
		{
			std::string boneName;
			double weight;
		};

		const std::vector<BoneWeight>& GetBoneWeightsForVertex(uint32_t vertex) const;
		std::vector<BoneWeight>& GetBonesWeightsForVertex(uint32_t vertex);

		void SetNumVertices(size_t numVertices);
		size_t GetNumVertices() const;

	private:

		void NormalizeWeights(std::vector<BoneWeight>& boneWeightArray);

		std::vector<std::vector<BoneWeight>> weightedVertexArray;
	};
}