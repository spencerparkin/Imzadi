#include "Alice.h"
#include "GameApp.h"
#include "Entities/FollowCam.h"
#include "Entities/RubiksCubeMaster.h"
#include "Entities/Pickup.h"
#include "Characters/Borggy.h"
#include "EventSystem.h"
#include "DialogSystem.h"
#include "Collision/Result.h"
#include "Collision/CollisionCache.h"
#include "Log.h"
#include "Audio/System.h"
#if defined AUTHOR_NAV_GRAPH_CAPABILITY
#include "Entities/Level.h"
#include <rapidjson/prettywriter.h>
#endif //AUTHOR_NAV_GRAPH_CAPABILITY

//------------------------------------ Alice ------------------------------------

Alice::Alice()
{
	this->cameraHandle = 0;
	this->freeCamListenerHandle = 0;
	this->triggerBoxListenerHandle = 0;
	this->rayCastQueryTaskID = 0;
	this->entityOverlapQueryTaskID = 0;
#if defined AUTHOR_NAV_GRAPH_CAPABILITY
	this->authoringNavGraph = false;
#endif //AUTHOR_NAV_GRAPH_CAPABILITY
	this->SetName("Alice");
	this->abilities->SetAbility("jump", new FloatAbility(1000.0));
	this->abilities->SetAbility("run_speed", new FloatAbility(20.0));
}

/*virtual*/ Alice::~Alice()
{
}

/*virtual*/ bool Alice::Setup()
{
	std::string modelFile = "Models/Alice/Alice.skinned_render_mesh";
	this->renderMesh.SafeSet(Imzadi::Game::Get()->LoadAndPlaceRenderMesh(modelFile));

	if (!Character::Setup())
		return false;

	Imzadi::Game::Get()->PushControllerUser("Alice");
	this->actionManager.SetControllerUser("Alice");

	auto followCam = Imzadi::Game::Get()->SpawnEntity<Imzadi::FollowCam>();
	followCam->SetSubject(this);
	followCam->SetCameraUser("Alice");
	followCam->SetCamera(Imzadi::Game::Get()->GetCamera());

	this->cameraHandle = followCam->GetHandle();

	this->triggerBoxListenerHandle = Imzadi::Game::Get()->GetEventSystem()->RegisterEventListener("TriggerBox", Imzadi::EventListenerType::TRANSITORY, new Imzadi::LambdaEventListener([=](const Imzadi::Event* event) {
		this->HandleTriggerBoxEvent((const Imzadi::TriggerBoxEvent*)event);
	}));

	this->freeCamListenerHandle = Imzadi::Game::Get()->GetEventSystem()->RegisterEventListener("FreeCam", Imzadi::EventListenerType::TRANSITORY, new Imzadi::LambdaEventListener([=](const Imzadi::Event* event) {
		this->HandleFreeCamEvent(event);
	}));

	return true;
}

/*virtual*/ bool Alice::Shutdown()
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

void Alice::ConfigureCollisionCapsule(Imzadi::Collision::CapsuleShape* capsule)
{
	capsule->SetVertex(0, Imzadi::Vector3(0.0, 1.0, 0.0));
	capsule->SetVertex(1, Imzadi::Vector3(0.0, 5.0, 0.0));
	capsule->SetRadius(1.0);
	capsule->SetUserFlags(IMZADI_SHAPE_FLAG_BIPED_ENTITY);
}

/*virtual*/ std::string Alice::GetAnimName(Imzadi::Biped::AnimType animType)
{
	switch (animType)
	{
	case Imzadi::Biped::AnimType::ABYSS_FALLING:
		return "AliceAbyssFalling";
	case Imzadi::Biped::AnimType::FATAL_LANDING:
		return "AliceFatalLanding";
	case Imzadi::Biped::AnimType::HIT_FALLING:
		return "AliceHitFalling";
	case Imzadi::Biped::AnimType::IDLE:
		return "AliceIdle";
	case Imzadi::Biped::AnimType::JUMP:
		return "AliceJumping";
	case Imzadi::Biped::AnimType::RUN:
		return "AliceRun";
	}

	return "";
}

