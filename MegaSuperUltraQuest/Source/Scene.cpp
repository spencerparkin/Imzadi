#include "Scene.h"
#include "Camera.h"
#include "RenderMesh.h"

Scene::Scene()
{
}

/*virtual*/ Scene::~Scene()
{
	this->Clear();
}

void Scene::Clear()
{
	this->renderMeshList.clear();
}

void Scene::AddRenderMesh(std::shared_ptr<RenderMesh> renderMesh)
{
	this->renderMeshList.push_back(renderMesh);
}

void Scene::Render()
{
	if (!this->camera)
		return;

	for (const std::shared_ptr<RenderMesh>& renderMesh : this->renderMeshList)
	{
		if (!this->camera->IsApproximatelyVisible(renderMesh.get()))
			continue;
			
		renderMesh->Render(this);
	}
}