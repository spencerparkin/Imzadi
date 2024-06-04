#pragma once

#include "Entity.h"
#include "Camera.h"

class FreeCam : public Entity
{
public:
	FreeCam();
	virtual ~FreeCam();

	virtual bool Setup() override;
	virtual bool Shutdown(bool gameShuttingDown) override;
	virtual bool Tick(TickPass tickPass, double deltaTime) override;

	void SetCamera(Camera* camera) { this->camera.Set(camera); }
	void SetEnabled(bool enabled) { this->enabled = enabled; }
	bool IsEnabled() const { return this->enabled; }

private:

	enum StrafeMode
	{
		XZ_PLANE,
		XY_PLANE
	};

	Reference<Camera> camera;
	bool enabled;
	double strafeSpeed;
	double rotationRate;
	StrafeMode strafeMode;
};