void Alice::HandleTriggerBoxEvent(const Imzadi::TriggerBoxEvent* event)
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
				else if (triggerBoxName == "RubiksCube")
				{
					if (!((GameApp*)Imzadi::Game::Get())->GetGameProgress()->WasMileStoneReached("rubiks_cube_solved"))
					{
						auto action = new ControlRubiksCubeAction(this);
						action->masterName = "RubiksCubeMaster";
						this->actionManager.BindAction(Imzadi::Button::A_BUTTON, action);
					}
					else
					{
						auto action = new TeleportToLevelAction(this);
						action->targetLevel = "Level8";
						this->actionManager.BindAction(Imzadi::Button::A_BUTTON, action);
					}
				}
				else if (triggerBoxName.find("OpenDoor") == 0)
				{
					auto action = new OpenDoorAction(this);
					action->doorChannel = triggerBoxName.substr(9);
					this->actionManager.BindAction(Imzadi::Button::A_BUTTON, action);
				}

				break;
			}
			case Imzadi::TriggerBoxEvent::Type::SHAPE_EXITED:
			{
				IMZADI_LOG_INFO("Exited trigger box \"%s\".", triggerBoxName.c_str());

				if (triggerBoxName.find("JumpTo") == 0 ||
					triggerBoxName == "RubiksCube" ||
					triggerBoxName.find("OpenDoor") == 0)
				{
					this->actionManager.UnbindAction(Imzadi::Button::A_BUTTON);
				}

				break;
			}
		}
	}
}

/*virtual*/ void Alice::AccumulateForces(Imzadi::Vector3& netForce)
{
	Character::AccumulateForces(netForce);

	Imzadi::Input* controller = Imzadi::Game::Get()->GetController("Alice");
	if (!controller)
		return;

	if (this->inContactWithGround && controller->ButtonPressed(Imzadi::Button::Y_BUTTON))
	{
		double jumpForceValue = 0.0;
		this->abilities->GetAbilityValue("jump", jumpForceValue);
		Imzadi::Vector3 jumpForce(0.0, jumpForceValue, 0.0);
		netForce += jumpForce;
	}
}

/*virtual*/ void Alice::IntegrateVelocity(const Imzadi::Vector3& acceleration, double deltaTime)
{
	if (this->inContactWithGround)
	{
		Imzadi::Reference<ReferenceCounted> followCamRef;
		Imzadi::HandleManager::Get()->GetObjectFromHandle(this->cameraHandle, followCamRef);
		auto followCam = dynamic_cast<Imzadi::FollowCam*>(followCamRef.Get());
		if (!followCam)
			return;

		Imzadi::Vector2 leftStick(0.0, 0.0);
		Imzadi::Input* controller = Imzadi::Game::Get()->GetController("Alice");
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

		double maxMoveSpeed = 0.0;
		this->abilities->GetAbilityValue("run_speed", maxMoveSpeed);

		Imzadi::Vector3 moveDelta = (xAxis * leftStick.x - zAxis * leftStick.y) * maxMoveSpeed;
		double speed = moveDelta.Length();
		
		if (this->animationMode == Imzadi::Biped::AnimationMode::DEATH_BY_BADDY_HIT ||
			this->animationMode == Imzadi::Biped::AnimationMode::DEATH_BY_FATAL_LANDING)
		{
			speed = 0.0;
		}

		// We can stomp this when we're on the ground, because no "physics" is happening.
		this->velocity = moveDelta.RejectedFrom(this->groundSurfaceNormal);
		if (this->velocity.Normalize())
			this->velocity *= speed;

		// Transform the velocity vector from world space to platform space.
		Imzadi::Transform worldToPlatform = this->platformToWorld.Inverted();
		this->velocity = worldToPlatform.TransformVector(this->velocity);
	}

	Character::IntegrateVelocity(acceleration, deltaTime);
}

