#include "DeannaTroi.h"
#include "GameApp.h"
#include "Entities/FollowCam.h"
#include "EventSystem.h"
#include "DialogSystem.h"
#include "Collision/Result.h"
#include "Log.h"

//------------------------------------ DeannaTroi ------------------------------------

DeannaTroi::DeannaTroi()
{
	this->cameraHandle = 0;
	this->maxMoveSpeed = 20.0;
	this->triggerBoxListenerHandle = 0;
	this->rayCastQueryTaskID = 0;
	this->SetName("Deanna");
}

/*virtual*/ DeannaTroi::~DeannaTroi()
{
}

/*virtual*/ bool DeannaTroi::Setup()
{
	std::string modelFile = "Models/DeannaTroi/Troi.skinned_render_mesh";
	this->renderMesh.SafeSet(Imzadi::Game::Get()->LoadAndPlaceRenderMesh(modelFile));

	if (!Biped::Setup())
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
	Biped::Shutdown();

	this->actionManager.Clear();

	Imzadi::Game::Get()->PopControllerUser();

	if (this->triggerBoxListenerHandle)
	{
		Imzadi::Game::Get()->GetEventSystem()->UnregisterEventListener(this->triggerBoxListenerHandle);
		this->triggerBoxListenerHandle = 0;
	}

	return true;
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
					this->actionManager.BindAction(XINPUT_GAMEPAD_A, action);
				}

				break;
			}
			case Imzadi::TriggerBoxEvent::Type::SHAPE_EXITED:
			{
				IMZADI_LOG_INFO("Exited trigger box \"%s\".", triggerBoxName.c_str());

				if (triggerBoxName.find("JumpTo") == 0)
					this->actionManager.UnbindAction(XINPUT_GAMEPAD_A);

				break;
			}
		}
	}
}

/*virtual*/ void DeannaTroi::AccumulateForces(Imzadi::Vector3& netForce)
{
	Biped::AccumulateForces(netForce);

	Imzadi::Controller* controller = Imzadi::Game::Get()->GetController("DeannaTroi");
	if (!controller)
		return;

	if (this->inContactWithGround && controller->ButtonPressed(XINPUT_GAMEPAD_Y))
	{
		Imzadi::Vector3 jumpForce(0.0, 1000.0, 0.0);
		netForce += jumpForce;
	}
}

/*virtual*/ void DeannaTroi::IntegrateVelocity(const Imzadi::Vector3& acceleration, double deltaTime)
{
	if (this->inContactWithGround)
	{
		Imzadi::Reference<ReferenceCounted> followCamRef;
		Imzadi::HandleManager::Get()->GetObjectFromHandle(this->cameraHandle, followCamRef);
		auto followCam = dynamic_cast<Imzadi::FollowCam*>(followCamRef.Get());
		if (!followCam)
			return;

		Imzadi::Vector2 leftStick(0.0, 0.0);
		Imzadi::Controller* controller = Imzadi::Game::Get()->GetController("DeannaTroi");
		if (controller)
			controller->GetAnalogJoyStick(Imzadi::Controller::Side::LEFT, leftStick.x, leftStick.y);

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

		// We can stomp this when we're on the ground, because no "physics" is happening.
		this->velocity = moveDelta.RejectedFrom(this->groundSurfaceNormal);
	}

	Biped::IntegrateVelocity(acceleration, deltaTime);
}

/*virtual*/ bool DeannaTroi::Tick(Imzadi::TickPass tickPass, double deltaTime)
{
	if (!Biped::Tick(tickPass, deltaTime))
		return false;

	this->actionManager.Tick(deltaTime);

	switch (tickPass)
	{
		case Imzadi::TickPass::SUBMIT_COLLISION_QUERIES:
		{
			Imzadi::CollisionSystem* collisionSystem = Imzadi::Game::Get()->GetCollisionSystem();

			Imzadi::Transform transform;
			this->GetTransform(transform);

			Imzadi::Ray ray;
			ray.unitDirection = -transform.matrix.GetColumnVector(2);
			ray.origin = transform.translation + ray.unitDirection * 2.0;
			ray.origin.y += 2.0;

			auto rayCastQuery = Imzadi::RayCastQuery::Create();
			rayCastQuery->SetRay(ray);
			collisionSystem->MakeQuery(rayCastQuery, this->rayCastQueryTaskID);

#if 0
			Imzadi::DebugLines* debugLines = Imzadi::Game::Get()->GetDebugLines();
			Imzadi::DebugLines::Line line;
			line.color.SetComponents(1.0, 0.0, 0.0);
			line.segment.point[0] = ray.origin;
			line.segment.point[1] = ray.origin + ray.unitDirection * 100.0;
			debugLines->AddLine(line);
#endif

			break;
		}
		case Imzadi::TickPass::RESOLVE_COLLISIONS:
		{
			Imzadi::CollisionSystem* collisionSystem = Imzadi::Game::Get()->GetCollisionSystem();

			if (this->rayCastQueryTaskID)
			{
				Imzadi::Result* result = collisionSystem->ObtainQueryResult(this->rayCastQueryTaskID);
				if (result)
				{
					auto rayCastResult = dynamic_cast<Imzadi::RayCastResult*>(result);
					if (rayCastResult)
					{
						const Imzadi::RayCastResult::HitData& hitData = rayCastResult->GetHitData();
						if (hitData.shape)
						{
							auto gameApp = (GameApp*)Imzadi::Game::Get();
							uint64_t userFlags = hitData.shape->GetUserFlags();
							if ((userFlags & SHAPE_FLAG_TALKER) != 0 && hitData.alpha < 10.0 && !gameApp->GetDialogSystem()->PresentlyEngagedInConversation())
							{
								Imzadi::Reference<Imzadi::Entity> entity;
								if (Imzadi::Game::Get()->FindEntityByShapeID(hitData.shapeID, entity))
								{
									if (!this->actionManager.IsBound(XINPUT_GAMEPAD_A))
									{
										auto action = new TalkToEntityAction(this);
										action->targetEntity = entity->GetName();
										this->actionManager.BindAction(XINPUT_GAMEPAD_A, action);
									}
								}
							}
							else
							{
								if (dynamic_cast<TalkToEntityAction*>(this->actionManager.GetBoundAction(XINPUT_GAMEPAD_A)))
									this->actionManager.UnbindAction(XINPUT_GAMEPAD_A);
							}
						}
					}

					collisionSystem->Free(result);
				}
			}

			break;
		}
	}

	return true;
}

/*virtual*/ void DeannaTroi::Reset()
{
	// TODO: This is where we might incur a penalty for dying.

	Biped::Reset();
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
	this->sceneObjectName = scene->AddRenderObject(this->textRenderObject.Get());
}

/*virtual*/ void DeannaTroi::LabeledAction::Deinit()
{
	Imzadi::Scene* scene = Imzadi::Game::Get()->GetScene();
	scene->RemoveRenderObject(this->sceneObjectName);
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