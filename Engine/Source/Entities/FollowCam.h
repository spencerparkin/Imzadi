#pragma once

#include "Entity.h"
#include "Camera.h"
#include "Math/SphericalCoords.h"

class FreeCam;

/**
 * This class gives us our 3rd-person view of the player's character.
 */
class FollowCam : public Entity
{
public:
	FollowCam();
	virtual ~FollowCam();

	virtual bool Setup() override;
	virtual bool Shutdown(bool gameShuttingDown) override;
	virtual bool Tick(TickPass tickPass, double deltaTime) override;

	void SetSubject(Entity* entity) { this->subject.SafeSet(entity); }
	Entity* GetSubject() { return this->subject.Get(); }

	void SetCamera(Camera* camera) { this->camera.SafeSet(camera); }
	Camera* GetCamera() { return this->camera.Get(); }

	struct FollowParams
	{
		double maxRotationRate;
		Collision::Vector3 objectSpaceFocalPoint;
	};

	const FollowParams& GetFollowParams() const { return this->followParams; }
	void SetFollowParams(const FollowParams& followParams) { this->followParams = followParams; }

private:
	void CalculateCameraPositionAndOrientation();
	void MoveCameraOrbitBehindSubject(bool immediate);

	Reference<Entity> subject;
	Reference<Camera> camera;
	Reference<FreeCam> freeCam;
	FollowParams followParams;
	Collision::SphericalCoords orbitLocation;
	Collision::SphericalCoords targetOrbitLocation;
};