#pragma once

#include "Entity.h"
#include "Camera.h"
#include "EventSystem.h"

namespace Imzadi
{
	class IMZADI_API FreeCam : public Entity
	{
	public:
		FreeCam();
		virtual ~FreeCam();

		virtual bool Setup() override;
		virtual bool Shutdown() override;
		virtual bool Tick(TickPass tickPass, double deltaTime) override;

		void SetCamera(Camera* camera);
		void SetEnabled(bool enabled);

	private:

		double GetStrafeSpeed();
		double GetRotationRate();

		enum StrafeMode
		{
			XZ_PLANE,
			XY_PLANE
		};

		enum Speed
		{
			SLOW,
			MEDIUM,
			FAST
		};

		Reference<Camera> camera;
		Speed speed;
		StrafeMode strafeMode;
	};

	class FreeCamTeleportEvent : public Event
	{
	public:
		FreeCamTeleportEvent();
		virtual ~FreeCamTeleportEvent();

		Transform transform;
	};
}