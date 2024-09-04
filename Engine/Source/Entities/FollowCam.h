#pragma once

#include "Entity.h"
#include "Camera.h"
#include "Math/SphericalCoords.h"
#include "Collision/System.h"
#include "Collision/Query.h"
#include "WarpTunnel.h"

namespace Imzadi
{
	class FreeCam;

	/**
	 * This class gives us our 3rd-person view of the player's character.
	 */
	class IMZADI_API FollowCam : public Entity
	{
	public:
		FollowCam();
		virtual ~FollowCam();

		virtual bool Setup() override;
		virtual bool Shutdown() override;
		virtual bool Tick(TickPass tickPass, double deltaTime) override;
		virtual uint32_t TickOrder() const override;

		void SetSubject(Entity* entity) { this->subject.SafeSet(entity); }
		Entity* GetSubject() { return this->subject.Get(); }

		void SetCamera(Camera* camera) { this->camera.SafeSet(camera); }
		Camera* GetCamera() { return this->camera.Get(); }

		struct FollowParams
		{
			double maxRotationRate;
			Vector3 objectSpaceFocalPoint;
		};

		const FollowParams& GetFollowParams() const { return this->followParams; }
		void SetFollowParams(const FollowParams& followParams) { this->followParams = followParams; }

		void SetCameraUser(const std::string& user) { this->cameraUser = user; }
		const std::string& GetCameraUser() const { return this->cameraUser; }

	private:
		void CalculateDesiredCameraPositionAndOrientation();
		void MoveCameraOrbitBehindSubject(bool immediate);
		void HandleWarpTunnelEvent(const WarpTunnelEvent* event);

		Reference<Entity> subject;
		Reference<Camera> camera;
		Reference<FreeCam> freeCam;
		FollowParams followParams;
		SphericalCoords orbitLocation;
		SphericalCoords targetOrbitLocation;
		std::string cameraUser;
		Vector3 worldSpaceFocalPoint;
		Transform desiredCameraObjectToWorld;
		Collision::TaskID rayCastQueryTaskID;
		Vector3 preservedCameraOffset;
		bool fixOrbitLocation;
	};
}