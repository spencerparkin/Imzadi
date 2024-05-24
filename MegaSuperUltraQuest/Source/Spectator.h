#pragma once

#include "Entity.h"
#include "Camera.h"

class FreeCam;

/**
 * An instance of this class is our follow-cam.  It can follow
 * any entity, but typically follows the hero to give us our
 * 3rd-person view during game-play.
 */
class Spectator : public Entity
{
public:
	Spectator();
	virtual ~Spectator();

	virtual bool Setup() override;
	virtual bool Shutdown(bool gameShuttingDown) override;
	virtual void Tick(double deltaTime) override;

	void SetSubject(Entity* entity) { this->subject.SafeSet(entity); }
	void SetCamera(Camera* camera) { this->camera.SafeSet(camera); }

	struct FollowParams
	{
		double followingDistance;
		double hoverHeight;
		double rotationRate;
		Collision::Vector3 objectSpaceFocalPoint;
	};

	const FollowParams& GetFollowParams() const { return this->followParams; }
	void SetFollowParams(const FollowParams& followParams) { this->followParams = followParams; }

private:
	Reference<Entity> subject;
	Reference<Camera> camera;
	Reference<FreeCam> freeCam;
	FollowParams followParams;
};