/*virtual*/ bool Alice::Tick(Imzadi::TickPass tickPass, double deltaTime)
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
		case Imzadi::TickPass::PARALLEL_WORK:
		{
			Imzadi::Input* controller = Imzadi::Game::Get()->GetController("Alice");
			if (controller)
			{
				if (controller->ButtonPressed(Imzadi::Button::R_SHOULDER))
				{
					GameProgress* gameProgress = ((GameApp*)Imzadi::Game::Get())->GetGameProgress();
					uint32_t count = gameProgress->GetPossessedItemCount("booster");
					if (count > 0)
					{
						if(abilities->OverrideAbility("run_speed", new FloatAbility(40.0), 30.0))
							gameProgress->SetPossessedItemCount("booster", count - 1);
					}
				}
			}

#if defined AUTHOR_NAV_GRAPH_CAPABILITY
			// Rather than try to procedurally generate the nav-graph as a function of
			// the collision mesh, here I'm just going to author it by hand.  This code
			// can be compiled out of the final release binary.
			if (this->authoringNavGraph && controller)
			{
				std::vector<Imzadi::Level*> foundEntityArray;
				Imzadi::Game::Get()->FindAllEntitiesOfType<Imzadi::Level>(foundEntityArray);
				if (foundEntityArray.size() == 1)
				{
					Imzadi::Level* level = foundEntityArray[0];
					Imzadi::NavGraph* navGraph = level->GetNavGraph();
					if (navGraph)
					{
						if (controller->ButtonPressed(Imzadi::Button::R_SHOULDER))
						{
							Imzadi::Transform objectToWorld;
							this->GetTransform(objectToWorld);

							auto newNode = const_cast<Imzadi::NavGraph::Node*>(navGraph->FindNearestNodeWithinDistance(objectToWorld.translation, 2.0));
							if (!newNode)
								newNode = navGraph->AddLocation(objectToWorld.translation);

							if (!this->currentNode.Get())
								this->currentNode = newNode;
							else if(newNode && newNode != this->currentNode.Get())
							{
								if (navGraph->AddPathBetweenNodes(this->currentNode, newNode))
									this->currentNode = newNode;
							}
						}
						else if (controller->ButtonPressed(Imzadi::Button::L_SHOULDER))
						{
							this->currentNode.Reset();
						}
						else if (controller->ButtonPressed(Imzadi::Button::BACK))
						{
							rapidjson::Document jsonDoc;
							if (navGraph->Save(jsonDoc))
							{
								std::ofstream fileStream;
								fileStream.open(R"(E:\ENG_DEV\Imzadi\Games\BenzoBonanza\Assets\Models\Level9\Level9.nav_graph)", std::ios::out);
								rapidjson::StringBuffer stringBuffer;
								rapidjson::PrettyWriter<rapidjson::StringBuffer> prettyWriter(stringBuffer);
								jsonDoc.Accept(prettyWriter);
								fileStream << stringBuffer.GetString();
								fileStream.close();
							}
						}
					}
				}
			}
#endif //AUTHOR_NAV_GRAPH_CAPABILITY

			break;
		}
	}

	return true;
}

void Alice::HandleEntityOverlapResults()
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
				auto borg = dynamic_cast<Borggy*>(foundEntity.Get());
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

/*virtual*/ bool Alice::OnBipedDied()
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

/*virtual*/ void Alice::Reset()
{
	Character::Reset();
}

void Alice::HandleFreeCamEvent(const Imzadi::Event* event)
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

/*virtual*/ bool Alice::HangingOnToZipLine()
{
	Imzadi::Input* controller = Imzadi::Game::Get()->GetController("Alice");
	if (!controller)
		return false;

	// A player jumps to grab a zip-line and then remains attached
	// to it until they let go of the jump button.
	return controller->ButtonDown(Imzadi::Button::Y_BUTTON);
}

/*virtual*/ std::string Alice::GetZipLineAnimationName()
{
	return "AliceZipLine";
}

/*virtual*/ bool Alice::ConstraintVelocityWithGround()
{
	double platformLandingSpeed = this->velocity.Length();

	if (platformLandingSpeed > MAX_PLATFORM_LANDING_SPEED)
		return false;

	return Biped::ConstraintVelocityWithGround();
}

/*virtual*/ void Alice::OnBipedFatalLanding()
{
	Imzadi::Game::Get()->GetAudioSystem()->PlaySound("DeathGroan");
}

/*virtual*/ void Alice::OnBipedAbyssFalling()
{
	Imzadi::Game::Get()->GetAudioSystem()->PlaySound("HelpMeAhhh");
}

/*virtual*/ void Alice::OnBipedBaddyHit()
{
	Imzadi::Game::Get()->GetAudioSystem()->PlaySound("BorgNanoProbesUgh");
}

//------------------------------------ Alice::LabeledAction ------------------------------------

Alice::LabeledAction::LabeledAction(Alice* alice)
{
	this->entityHandle = alice->GetHandle();
}

/*virtual*/ Alice::LabeledAction::~LabeledAction()
{
}

/*virtual*/ void Alice::LabeledAction::Init()
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

/*virtual*/ void Alice::LabeledAction::Deinit()
{
	Imzadi::Scene* scene = Imzadi::Game::Get()->GetScene();
	scene->RemoveRenderObject(this->textRenderObject->GetName());
	this->textRenderObject.Reset();
}

