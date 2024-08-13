#include "CollisionStatsRenderObject.h"
#include "Collision/Command.h"
#include "Collision/Query.h"
#include "Collision/Result.h"
#include "Game.h"

using namespace Imzadi;

CollisionStatsRenderObject::CollisionStatsRenderObject()
{
}

/*virtual*/ CollisionStatsRenderObject::~CollisionStatsRenderObject()
{
}

/*virtual*/ void CollisionStatsRenderObject::PreRender()
{
	Collision::System* collisionSystem = Game::Get()->GetCollisionSystem();

	this->SetText("?");

	auto query = Collision::ProfileStatsQuery::Create();
	Collision::TaskID taskID = 0;
	collisionSystem->MakeQuery(query, taskID);
	collisionSystem->FlushAllTasks();
	Collision::Result* result = collisionSystem->ObtainQueryResult(taskID);
	if (result)
	{
		auto statsResult = dynamic_cast<Collision::StringResult*>(result);
		if (statsResult)
			this->SetText(statsResult->GetText());

		collisionSystem->Free(result);
	}

	auto command = Collision::ResetProfileDataCommand::Create();
	collisionSystem->IssueCommand(command);

	this->SetForegroundColor(Vector3(1.0, 0.0, 0.0));

	uint32_t flags = 0;
	flags |= Flag::ALWAYS_ON_TOP;
	flags |= Flag::LEFT_JUSTIFY;
	flags |= Flag::STICK_WITH_CAMERA_PROJ;
	flags |= Flag::MULTI_LINE;
	flags |= Flag::USE_NEWLINE_CHARS;
	this->SetFlags(flags);

	double aspectRatio = Game::Get()->GetAspectRatio();

	Transform scale;
	scale.SetIdentity();
	scale.matrix.SetNonUniformScale(Vector3(1.0, aspectRatio, 1.0));

	Transform transform;
	transform.SetIdentity();
	transform.translation.SetComponents(0.0, -0.9, -0.5);

	this->SetTransform(transform * scale);

	TextRenderObject::PreRender();
}