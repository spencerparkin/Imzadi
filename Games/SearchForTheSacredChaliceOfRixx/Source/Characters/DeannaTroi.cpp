#include "DeannaTroi.h"
#include "GameApp.h"
#include "Entities/FollowCam.h"
#include "Characters/Borg.h"
#include "EventSystem.h"
#include "DialogSystem.h"
#include "Collision/Result.h"
#include "Collision/CollisionCache.h"
#include "Log.h"
#include "Pickup.h"
#include "Audio/System.h"

//------------------------------------ DeannaTroi ------------------------------------

DeannaTroi::DeannaTroi()
{
	this->cameraHandle = 0;
	this->maxMoveSpeed = 20.0;
	this->triggerBoxListenerHandle = 0;
	this->rayCastQueryTaskID = 0;
	this->entityOverlapQueryTaskID = 0;
	this->SetName("Deanna");
}

/*virtual*/ DeannaTroi::~DeannaTroi()
{
}

/*virtual*/ bool DeannaTroi::Setup()
{
	std::string modelFile = "Models/DeannaTroi/Troi.skinned_render_mesh";
	this->renderMesh.SafeSet(Imzadi::Game::Get()->LoadAndPlaceRenderMesh(modelFile));

	if (!Character::Setup())
		return false;

	Imzadi::Game::Get()->PushControllerUser("DeannaTroi");
	this->actionManager.SetControllerUser("DeannaTroi");

	auto followCam = Imzadi::Game::Get()->SpawnEntity<Imzadi::FollowCam>();
	followCam->SetSubject(this);
	followCam->SetCameraUser("DeannaTroi");
	followCam->SetCamera(Imzadi::Game::Get()->GetCamera());

	this->cameraHandle = followCam->GetHandle();

	this->triggerBoxListenerHandle = Imzadi::Game::Get()->GetEventSystem()->RegisterEventListener("TriggerBox", new Imzadi::LambdaEventListener([=](const Imzadi::Event* event) {
		this->HandleTriggerBoxEvent((const Imzadi::TriggerBoxEvent*)event);
	}));

#if defined _DEBUG
	this->freeCamListenerHandle = Imzadi::Game::Get()->GetEventSystem()->RegisterEventListener("FreeCam", new Imzadi::LambdaEventListener([=](const Imzadi::Event* event) {
		this->HandleFreeCamEvent(event);
	}));
#endif

	return true;
}

/*virtual*/ bool DeannaTroi::Shutdown()
{
	Character::Shutdown();

	this->actionManager.Clear();

	Imzadi::Game::Get()->PopControllerUser();

	if (this->triggerBoxListenerHandle)
	{
		Imzadi::Game::Get()->GetEventSystem()->UnregisterEventListener(this->triggerBoxListenerHandle);
		this->triggerBoxListenerHandle = 0;
	}

	return true;
}

void DeannaTroi::ConfigureCollisionCapsule(Imzadi::Collision::CapsuleShape* capsule)
{
	capsule->SetVertex(0, Imzadi::Vector3(0.0, 1.0, 0.0));
	capsule->SetVertex(1, Imzadi::Vector3(0.0, 5.0, 0.0));
	capsule->SetRadius(1.0);
	capsule->SetUserFlags(IMZADI_SHAPE_FLAG_BIPED_ENTITY);
}

/*virtual*/ std::string DeannaTroi::GetAnimName(Imzadi::Biped::AnimType animType)
{
	switch (animType)
	{
	case Imzadi::Biped::AnimType::IDLE:
		return "Idle";
	case Imzadi::Biped::AnimType::JUMP:
		return "Jumping";
	case Imzadi::Biped::AnimType::RUN:
		return "Run";
	case Imzadi::Biped::AnimType::ABYSS_FALLING:
		return "DeannaTroiAbyssFalling";
	case Imzadi::Biped::AnimType::FATAL_LANDING:
		return "DeannaTroiFatalLanding";
	case Imzadi::Biped::AnimType::HIT_FALLING:
		return "DeannaTroiHitFalling";
	}

	return "";
}

