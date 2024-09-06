#include "Scene.h"
#include "Camera.h"
#include "Game.h"
#include "RenderObjects/DebugLines.h"
#include <algorithm>
#include <format>

using namespace Imzadi;

//--------------------------- Scene ---------------------------

Scene::Scene()
{
}

/*virtual*/ Scene::~Scene()
{
	this->Clear();
}

void Scene::Clear()
{
	this->renderObjectMap.clear();
}

bool Scene::AddRenderObject(Reference<RenderObject> renderObject, bool adjustNameIfNecessary /*= true*/)
{
	std::string name = renderObject->GetName();
	if (this->renderObjectMap.find(name) != this->renderObjectMap.end())
	{
		if (!adjustNameIfNecessary)
			return false;

		int i = 0;
		std::string alternativeName;
		do
		{
			alternativeName = std::format("{}_{}", name.c_str(), i++);
		} while (this->renderObjectMap.find(alternativeName) != this->renderObjectMap.end());

		name = alternativeName;
		renderObject->SetName(name);
	}

	this->renderObjectMap.insert(std::pair<std::string, Reference<RenderObject>>(name, renderObject));
	renderObject->OnPostAdded();
	return true;
}

bool Scene::RemoveRenderObject(const std::string& name, Reference<RenderObject>* renderObject /*= nullptr*/)
{
	RenderObjectMap::iterator iter = this->renderObjectMap.find(name);
	if (iter == this->renderObjectMap.end())
		return false;

	if (renderObject)
		renderObject->Set(iter->second);

	iter->second->OnPreRemoved();
	this->renderObjectMap.erase(iter);
	return true;
}

bool Scene::FindRenderObject(const std::string& name, Reference<RenderObject>& renderObject)
{
	RenderObjectMap::iterator iter = this->renderObjectMap.find(name);
	if (iter == this->renderObjectMap.end())
		return false;

	renderObject.Set(iter->second);
	return true;
}

void Scene::DrawVisibilityBoxes()
{
	DebugLines* debugLines = Game::Get()->GetDebugLines();

	for (auto pair : this->renderObjectMap)
	{
		RenderObject* renderObject = pair.second.Get();
		if (!renderObject->IsHidden())
		{
			Vector3 center;
			double radius = 0.0;
			if (renderObject->GetWorldBoundingSphere(center, radius))
			{
				AxisAlignedBoundingBox box;
				box.SetFromSphere(center, radius);
				debugLines->AddBox(box, Vector3(0.0, 0.0, 1.0));
			}
		}
	}
}

void Scene::Render(Camera* camera, RenderPass renderPass)
{
	std::vector<RenderObject*> visibleObjects;

	for (auto pair : this->renderObjectMap)
	{
		RenderObject* renderObject = pair.second.Get();
		if (renderObject->IsHidden())
			continue;

		if (!camera->IsApproximatelyVisible(renderObject))
			continue;
			
		visibleObjects.push_back(renderObject);
	}

	std::sort(visibleObjects.begin(), visibleObjects.end(), [](const RenderObject* objectA, const RenderObject* objectB) -> bool {
		return objectA->SortKey() < objectB->SortKey();
	});

	for (RenderObject* renderObject : visibleObjects)
		renderObject->Render(camera, renderPass);
}

void Scene::PrepareRenderObjects()
{
	for (auto pair : this->renderObjectMap)
	{
		RenderObject* renderObject = pair.second.Get();
		renderObject->Prepare();
	}
}

void Scene::PreRender()
{
	for (auto pair : this->renderObjectMap)
	{
		RenderObject* renderObject = pair.second.Get();
		renderObject->PreRender();
	}
}

//--------------------------- RenderObject ---------------------------

RenderObject::RenderObject()
{
	this->hide = false;
	this->name = std::format("{:#010x}", uintptr_t(this));
}

/*virtual*/ RenderObject::~RenderObject()
{
}

/*virtual*/ int RenderObject::SortKey() const
{
	return 0;
}

/*virtual*/ void RenderObject::Prepare()
{
}

/*virtual*/ void RenderObject::PreRender()
{
}

/*virtual*/ void RenderObject::OnPostAdded()
{
}

/*virtual*/ void RenderObject::OnPreRemoved()
{
}

/*virtual*/ bool RenderObject::GetWorldBoundingSphere(Vector3& center, double& radius) const
{
	return false;
}