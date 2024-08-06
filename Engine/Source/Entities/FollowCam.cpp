#include "FollowCam.h"
#include "Math/Vector3.h"
#include "Math/Matrix3x3.h"
#include "Game.h"
#include "FreeCam.h"
#include "Math/Angle.h"
#include "Collision/Result.h"

using namespace Imzadi;

FollowCam::FollowCam()
{
	this->rayCastQueryTaskID = 0;
	this->followParams.maxRotationRate = M_PI / 1.5;
	this->followParams.objectSpaceFocalPoint.SetComponents(0.0, 5.0, 0.0);
	this->orbitLocation.radius = 20.0;
	this->orbitLocation.longitudeAngle = 0.0;
	this->orbitLocation.latitudeAngle = 0.0;
}

/*virtual*/ FollowCam::~FollowCam()
{
}

/*virtual*/ bool FollowCam::Setup()
{
	if (!this->camera || !this->subject)
		return false;

	this->MoveCameraOrbitBehindSubject(true);
	this->CalculateDesiredCameraPositionAndOrientation();

	this->camera->SetCameraToWorldTransform(this->desiredCameraObjectToWorld);

	this->freeCam = Game::Get()->SpawnEntity<FreeCam>();
	this->freeCam->SetCamera(this->camera);

	return true;
}

/*virtual*/ bool FollowCam::Shutdown()
{
	Entity::Shutdown();

	return true;
}

/*virtual*/ uint32_t FollowCam::TickOrder() const
{
	return 2;
}

/*virtual*/ bool FollowCam::Tick(TickPass tickPass, double deltaTime)
{
	switch (tickPass)
	{
		case TickPass::MOVE_UNCONSTRAINTED:
		{
			Controller* controller = Game::Get()->GetController(this->cameraUser);
			if (controller)
			{
#if defined _DEBUG
				if (controller->ButtonPressed(XINPUT_GAMEPAD_START, true))
					this->freeCam->SetEnabled(true);
#endif

				if (controller->ButtonPressed(XINPUT_GAMEPAD_LEFT_SHOULDER))
					this->MoveCameraOrbitBehindSubject(false);

				Vector2 rightStick;
				controller->GetAnalogJoyStick(Controller::Side::RIGHT, rightStick.x, rightStick.y);

				double longitudeAngleDelta = this->followParams.maxRotationRate * deltaTime * rightStick.x;
				double latitudeAngleDelta = this->followParams.maxRotationRate * deltaTime * -rightStick.y;

				this->targetOrbitLocation.longitudeAngle += longitudeAngleDelta;
				this->targetOrbitLocation.latitudeAngle += latitudeAngleDelta;

				this->orbitLocation.Lerp(this->orbitLocation, this->targetOrbitLocation, 0.3);

				this->CalculateDesiredCameraPositionAndOrientation();
			}

			break;
		}
		case TickPass::SUBMIT_COLLISION_QUERIES:
		{
			CollisionSystem* collisionSystem = Game::Get()->GetCollisionSystem();
			
			Ray ray;
			ray.origin = this->worldSpaceFocalPoint;
			ray.unitDirection = (this->desiredCameraObjectToWorld.translation - this->worldSpaceFocalPoint).Normalized();

			auto rayCastQuery = RayCastQuery::Create();
			rayCastQuery->SetRay(ray);
			rayCastQuery->SetUserFlagsMask(IMZADI_SHAPE_FLAG_WORLD_SURFACE);
			collisionSystem->MakeQuery(rayCastQuery, this->rayCastQueryTaskID);

			break;
		}
		case TickPass::PARALLEL_WORK:
		{
			break;
		}
		case TickPass::RESOLVE_COLLISIONS:
		{
			Transform cameraObjectToWorld = this->desiredCameraObjectToWorld;

			if (this->rayCastQueryTaskID)
			{
				CollisionSystem* collisionSystem = Game::Get()->GetCollisionSystem();

				Result* result = collisionSystem->ObtainQueryResult(this->rayCastQueryTaskID);
				if (result)
				{
					auto rayCastResult = dynamic_cast<RayCastResult*>(result);
					if (rayCastResult)
					{
						const RayCastResult::HitData& hitData = rayCastResult->GetHitData();
						if (hitData.shapeID != 0)
						{
							// Is something obscurring the view of the camera?
							Vector3 sightVector = this->desiredCameraObjectToWorld.translation - this->worldSpaceFocalPoint;
							double hitDistance = (hitData.surfacePoint - this->worldSpaceFocalPoint).Length();
							double eyeDistance = 0.0;
							sightVector.Normalize(&eyeDistance);
							if (hitDistance < eyeDistance)
							{
								// Yes.  Adjust our eye-point so that our view is not obscurred.
								double margin = 1e-2;
								eyeDistance = hitDistance - margin;
								cameraObjectToWorld.translation = this->worldSpaceFocalPoint + sightVector * eyeDistance;
							}
						}
					}

					collisionSystem->Free(result);
				}
			}

			this->camera->SetCameraToWorldTransform(cameraObjectToWorld);

			break;
		}
	}

	return true;
}

void FollowCam::MoveCameraOrbitBehindSubject(bool immediate)
{
	Transform subjectObjectToWorld;
	this->subject->GetTransform(subjectObjectToWorld);

	Vector3 xAxis, yAxis, zAxis;
	subjectObjectToWorld.matrix.GetColumnVectors(xAxis, yAxis, zAxis);

	Vector3 upVector(0.0, 1.0, 0.0);
	Vector3 behindVector = zAxis.RejectedFrom(upVector).Normalized();
	SphericalCoords coords;
	coords.SetFromVector(behindVector);

	coords.longitudeAngle = Angle::MakeClose(coords.longitudeAngle, this->orbitLocation.longitudeAngle);

	this->targetOrbitLocation.latitudeAngle = this->orbitLocation.latitudeAngle;
	this->targetOrbitLocation.longitudeAngle = coords.longitudeAngle;
	this->targetOrbitLocation.radius = this->orbitLocation.radius;

	if (immediate)
		this->orbitLocation = this->targetOrbitLocation;
}

void FollowCam::CalculateDesiredCameraPositionAndOrientation()
{
	Transform subjectObjectToWorld;
	this->subject->GetTransform(subjectObjectToWorld);

	this->worldSpaceFocalPoint = subjectObjectToWorld.TransformPoint(this->followParams.objectSpaceFocalPoint);

	Vector3 cameraOffset = this->orbitLocation.GetToVector();

	this->desiredCameraObjectToWorld.translation = this->worldSpaceFocalPoint + cameraOffset;

	Vector3 xAxis, yAxis, zAxis;
	Vector3 upAxis(0.0, 1.0, 0.0);

	zAxis = cameraOffset.Normalized();
	yAxis = upAxis.RejectedFrom(zAxis);
	xAxis = yAxis.Cross(zAxis);

	this->desiredCameraObjectToWorld.matrix.SetColumnVectors(xAxis, yAxis, zAxis);
}