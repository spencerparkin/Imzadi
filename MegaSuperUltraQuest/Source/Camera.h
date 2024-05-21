#pragma once

#include "Reference.h"
#include "Math/Transform.h"
#include "Math/Frustum.h"

class RenderObject;

/**
 * An instance of this class describes how we are viewing a scene.
 */
class Camera : public ReferenceCounted
{
public:
	Camera();
	virtual ~Camera();

	/**
	 * False is returned here if the given render object is definitely not visible.
	 * True is returned here if the given render object is likely visible.
	 */
	bool IsApproximatelyVisible(const RenderObject* renderObject) const;

	/**
	 * Specify the position and orientation of this camera.  Remember that camera
	 * space is thought of as being at origin looking down -Z with +X right and +Y up.
	 */
	void SetCameraToWorldTransform(const Collision::Transform& cameraToWorld);

	/**
	 * Get read-only access to this camera's camera-space to world-space transform.
	 */
	const Collision::Transform& GetCameraToWorldTransform() const { return this->cameraToWorld; }

	/**
	 * Get our cached world-to-camera transform, updating it if necessary.
	 */
	const Collision::Transform& GetWorldToCameraTransform() const;

	const Collision::Frustum& GetFrustum() const { return this->frustum; }

	void SetFrustum(const Collision::Frustum& frustum) { this->frustum = frustum; }

private:

	Collision::Frustum frustum;
	Collision::Transform cameraToWorld;
	mutable Collision::Transform worldToCamera;
	mutable bool worldToCameraValid;
};