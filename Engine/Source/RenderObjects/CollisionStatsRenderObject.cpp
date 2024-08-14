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

	auto query = new Collision::ProfileStatsQuery();
	Collision::TaskID taskID = 0;
	collisionSystem->MakeQuery(query, taskID);
	collisionSystem->FlushAllTasks();
	Collision::Result* result = collisionSystem->ObtainQueryResult(taskID);
	if (result)
	{
		auto statsResult = dynamic_cast<Collision::StringResult*>(result);
		if (statsResult)
			this->SetText(statsResult->GetText());

		delete result;
	}

	auto command = new Collision::ResetProfileDataCommand();
	collisionSystem->IssueCommand(command);

	this->SetForegroundColor(Vector3(1.0, 1.0, 1.0));
	this->SetBackgroundColor(Vector3(0.0, 0.0, 0.0));
	this->SetBackgroundAlpha(0.85);

	uint32_t flags = 0;
	flags |= Flag::ALWAYS_ON_TOP;
	flags |= Flag::LEFT_JUSTIFY;
	flags |= Flag::STICK_WITH_CAMERA_PROJ;
	flags |= Flag::MULTI_LINE;
	flags |= Flag::USE_NEWLINE_CHARS;
	flags |= Flag::DRAW_BACKGROUND;
	this->SetFlags(flags);

	double aspectRatio = Game::Get()->GetAspectRatio();

	Transform scale;
	scale.SetIdentity();
	scale.matrix.SetNonUniformScale(Vector3(1.0, aspectRatio, 1.0));

	Transform transform;
	transform.SetIdentity();
	transform.translation.SetComponents(-0.9, -0.9, -0.5);

	this->SetTransform(transform * scale);

	TextRenderObject::PreRender();
}