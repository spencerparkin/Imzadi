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

	/**
	 * Build a camera-to-world transform for this camera instance based upon the given parameters.
	 * 
	 * @param[in] eyePoint This is where you want the camera placed.
	 * @param[in] focalPoint This is what you want the camera to look at.
	 * @param[in] upVector This is used to know which way is up for the viewer.
	 * @return True is returned if and only if the resulting orientation matrix is non-singular.
	 */
	bool LookAt(const Collision::Vector3& eyePoint, const Collision::Vector3& focalPoint, const Collision::Vector3& upVector);

	/**
	 * Get the frustum used by this camera.
	 */
	const Collision::Frustum& GetFrustum() const { return this->frustum; }

	/**
	 * Set the frustum used by this camera.
	 */
	void SetFrustum(const Collision::Frustum& frustum) { this->frustum = frustum; }

private:

	Collision::Frustum frustum;
	Collision::Transform cameraToWorld;
	mutable Collision::Transform worldToCamera;
	mutable bool worldToCameraValid;
};