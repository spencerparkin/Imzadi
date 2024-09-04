#pragma once

#include "Entity.h"
#include "Collision/Shape.h"
#include "RenderObjects/RenderMeshInstance.h"
#include "EventSystem.h"
#include "Entities/Biped.h"

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

		/**
		 * Specify the entity to track in this warp tunnel.
		 */
		void SetMainCharacterHandle(uint32_t handle) { this->mainCharacterHandle = handle; }

	private:

		bool BindPort(int portNumber);
		void HandleBipedResetEvent(const BipedResetEvent* event);

		Reference<WarpTunnelData> data;
		Reference<RenderMeshInstance> renderMesh;
		Reference<Entity> targetEntity;
		std::set<Collision::ShapeID> collisionShapeSet;
		std::string warpTunnelFile;
		uint32_t mainCharacterHandle;
		int currentlyBoundPortNumber;
		int coolDownCount;
		EventListenerHandle eventListenerHandle;
	};

	/**
	 * This let's other systems know
	 */
	class WarpTunnelEvent : public Event
	{
	public:
		WarpTunnelEvent()
		{
		}

		virtual ~WarpTunnelEvent()
		{
		}
	};
}