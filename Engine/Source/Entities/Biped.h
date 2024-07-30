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

		void SetRestartLocation(const Vector3& restartLocation);
		void SetRestartOrientation(const Quaternion& restartOrientation);

		void SetCanRestart(bool canRestart) { this->canRestart = canRestart; }
		bool GetCanRestart() const { return this->canRestart; }

	protected:
		void HandleWorldSurfaceCollisionResult(CollisionQueryResult* collisionResult);

		bool canRestart;
		ShapeID collisionShapeID;
		ShapeID groundShapeID;
		Reference<RenderMeshInstance> renderMesh;
		bool inContactWithGround;
		TaskID boundsQueryTaskID;
		TaskID worldSurfaceCollisionQueryTaskID;
		TaskID groundQueryTaskID;
		Transform objectToPlatform;
		Transform platformToWorld;
		Transform restartTransformObjectToWorld;
	};
}