void DeannaTroi::HandleTriggerBoxEvent(const Imzadi::TriggerBoxEvent* event)
{
	if (this->collisionShapeID == event->shapeID)
	{
		const std::string& triggerBoxName = event->GetName();
		switch (event->type)
		{
			case Imzadi::TriggerBoxEvent::Type::SHAPE_ENTERED:
			{
				IMZADI_LOG_INFO("Entered trigger box \"%s\".", triggerBoxName.c_str());

				if (triggerBoxName.find("JumpTo") == 0)
				{
					auto action = new TeleportToLevelAction(this);
					action->targetLevel = triggerBoxName.substr(6);
					this->actionManager.BindAction(Imzadi::Button::A_BUTTON, action);
				}

				break;
			}
			case Imzadi::TriggerBoxEvent::Type::SHAPE_EXITED:
			{
				IMZADI_LOG_INFO("Exited trigger box \"%s\".", triggerBoxName.c_str());

				if (triggerBoxName.find("JumpTo") == 0)
					this->actionManager.UnbindAction(Imzadi::Button::A_BUTTON);

				break;
			}
		}
	}
}

/*virtual*/ void DeannaTroi::AccumulateForces(Imzadi::Vector3& netForce)
{
	Character::AccumulateForces(netForce);

	Imzadi::Input* controller = Imzadi::Game::Get()->GetController("DeannaTroi");
	if (!controller)
		return;

	if (this->inContactWithGround && controller->ButtonPressed(Imzadi::Button::Y_BUTTON))
	{
		Imzadi::Vector3 jumpForce(0.0, 1000.0, 0.0);
		netForce += jumpForce;
	}
}

/*virtual*/ void DeannaTroi::IntegrateVelocity(const Imzadi::Vector3& acceleration, double deltaTime)
{
	if (this->inContactWithGround && this->animationMode != Imzadi::Biped::AnimationMode::DEATH_BY_BADDY_HIT)
	{
		Imzadi::Reference<ReferenceCounted> followCamRef;
		Imzadi::HandleManager::Get()->GetObjectFromHandle(this->cameraHandle, followCamRef);
		auto followCam = dynamic_cast<Imzadi::FollowCam*>(followCamRef.Get());
		if (!followCam)
			return;

		Imzadi::Vector2 leftStick(0.0, 0.0);
		Imzadi::Input* controller = Imzadi::Game::Get()->GetController("DeannaTroi");
		if (controller)
			leftStick = controller->GetAnalogJoyStick(Imzadi::Button::L_JOY_STICK);

		Imzadi::Camera* camera = followCam->GetCamera();
		if (!camera)
			return;

		Imzadi::Transform cameraToWorld = camera->GetCameraToWorldTransform();

		Imzadi::Vector3 xAxis, yAxis, zAxis;
		cameraToWorld.matrix.GetColumnVectors(xAxis, yAxis, zAxis);

		Imzadi::Vector3 upVector(0.0, 1.0, 0.0);
		xAxis = xAxis.RejectedFrom(upVector).Normalized();
		zAxis = zAxis.RejectedFrom(upVector).Normalized();

		Imzadi::Vector3 moveDelta = (xAxis * leftStick.x - zAxis * leftStick.y) * this->maxMoveSpeed;
		double speed = moveDelta.Length();

		// We can stomp this when we're on the ground, because no "physics" is happening.
		this->velocity = moveDelta.RejectedFrom(this->groundSurfaceNormal);
		if (this->velocity.Normalize())
			this->velocity *= speed;
	}

	Character::IntegrateVelocity(acceleration, deltaTime);
}