/*virtual*/ void Alice::LabeledAction::Tick(double deltaTime)
{
	this->textRenderObject->SetText(this->GetActionLabel());
	this->UpdateTransform();
}

void Alice::LabeledAction::UpdateTransform()
{
	Imzadi::Reference<ReferenceCounted> ref;
	if (Imzadi::HandleManager::Get()->GetObjectFromHandle(this->entityHandle, ref))
	{
		auto alice = dynamic_cast<Alice*>(ref.Get());
		if (alice)
		{
			Imzadi::Transform transform;
			alice->GetTransform(transform);
			transform.matrix.SetIdentity();
			transform.matrix.SetUniformScale(20.0);
			transform.translation += Imzadi::Vector3(0.0, 4.0, 0.0);
			this->textRenderObject->SetTransform(transform);
		}
	}
}

//------------------------------------ Alice::TeleportToLevelAction ------------------------------------

Alice::TeleportToLevelAction::TeleportToLevelAction(Alice* alice) : LabeledAction(alice)
{
}

/*virtual*/ Alice::TeleportToLevelAction::~TeleportToLevelAction()
{
}

/*virtual*/ bool Alice::TeleportToLevelAction::Perform()
{
	Imzadi::EventSystem* eventSystem = Imzadi::Game::Get()->GetEventSystem();
	eventSystem->SendEvent("LevelTransition", new Imzadi::Event(this->targetLevel));
	return false;	// Return false here to unbind the action once it's been performed.
}

/*virtual*/ std::string Alice::TeleportToLevelAction::GetActionLabel() const
{
	return std::format("Press \"A\" to teleport to {}.", this->targetLevel.c_str());
}

//------------------------------------ Alice::TalkToEntityAction ------------------------------------

Alice::TalkToEntityAction::TalkToEntityAction(Alice* alice) : LabeledAction(alice)
{
}

/*virtual*/ Alice::TalkToEntityAction::~TalkToEntityAction()
{
}

/*virtual*/ bool Alice::TalkToEntityAction::Perform()
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

/*virtual*/ std::string Alice::TalkToEntityAction::GetActionLabel() const
{
	return std::format("Press \"A\" to talk to {}.", this->targetEntity.c_str());
}

//------------------------------------ Alice::TalkToEntityAction ------------------------------------

Alice::CollectPickupAction::CollectPickupAction(Alice* alice) : LabeledAction(alice)
{
}

/*virtual*/ Alice::CollectPickupAction::~CollectPickupAction()
{
}

/*virtual*/ bool Alice::CollectPickupAction::Perform()
{
	this->pickup->Collect();
	return true;
}

/*virtual*/ std::string Alice::CollectPickupAction::GetActionLabel() const
{
	return std::format("Press \"B\" to {} {}.", this->pickup->GetVerb().c_str(), this->pickup->GetLabel().c_str());
}

//------------------------------------ Alice::ControlRubiksCubeAction ------------------------------------

Alice::ControlRubiksCubeAction::ControlRubiksCubeAction(Alice* alice) : LabeledAction(alice)
{
}

/*virtual*/ Alice::ControlRubiksCubeAction::~ControlRubiksCubeAction()
{
}

/*virtual*/ bool Alice::ControlRubiksCubeAction::Perform()
{
	Imzadi::Reference<Imzadi::Entity> foundEntity;
	if (Imzadi::Game::Get()->FindEntityByName(this->masterName, foundEntity))
	{
		auto master = dynamic_cast<RubiksCubeMaster*>(foundEntity.Get());
		if (master)
			master->Enable(true);
	}

	return false;
}

/*virtual*/ std::string Alice::ControlRubiksCubeAction::GetActionLabel() const
{
	return "Press \"A\" to telepathically control the Rubik's Cube.";
}

//------------------------------------ Alice::OpenDoorAction ------------------------------------

Alice::OpenDoorAction::OpenDoorAction(Alice* alice) : LabeledAction(alice)
{
}

/*virtual*/ Alice::OpenDoorAction::~OpenDoorAction()
{
}

/*virtual*/ bool Alice::OpenDoorAction::Perform()
{
	Imzadi::Game::Get()->GetEventSystem()->SendEvent(this->doorChannel, new Imzadi::Event("OpenDoor"));
	return false;
}

/*virtual*/ std::string Alice::OpenDoorAction::GetActionLabel() const
{
	return "Press \"A\" to open the door.";
}