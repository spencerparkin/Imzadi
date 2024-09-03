#pragma once

#include "Entity.h"
#include "Collision/Shape.h"
#include "RenderObjects/RenderMeshInstance.h"

namespace Imzadi
{
	class WarpTunnelData;
	class CollisionShapeSet;

	/**
	 * These are similar to moving platforms, but are designed to add a sense of spacial paradox to the level.
	 */
	class WarpTunnel : public Entity
	{
	public:
		WarpTunnel();
		virtual ~WarpTunnel();

		virtual bool Setup() override;
		virtual bool Shutdown() override;

		/**
		 * Manage the binding of the warp-tunnel.
		 */
		virtual bool Tick(TickPass tickPass, double deltaTime) override;

		/**
		 * Specify where this wrap tunnel's configuration data is on disk.
		 */
		void SetWarpTunnelFile(const std::string& file) { this->warpTunnelFile = file; }

	private:

		bool BindPort(int portNumber);

		Reference<WarpTunnelData> data;
		Reference<RenderMeshInstance> renderMesh;
		std::vector<Collision::ShapeID> collisionShapeArray;
		std::string warpTunnelFile;
	};
}