/*virtual*/ bool DeannaTroi::Tick(Imzadi::TickPass tickPass, double deltaTime)
{
	if (!Character::Tick(tickPass, deltaTime))
		return false;

	this->actionManager.Tick(deltaTime);

	switch (tickPass)
	{
		case Imzadi::TickPass::SUBMIT_COLLISION_QUERIES:
		{
			Imzadi::Collision::System* collisionSystem = Imzadi::Game::Get()->GetCollisionSystem();

			Imzadi::Transform transform;
			this->GetTransform(transform);

			Imzadi::Ray ray;
			ray.unitDirection = -transform.matrix.GetColumnVector(2);
			ray.origin = transform.translation + ray.unitDirection * 2.0;
			ray.origin.y += 2.0;

			auto rayCastQuery = new Imzadi::Collision::RayCastQuery();
			rayCastQuery->SetRay(ray);
			rayCastQuery->SetUserFlagsMask(SHAPE_FLAG_TALKER);
			collisionSystem->MakeQuery(rayCastQuery, this->rayCastQueryTaskID);

#if 0
			Imzadi::DebugLines* debugLines = Imzadi::Game::Get()->GetDebugLines();
			Imzadi::DebugLines::Line line;
			line.color.SetComponents(1.0, 0.0, 0.0);
			line.segment.point[0] = ray.origin;
			line.segment.point[1] = ray.origin + ray.unitDirection * 100.0;
			debugLines->AddLine(line);
#endif

			if (this->animationMode != Imzadi::Biped::AnimationMode::DEATH_BY_BADDY_HIT)
			{
				auto entityOverlapQuery = new Imzadi::Collision::CollisionQuery();
				entityOverlapQuery->SetShapeID(this->collisionShapeID);
				entityOverlapQuery->SetUserFlagsMask(SHAPE_FLAG_BADDY | SHAPE_FLAG_PICKUP);
				collisionSystem->MakeQuery(entityOverlapQuery, this->entityOverlapQueryTaskID);
			}

			break;
		}
		case Imzadi::TickPass::RESOLVE_COLLISIONS:
		{
			Imzadi::Collision::System* collisionSystem = Imzadi::Game::Get()->GetCollisionSystem();

			if (this->rayCastQueryTaskID)
			{
				Imzadi::Collision::Result* result = collisionSystem->ObtainQueryResult(this->rayCastQueryTaskID);
				if (result)
				{
					auto rayCastResult = dynamic_cast<Imzadi::Collision::RayCastResult*>(result);
					if (rayCastResult)
					{
						bool unbindTalkActionIfAny = true;

						const Imzadi::Collision::RayCastResult::HitData& hitData = rayCastResult->GetHitData();
						if (hitData.shape)
						{
							auto gameApp = (GameApp*)Imzadi::Game::Get();
							IMZADI_ASSERT((hitData.shape->GetUserFlags() & SHAPE_FLAG_TALKER) != 0);
							static double maxTalkingDistance = 10.0;
							if (hitData.alpha < maxTalkingDistance && !gameApp->GetDialogSystem()->PresentlyEngagedInConversation())
							{
								Imzadi::Reference<Imzadi::Entity> entity;
								if (Imzadi::Game::Get()->FindEntityByShapeID(hitData.shapeID, entity))
								{
									unbindTalkActionIfAny = false;

									if (!this->actionManager.IsBound(Imzadi::Button::A_BUTTON))
									{
										auto action = new TalkToEntityAction(this);
										action->targetEntity = entity->GetName();
										this->actionManager.BindAction(Imzadi::Button::A_BUTTON, action);
									}
								}
							}
						}

						if (unbindTalkActionIfAny && dynamic_cast<TalkToEntityAction*>(this->actionManager.GetBoundAction(Imzadi::Button::A_BUTTON)))
							this->actionManager.UnbindAction(Imzadi::Button::A_BUTTON);
					}

					delete result;
				}
			}

			this->HandleEntityOverlapResults();

			break;
		}
	}

	return true;
}

void DeannaTroi::HandleEntityOverlapResults()
{
	if (!this->entityOverlapQueryTaskID)
		return;

	Imzadi::Collision::System* collisionSystem = Imzadi::Game::Get()->GetCollisionSystem();
	std::unique_ptr<Imzadi::Collision::Result> result(collisionSystem->ObtainQueryResult(this->entityOverlapQueryTaskID));
	if (!result)
		return;

	auto collisionResult = dynamic_cast<Imzadi::Collision::CollisionQueryResult*>(result.get());
	if (!collisionResult)
		return;
	
	bool unbindPickupActionIfAny = true;

	for (auto& collisionPair : collisionResult->GetCollisionStatusArray())
	{
		Imzadi::Collision::ShapeID shapeID = collisionPair->GetOtherShape(this->collisionShapeID);
		const Imzadi::Collision::Shape* shape = collisionPair->GetShape(shapeID);
		uint64_t userFlags = shape->GetUserFlags();
		if ((userFlags & SHAPE_FLAG_BADDY) != 0)
		{
			this->SetAnimationMode(Imzadi::Biped::AnimationMode::DEATH_BY_BADDY_HIT);

			Imzadi::Reference<Imzadi::Entity> foundEntity;
			if (Imzadi::Game::Get()->FindEntityByShapeID(shapeID, foundEntity))
			{
				auto borg = dynamic_cast<Borg*>(foundEntity.Get());
				if (borg)
					borg->assimulatedHuman = true;
			}

			break;
		}
		else if ((userFlags & SHAPE_FLAG_PICKUP) != 0)
		{
			Imzadi::Reference<Imzadi::Entity> foundEntity;
			if (Imzadi::Game::Get()->FindEntityByShapeID(shapeID, foundEntity))
			{
				auto pickup = dynamic_cast<Pickup*>(foundEntity.Get());
				if (pickup && pickup->CanBePickedUp())
				{
					unbindPickupActionIfAny = false;

					if (!this->actionManager.IsBound(Imzadi::Button::B_BUTTON))
					{
						auto action = new CollectPickupAction(this);
						action->pickup = pickup;
						this->actionManager.BindAction(Imzadi::Button::B_BUTTON, action);
					}
				}
			}
		}
	}

	if (unbindPickupActionIfAny && dynamic_cast<CollectPickupAction*>(this->actionManager.GetBoundAction(Imzadi::Button::B_BUTTON)))
		this->actionManager.UnbindAction(Imzadi::Button::B_BUTTON);
}

