#pragma once

#include <list>
#include <memory>

class RenderMesh;
class Camera;

/**
 * This class represents the entire renderable scene and how we're viewing it.
 * It is a collection of render meshes and can be asked to update and draw each frame.
 */
class Scene
{
public:
	Scene();
	virtual ~Scene();

	/**
	 * Remove all RenderMesh instances being tracked by this scene.  Once cleared,
	 * nothing will render when the scene is rendered.
	 */
	void Clear();

	/**
	 * Add a RenderMesh instance to the scene.  It will get drawn if it intersects the view frustum.
	 */
	void AddRenderMesh(std::shared_ptr<RenderMesh> renderMesh);

	/**
	 * Submit draw-calls for everything approximately deemed visible in the scene.
	 */
	void Render();

	/**
	 * Assign the given camera to the scene.  This will dictate
	 * how the scene is viewed and therefore rendered.
	 */
	void SetCamera(std::shared_ptr<Camera> camera) { this->camera = camera; }

	/**
	 * Get access to the camera representing our view into the scene.
	 */
	Camera* GetCamera() { return this->camera.get(); }

private:
	typedef std::list<std::shared_ptr<RenderMesh>> RenderMeshList;

	// Note that a more sophisticated system would spacially sort scene objects
	// or put them in some sort of hierarchy or something like that.  I'm just
	// going to do something super simple here and just have a list of render
	// objects.  That's it.  I'll cull them based on the view frustum, but
	// that's as far as I'm going to take visible surface determination in
	// this simple application.
	RenderMeshList renderMeshList;

	// Before a render mesh can draw, we must calculate its full object-space
	// to projection-space transform, which is why we need to associate a
	// camera with the scene.
	std::shared_ptr<Camera> camera;
};