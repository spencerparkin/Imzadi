#include "Scene.h"
#include "Camera.h"
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

std::string Scene::AddRenderObject(Reference<RenderObject> renderObject)
{
	std::string name = std::format("{:#010x}", uintptr_t(renderObject.Get()));
	if (!this->AddRenderObject(name, renderObject))
		return "";

	return name;
}

bool Scene::AddRenderObject(const std::string& name, Reference<RenderObject> renderObject)
{
	if (this->renderObjectMap.find(name) != this->renderObjectMap.end())
		return false;

	this->renderObjectMap.insert(std::pair<std::string, Reference<RenderObject>>(name, renderObject));
	return true;
}

bool Scene::RemoveRenderObject(const std::string& name, Reference<RenderObject>* renderObject /*= nullptr*/)
{
	RenderObjectMap::iterator iter = this->renderObjectMap.find(name);
	if (iter == this->renderObjectMap.end())
		return false;

	if (renderObject)
		renderObject->Set(iter->second);

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

void Scene::Render(Camera* camera, RenderPass renderPass)
{
	std::vector<RenderObject*> visibleObjects;

	for (auto pair : this->renderObjectMap)
	{
		RenderObject* renderObject = pair.second.Get();
		if (renderObject->IsHidden())
			continue;

		// TODO: Activate this code when ready.
		//if (!camera->IsApproximatelyVisible(renderObject))
		//	continue;
			
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

//--------------------------- RenderObject ---------------------------

RenderObject::RenderObject()
{
	this->hide = false;
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