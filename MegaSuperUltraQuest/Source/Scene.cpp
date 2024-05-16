#include "Scene.h"
#include "Camera.h"
#include "RenderMesh.h"

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

void Scene::Render()
{
	if (!this->camera)
		return;

	for (/*const*/ Reference<RenderObject>& renderObject : this->renderObjectList)
	{
		if (!this->camera->IsApproximatelyVisible(renderObject.Get()))
			continue;
			
		renderObject->Render(this);
	}
}

//--------------------------- RenderObject ---------------------------

RenderObject::RenderObject()
{
}

/*virtual*/ RenderObject::~RenderObject()
{
}