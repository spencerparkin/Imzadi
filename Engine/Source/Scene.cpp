#include "Scene.h"
#include "Camera.h"
#include <algorithm>

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
	this->renderObjectList.clear();
}

void Scene::AddRenderObject(Reference<RenderObject> renderObject)
{
	this->renderObjectList.push_back(renderObject);
}

void Scene::Render(Camera* camera, RenderPass renderPass)
{
	std::vector<RenderObject*> visibleObjects;

	for (/*const*/ Reference<RenderObject>& renderObject : this->renderObjectList)
	{
		if (renderObject->IsHidden())
			continue;

		// TODO: Activate this code when ready.
		//if (!camera->IsApproximatelyVisible(renderObject.Get()))
		//	continue;
			
		visibleObjects.push_back(renderObject.Get());
	}

	std::sort(visibleObjects.begin(), visibleObjects.end(), [](const RenderObject* objectA, const RenderObject* objectB) -> bool {
		return objectA->SortKey() < objectB->SortKey();
	});

	for (RenderObject* renderObject : visibleObjects)
		renderObject->Render(camera, renderPass);
}

void Scene::PrepareRenderObjects()
{
	for (Reference<RenderObject>& renderObject : this->renderObjectList)
		renderObject->Prepare();
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