#pragma once

#include "AssetCache.h"

/**
 * This class specifies how each vertex in a bind-pose vertex buffer is weighted
 * to one or more named bones of a skeleton.  "Vertex X is bound to bone Y by
 * percentage Z", and so forth.
 */
class SkinWeights : public Asset
{
public:
	SkinWeights();
	virtual ~SkinWeights();

	virtual bool Load(const rapidjson::Document& jsonDoc, AssetCache* assetCache) override;
	virtual bool Unload() override;
	virtual bool Save(rapidjson::Document& jsonDoc) const override;

	// TODO: Add auto skin algorithm?

private:
	// TODO: Add members here.
};