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
	CollisionSystem* collisionSystem = Game::Get()->GetCollisionSystem();

	this->SetText("?");

	auto query = ProfileStatsQuery::Create();
	TaskID taskID = 0;
	collisionSystem->MakeQuery(query, taskID);
	collisionSystem->FlushAllTasks();
	Result* result = collisionSystem->ObtainQueryResult(taskID);
	if (result)
	{
		auto statsResult = dynamic_cast<StringResult*>(result);
		if (statsResult)
			this->SetText(statsResult->GetText());

		collisionSystem->Free(result);
	}

	auto command = ResetProfileDataCommand::Create();
	collisionSystem->IssueCommand(command);

	this->SetForegroundColor(Vector3(1.0, 0.0, 0.0));

	uint32_t flags = 0;
	flags |= Flag::ALWAYS_ON_TOP;
	flags |= Flag::LEFT_JUSTIFY;
	flags |= Flag::STICK_WITH_CAMERA_PROJ;
	flags |= Flag::MULTI_LINE;
	flags |= Flag::USE_NEWLINE_CHARS;
	this->SetFlags(flags);

	const D3D11_VIEWPORT* viewport = Game::Get()->GetViewportInfo();
	double aspectRatio = double(viewport->Width) / double(viewport->Height);

	Transform scale;
	scale.SetIdentity();
	scale.matrix.SetNonUniformScale(Vector3(1.0, aspectRatio, 1.0));

	Transform transform;
	transform.SetIdentity();
	transform.translation.SetComponents(0.0, -0.9, -0.5);

	this->SetTransform(transform * scale);

	TextRenderObject::Prepare();
}