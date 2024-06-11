#pragma once

#include "Reference.h"
#include "Math/Transform.h"
#include "Math/Frustum.h"

namespace Imzadi
{
	class RenderObject;

	/**
	 * An instance of this class describes how we are viewing a scene.
	 */
	class IMZADI_API Camera : public ReferenceCounted
	{
	public:
		Camera();
		virtual ~Camera();

		enum ViewMode
		{
			PERSPECTIVE,
			ORTHOGRAPHIC
		};

		struct OrthographicParams
		{
			double width;					///< This indicates the range [-width/2, width/2] along the X-axis.
			double height;					///< This indicates the range [-height/2, height/2] along the Y-axis.
			double nearClip;				///< This is a positive distance along the -Z-axis to the near clipping plane.
			double farClip;					///< This is a positive distance along the -Z-axis to the far clipping plane.
			double desiredAspectRatio;		///< If zero, this is ignored.  If not, width and height are adjusted (but not modified) just before the projection matrix is calculated.
			bool adjustWidth;				///< When adjusting for aspect ratio, adjust the width to match the desired aspect ratio; the height, otherwise.
		};

		/**
		 * False is returned here if the given render object is definitely not visible.
		 * True is returned here if the given render object is likely visible.
		 */
		bool IsApproximatelyVisible(const RenderObject* renderObject) const;

		/**
		 * Specify the position and orientation of this camera.  Remember that camera
		 * space is thought of as being at origin looking down -Z with +X right and +Y up.
		 */
		void SetCameraToWorldTransform(const Transform& cameraToWorld);

		/**
		 * Get read-only access to this camera's camera-space to world-space transform.
		 */
		const Transform& GetCameraToWorldTransform() const { return this->cameraToWorld; }

		/**
		 * Get our cached world-to-camera transform, updating it if necessary.
		 */
		const Transform& GetWorldToCameraTransform() const;

		/**
		 * Build a camera-to-world transform for this camera instance based upon the given parameters.
		 *
		 * @param[in] eyePoint This is where you want the camera placed.
		 * @param[in] focalPoint This is what you want the camera to look at.
		 * @param[in] upVector This is used to know which way is up for the viewer.
		 * @return True is returned if and only if the resulting orientation matrix is non-singular.
		 */
		bool LookAt(const Vector3& eyePoint, const Vector3& focalPoint, const Vector3& upVector);

		/**
		 * Calculate and return the projection matrix for this camera.
		 * This will be a perspective projection matrix if the view-mode is PERSPECTIVE.
		 * This will be an orthographic projection matrix if the view-mode is ORTHOGRAPHIC.
		 *
		 * @param[out] matrix The projection matrix is returned in this parameter.
		 */
		void GetProjectionMatrix(Matrix4x4& matrix) const;

		/**
		 * Get the frustum used by this camera.
		 */
		const Frustum& GetFrustum() const { return this->frustum; }

		/**
		 * Set the frustum used by this camera.
		 */
		void SetFrustum(const Frustum& frustum) { this->frustum = frustum; }

		ViewMode GetViewMode() const { return this->viewMode; }
		void SetViewMode(ViewMode viewMode) { this->viewMode = viewMode; }

		const OrthographicParams& GetOrthographicParameters() const { return this->orthoParams; }
		void SetOrthographicParams(const OrthographicParams& orthoParams) { this->orthoParams = orthoParams; }

		const Vector3& GetEyePoint() const { return this->cameraToWorld.translation; }

	private:
		ViewMode viewMode;
		OrthographicParams orthoParams;
		Frustum frustum;
		Transform cameraToWorld;
		mutable Transform worldToCamera;
		mutable bool worldToCameraValid;
	};
}