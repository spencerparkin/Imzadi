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
	 * Instances of this class represent any sort of person in the game,
	 * whether they be the main protagonist, an antogonist, or just some
	 * sort of NPC.
	 */
	class IMZADI_API Biped : public PhysicsEntity
	{
	public:
		Biped();
		virtual ~Biped();

		virtual bool Setup() override;
		virtual bool Shutdown() override;
		virtual bool Tick(TickPass tickPass, double deltaTime) override;
		virtual bool GetTransform(Transform& transform) override;
		virtual void Reset() override;

		void SetRestartLocation(const Vector3& restartLocation) { this->restartLocation = restartLocation; }
		void SetRestartOrientation(const Quaternion& restartOrientation) { this->restartOrientation = restartOrientation; }

		void SetCanRestart(bool canRestart) { this->canRestart = canRestart; }
		bool GetCanRestart() const { return this->canRestart; }

	protected:
		Vector3 restartLocation;
		Quaternion restartOrientation;
		bool canRestart;
		ShapeID collisionShapeID;
		ShapeID groundShapeID;
		Reference<RenderMeshInstance> renderMesh;
		bool inContactWithGround;
		TaskID boundsQueryTaskID;
		TaskID collisionQueryTaskID;
		TaskID groundQueryTaskID;
	};
}