/*virtual*/ bool DeannaTroi::OnBipedDied()
{
	auto game = (GameApp*)Imzadi::Game::Get();
	GameProgress* gameProgress = game->GetGameProgress();
	int numLives = gameProgress->GetNumLives();
	numLives--;
	gameProgress->SetNumLives(numLives);
	if (numLives <= 0)
		return false;
	return Character::OnBipedDied();
}

/*virtual*/ void DeannaTroi::Reset()
{
	Character::Reset();
}

#if defined _DEBUG
void DeannaTroi::HandleFreeCamEvent(const Imzadi::Event* event)
{
	auto teleportEvent = dynamic_cast<const Imzadi::FreeCamTeleportEvent*>(event);
	if (teleportEvent)
	{
		this->inContactWithGround = false;
		this->objectToPlatform.SetIdentity();
		this->platformToWorld = teleportEvent->transform;
		this->platformToWorld.matrix.SetIdentity();
		this->velocity.SetComponents(0.0, 0.0, 0.0);
	}
}
#endif //_DEBUG

/*virtual*/ bool DeannaTroi::HangingOnToZipLine()
{
	Imzadi::Input* controller = Imzadi::Game::Get()->GetController("DeannaTroi");
	if (!controller)
		return false;

	// A player jumps to grab a zip-line and then remains attached
	// to it until they let go of the jump button.
	return controller->ButtonDown(Imzadi::Button::Y_BUTTON);
}

/*virtual*/ std::string DeannaTroi::GetZipLineAnimationName()
{
	return "DeannaZipLine";
}

/*virtual*/ bool DeannaTroi::ConstraintVelocityWithGround()
{
	double platformLandingSpeed = this->velocity.Length();

	if (platformLandingSpeed > MAX_PLATFORM_LANDING_SPEED)
		return false;

	return Biped::ConstraintVelocityWithGround();
}

/*virtual*/ void DeannaTroi::OnBipedFatalLanding()
{
	Imzadi::Game::Get()->GetAudioSystem()->PlaySound("DeathGroan");
}

/*virtual*/ void DeannaTroi::OnBipedAbyssFalling()
{
	Imzadi::Game::Get()->GetAudioSystem()->PlaySound("HelpMeAhhh");
}

/*virtual*/ void DeannaTroi::OnBipedBaddyHit()
{
	Imzadi::Game::Get()->GetAudioSystem()->PlaySound("BorgNanoProbesUgh");
}

//------------------------------------ DeannaTroi::LabeledAction ------------------------------------

DeannaTroi::LabeledAction::LabeledAction(DeannaTroi* troi)
{
	this->entityHandle = troi->GetHandle();
}

/*virtual*/ DeannaTroi::LabeledAction::~LabeledAction()
{
}

