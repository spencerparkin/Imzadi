#pragma once

#include "Entity.h"
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
	class IMZADI_API Biped : public Entity
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
		virtual void Reset();
		virtual std::string GetAnimName(AnimType animType);
		virtual void AdjustFacingDirection(double deltaTime);
		virtual void AccumulateForces(Vector3& netForce);
		virtual void IntegrateVelocity(const Vector3& acceleration, double deltaTime);
		virtual void IntegratePosition(double deltaTime);
		virtual bool ConstraintVelocityWithGround();
		virtual bool OnBipedDied();
		virtual uint64_t GetAdditionalUserFlagsForCollisionShape();
		virtual bool OwnsCollisionShape(ShapeID shapeID) const override;
		virtual uint32_t TickOrder() const override;

		void SetRestartLocation(const Vector3& restartLocation);
		void SetRestartOrientation(const Quaternion& restartOrientation);

		void SetCanRestart(bool canRestart) { this->canRestart = canRestart; }
		bool GetCanRestart() const { return this->canRestart; }
		bool IsInContactWithGround() const { return this->inContactWithGround; }

		void SetVelocity(const Vector3& velocity) { this->velocity = velocity; }
		const Vector3& GetVelocity() const { return this->velocity; }

		void SetMass(double mass) { this->mass = mass; }
		double GetMass() const { return this->mass; }

		RenderMeshInstance* GetRenderMesh() { return this->renderMesh.Get(); }

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
		TaskID groundSurfaceQueryTaskID;
		Transform objectToPlatform;
		Transform platformToWorld;
		Transform restartTransformObjectToWorld;
		Vector3 velocity;
		double mass;
		Vector3 groundSurfaceNormal;
		Vector3 groundSurfacePoint;
	};
}