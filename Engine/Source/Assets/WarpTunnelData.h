#pragma once

#include "AssetCache.h"

namespace Imzadi
{
	/**
	 * This contains all the data needed to determine how a warp-tunnel moves.
	 */
	class IMZADI_API WarpTunnelData : public Asset
	{
	public:
		WarpTunnelData();
		virtual ~WarpTunnelData();

		virtual bool Load(const rapidjson::Document& jsonDoc, AssetCache* assetCache) override;
		virtual bool Unload() override;

		struct PortBind
		{
			std::string domesticPort;
			std::string foreignPort;
			std::string foreignMesh;
		};

		const std::vector<PortBind>& GetPortBindArray() const { return this->portBindArray; }
		const std::string& GetMeshFile() const { return this->meshFile; }
		const std::string& GetCollisionFile() const { return this->collisionFile; }

	private:
		std::vector<PortBind> portBindArray;
		std::string meshFile;
		std::string collisionFile;
	};
}