/*virtual*/ void DeannaTroi::LabeledAction::Init()
{
	uint32_t flags =
		Imzadi::TextRenderObject::Flag::ALWAYS_FACING_CAMERA |
		Imzadi::TextRenderObject::Flag::ALWAYS_ON_TOP |
		Imzadi::TextRenderObject::Flag::CENTER_JUSTIFY;

	this->textRenderObject = new Imzadi::TextRenderObject();
	this->textRenderObject->SetText(this->GetActionLabel());
	this->textRenderObject->SetFont("Roboto_Regular");
	this->textRenderObject->SetForegroundColor(Imzadi::Vector3(1.0, 0.0, 0.0));
	this->textRenderObject->SetFlags(flags);
	
	this->UpdateTransform();

	Imzadi::Scene* scene = Imzadi::Game::Get()->GetScene();
	scene->AddRenderObject(this->textRenderObject.Get());
}

/*virtual*/ void DeannaTroi::LabeledAction::Deinit()
{
	Imzadi::Scene* scene = Imzadi::Game::Get()->GetScene();
	scene->RemoveRenderObject(this->textRenderObject->GetName());
	this->textRenderObject.Reset();
}

/*virtual*/ void DeannaTroi::LabeledAction::Tick(double deltaTime)
{
	this->textRenderObject->SetText(this->GetActionLabel());
	this->UpdateTransform();
}

void DeannaTroi::LabeledAction::UpdateTransform()
{
	Imzadi::Reference<ReferenceCounted> ref;
	if (Imzadi::HandleManager::Get()->GetObjectFromHandle(this->entityHandle, ref))
	{
		auto troi = dynamic_cast<DeannaTroi*>(ref.Get());
		if (troi)
		{
			Imzadi::Transform transform;
			troi->GetTransform(transform);
			transform.matrix.SetIdentity();
			transform.matrix.SetUniformScale(20.0);
			this->textRenderObject->SetTransform(transform);
		}
	}
}

//------------------------------------ DeannaTroi::TeleportToLevelAction ------------------------------------

DeannaTroi::TeleportToLevelAction::TeleportToLevelAction(DeannaTroi* troi) : LabeledAction(troi)
{
}

/*virtual*/ DeannaTroi::TeleportToLevelAction::~TeleportToLevelAction()
{
}

/*virtual*/ bool DeannaTroi::TeleportToLevelAction::Perform()
{
	Imzadi::EventSystem* eventSystem = Imzadi::Game::Get()->GetEventSystem();
	eventSystem->SendEvent("LevelTransition", new Imzadi::Event(this->targetLevel));
	return false;	// Return false here to unbind the action once it's been performed.
}

/*virtual*/ std::string DeannaTroi::TeleportToLevelAction::GetActionLabel() const
{
	return std::format("Press \"A\" to teleport to {}.", this->targetLevel.c_str());
}

//------------------------------------ DeannaTroi::TalkToEntityAction ------------------------------------

DeannaTroi::TalkToEntityAction::TalkToEntityAction(DeannaTroi* troi) : LabeledAction(troi)
{
}

/*virtual*/ DeannaTroi::TalkToEntityAction::~TalkToEntityAction()
{
}

/*virtual*/ bool DeannaTroi::TalkToEntityAction::Perform()
{
	Imzadi::Reference<Entity> foundEntity;
	if (Imzadi::Game::Get()->FindEntityByName(this->targetEntity, foundEntity))
	{
		Imzadi::EventSystem* eventSystem = Imzadi::Game::Get()->GetEventSystem();
		auto convoEvent = new ConversationEvent();
		convoEvent->participantHandleArray.push_back(foundEntity->GetHandle());
		convoEvent->participantHandleArray.push_back(this->entityHandle);
		eventSystem->SendEvent("Conversation", convoEvent);
	}

	return false;
}

/*virtual*/ std::string DeannaTroi::TalkToEntityAction::GetActionLabel() const
{
	return std::format("Press \"A\" to talk to {}.", this->targetEntity.c_str());
}

//------------------------------------ DeannaTroi::TalkToEntityAction ------------------------------------

DeannaTroi::CollectPickupAction::CollectPickupAction(DeannaTroi* troi) : LabeledAction(troi)
{
}

/*virtual*/ DeannaTroi::CollectPickupAction::~CollectPickupAction()
{
}

/*virtual*/ bool DeannaTroi::CollectPickupAction::Perform()
{
	this->pickup->Collect();
	return true;
}

/*virtual*/ std::string DeannaTroi::CollectPickupAction::GetActionLabel() const
{
	return std::format("Press \"B\" to {} {}.", this->pickup->GetVerb().c_str(), this->pickup->GetLabel().c_str());
}