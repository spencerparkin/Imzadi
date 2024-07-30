#pragma once

#include "PhysicsEntity.h"
#include "Math/Vector3.h"
#include "Math/Quaternion.h"
#include "Math/Matrix3x3.h"
#include "Scene.h"
#include "RenderObjects/RenderMeshInstance.h"
#include "Collision/Shape.h"
#include "Collision/Task.h"
#include "Collision/Result.h"

namespace Imzadi
{
	/**
	 * Instances of this class represent any sort of person in the game,
	 * whether they be the main protagonist, an antogonist, or just some
	 * sort of NPC.
	 */
	// TODO: I want to rework this so that we maintain two transforms: objectToPlatform and platformToWorld.
	//       The objectToWorld transform for the render object and the collision object is simply the concatination
	//       of these two transforms.  The controller moves the platformToWorld transform while the platform we're
	//       standing on (or jumping from) determines the objectToPlatform transform.  That's the idea, anyway.
	//       Maybe I'll find some problems with this when I get into it.
	class IMZADI_API Biped : public PhysicsEntity
	{
	public:
		Biped();
		virtual ~Biped();

		enum AnimType
		{
			IDLE,
			RUN,
			JUMP
		};

		virtual bool Setup() override;
		virtual bool Shutdown() override;
		virtual bool Tick(TickPass tickPass, double deltaTime) override;
		virtual bool GetTransform(Transform& transform) const override;
		virtual bool SetTransform(const Transform& transform) override;
		virtual void Reset() override;
		virtual std::string GetAnimName(AnimType animType);
		virtual void AdjustFacingDirection(double deltaTime);
		virtual void IntegratePosition(double deltaTime);
		virtual uint64_t GetAdditionalUserFlagsForCollisionShape();
		virtual bool OwnsCollisionShape(ShapeID shapeID) const override;

		void SetRestartLocation(const Vector3& restartLocation) { this->restartLocation = restartLocation; }
		void SetRestartOrientation(const Quaternion& restartOrientation) { this->restartOrientation = restartOrientation; }

		void SetCanRestart(bool canRestart) { this->canRestart = canRestart; }
		bool GetCanRestart() const { return this->canRestart; }

	protected:
		void HandleWorldSurfaceCollisionResult(CollisionQueryResult* collisionResult);

		Vector3 restartLocation;
		Quaternion restartOrientation;
		bool canRestart;
		ShapeID collisionShapeID;
		ShapeID groundShapeID;
		Reference<RenderMeshInstance> renderMesh;
		bool inContactWithGround;
		TaskID boundsQueryTaskID;
		TaskID worldSurfaceCollisionQueryTaskID;
		TaskID groundQueryTaskID;
	};
}