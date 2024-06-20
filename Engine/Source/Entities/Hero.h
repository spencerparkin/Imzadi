#include "PhysicsEntity.h"
#include "Math/Vector3.h"
#include "Math/Quaternion.h"
#include "Math/Matrix3x3.h"
#include "Scene.h"
#include "RenderObjects/RenderMeshInstance.h"
#include "Collision/Shape.h"
#include "Collision/Task.h"

namespace Imzadi
{
	/**
	 * An instance of this class is the protagonist of our game saga.
	 */
	class IMZADI_API Hero : public PhysicsEntity
	{
	public:
		Hero();
		virtual ~Hero();

		virtual bool Setup() override;
		virtual bool Shutdown(bool gameShuttingDown) override;
		virtual bool Tick(TickPass tickPass, double deltaTime) override;
		virtual bool GetTransform(Transform& transform) override;
		virtual void AccumulateForces(Vector3& netForce) override;
		virtual void IntegrateVelocity(const Vector3& acceleration, double deltaTime) override;
		virtual void Reset() override;

		void SetRestartLocation(const Vector3& restartLocation) { this->restartLocation = restartLocation; }
		void SetRestartOrientation(const Quaternion& restartOrientation) { this->restartOrientation = restartOrientation; }

	protected:
		Vector3 restartLocation;
		Quaternion restartOrientation;
		ShapeID shapeID;
		Reference<RenderMeshInstance> renderMesh;
		uint32_t cameraHandle;
		double maxMoveSpeed;
		bool inContactWithGround;
		TaskID boundsQueryTaskID;
		TaskID collisionQueryTaskID;
	};
}