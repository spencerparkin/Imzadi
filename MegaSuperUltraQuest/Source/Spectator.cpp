#include "Spectator.h"
#include "Math/Vector3.h"
#include "Math/Matrix3x3.h"
#include "Game.h"
#include "FreeCam.h"

using namespace Collision;

Spectator::Spectator()
{
	this->followParams.followingDistance = 20.0;
	this->followParams.hoverHeight = 10.0;
	this->followParams.rotationRate = M_PI / 16.0;
	this->followParams.objectSpaceFocalPoint.SetComponents(0.0, 5.0, 0.0);
}

/*virtual*/ Spectator::~Spectator()
{
}

/*virtual*/ bool Spectator::Setup()
{
	if (!this->camera || !this->subject)
		return false;

	Transform subjectTransform;
	if (!this->subject->GetTransform(subjectTransform))
		return false;

	Vector3 xAxis, yAxis, zAxis;
	subjectTransform.matrix.GetColumnVectors(xAxis, yAxis, zAxis);

	Vector3 worldSpaceFocalPoint = subjectTransform.TransformPoint(this->followParams.objectSpaceFocalPoint);
	Vector3 eyePoint = worldSpaceFocalPoint + this->followParams.followingDistance * zAxis + this->followParams.hoverHeight * yAxis;
	this->camera->LookAt(eyePoint, worldSpaceFocalPoint, Vector3(0.0, 1.0, 0.0));

	this->freeCam = Game::Get()->SpawnEntity<FreeCam>();
	this->freeCam->SetEnabled(false);

	return true;
}

/*virtual*/ bool Spectator::Shutdown(bool gameShuttingDown)
{
	return true;
}

/*virtual*/ void Spectator::Tick(double deltaTime)
{
	// TODO: Respond to controller input here to move the camera around the subject.
	//       The subject itself responds to controller input to walk/run around.
	//       I'm going for Zelda-style controls here.  Left thumb-stick orbits the
	//       player.  Z-button moves the camera behind the player.

	// TODO: Toggle free-cam on/off with a controller button.  We don't do anything if the free cam is enabled,
	//       except turn it off if we're toggled again.

	if (!this->freeCam->IsEnabled())
	{
		Transform subjectTransform;
		this->subject->GetTransform(subjectTransform);

		Transform cameraTransform = this->camera->GetCameraToWorldTransform();

		// TODO: Ajust the camera position here in such a way that we're kind-of solving an IK problem.
	}
}