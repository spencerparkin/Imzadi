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
#include "Collision/Shapes/Capsule.h"
#include "EventSystem.h"

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
			JUMP,
			FATAL_LANDING,
			ABYSS_FALLING,
			HIT_FALLING
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
		virtual void OnBipedFatalLanding();
		virtual void OnBipedBaddyHit();
		virtual void OnBipedAbyssFalling();
		virtual void ConfigureCollisionCapsule(Collision::CapsuleShape* capsule);
		virtual bool OwnsCollisionShape(Collision::ShapeID shapeID) const override;
		virtual Collision::ShapeID GetGroundContactShape() const override;
		virtual uint32_t TickOrder() const override;
		virtual std::string GetInfo() const override;

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
		enum AnimationMode
		{
			BASIC_PLATFORMING,
			DEATH_BY_ABYSS_FALLING,
			DEATH_BY_BADDY_HIT,
			DEATH_BY_FATAL_LANDING
		};

		void SetAnimationMode(AnimationMode newMode);

		virtual bool ManageAnimation(double deltaTime);

		void HandleWorldSurfaceCollisionResult(Collision::CollisionQueryResult* collisionResult);

		bool canRestart;
		Collision::ShapeID collisionShapeID;
		Collision::ShapeID groundShapeID;
		Reference<RenderMeshInstance> renderMesh;
		bool inContactWithGround;
		Collision::TaskID boundsQueryTaskID;
		Collision::TaskID worldSurfaceCollisionQueryTaskID;
		Collision::TaskID groundQueryTaskID;
		Collision::TaskID groundSurfaceQueryTaskID;
		Transform objectToPlatform;
		Transform platformToWorld;
		Transform restartTransformObjectToWorld;
		Vector3 velocity;
		double mass;
		Vector3 groundSurfaceNormal;
		Vector3 groundSurfacePoint;
		AnimationMode animationMode;
		bool continuouslyUpdatePlatformTransform;
	};

	/**
	 * This event gets sent on the "Biped" channel whenever a biped is reset.
	 */
	class BipedResetEvent : public Event
	{
	public:
		BipedResetEvent(uint32_t handle)
		{
			this->handle = handle;
		}

		virtual ~BipedResetEvent()
		{
		}

		uint32_t handle;	///< This is a handle to the biped that was reset.
	};
}