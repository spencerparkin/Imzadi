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

bool Scene::AddRenderMesh(std::shared_ptr<Asset> renderMesh)
{
	if (!dynamic_cast<RenderMesh*>(renderMesh.get()))
		return false;

	this->renderMeshList.push_back(renderMesh);
}

void Scene::Render()
{
	if (!this->camera)
		return;

	for (const std::shared_ptr<Asset>& asset : this->renderMeshList)
	{
		auto renderMesh = static_cast<RenderMesh*>(asset.get());
		if (!this->camera->IsApproximatelyVisible(renderMesh))
			continue;
			
		renderMesh->Render(this);
	}
}