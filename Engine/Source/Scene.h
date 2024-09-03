#pragma once

#include "Reference.h"
#include "Math/AxisAlignedBoundingBox.h"
#include <list>
#include <unordered_map>

namespace Imzadi
{
	class RenderObject;
	class Camera;

	enum RenderPass
	{
		MAIN_PASS,
		SHADOW_PASS
	};

	/**
	 * This class represents the entire renderable scene and how we're viewing it.
	 * It is a collection of RenderObject instances and can be asked to draw each frame.
	 * 
	 * The scene here doesn't organize render objects into a hierarchy.  Maybe it should,
	 * but for now it doesn't.  There's also no spacial sorting going on here for the
	 * expidition of visible surface determination.  For now, I'm just doing a frustum
	 * check for each render object.
	 */
	class IMZADI_API Scene : public ReferenceCounted
	{
	public:
		Scene();
		virtual ~Scene();

		/**
		 * Remove all RenderObject instances being tracked by this scene.  Once cleared,
		 * nothing will render when the scene is rendered.
		 */
		void Clear();

		/**
		 * Add a RenderObject instance to the scene.  It will get drawn if it intersects the view frustum.
		 * 
		 * @param[in] renderObject This is the render object to add to the scene.
		 * @param[in] adjustNameIfNecessary If true (the default), a number is tacked on to the name of the given object in order to avoid a name collision, if necessary.
		 * @return True is returned on success; false, otherwise.
		 */
		bool AddRenderObject(Reference<RenderObject> renderObject, bool adjustNameIfNecessary = true);

		/**
		 * Remove a RenderObject instance from the scene by name.
		 * 
		 * @param[in] name The render object by this name will be removed.
		 * @param[out] renderObject The removed render object is returned here, if given as non-null.
		 * @return True is returned on success; false, otherwise.
		 */
		bool RemoveRenderObject(const std::string& name, Reference<RenderObject>* renderObject = nullptr);

		/**
		 * Find a render object in the scene by the given name.
		 * 
		 * @param[in] name Look for a render object stored under this name.
		 * @param[out] renderObject The found render object, if any, is returned in this reference.
		 * @return True is returned on success; false, otherwise.
		 */
		bool FindRenderObject(const std::string& name, Reference<RenderObject>& renderObject);

		/**
		 * This is a convenience wrapper around the non-templated @ref FindRenderObject method.
		 */
		template<typename T>
		bool FindRenderObject(const std::string& name, Reference<T>& renderObjectTyped)
		{
			Reference<RenderObject> renderObject;
			if (!this->FindRenderObject(name, renderObject))
				return false;

			renderObjectTyped.SafeSet(renderObject.Get());
			return renderObjectTyped.Get() != nullptr;
		}

		/**
		 * Submit draw-calls for everything approximately deemed visible in the scene
		 * to the given camera.
		 *
		 * @param[in] camera This is the camera to use to render the scene.
		 * @param[in] renderPass Indicate the purpose of this render.
		 */
		void Render(Camera* camera, RenderPass renderPass);

		/**
		 * Give all render objects a chance to prepare for an up-coming render.
		 * This called by the engine at a good time when other threads may be
		 * working hard on calculations we'll need before rendering anyway.
		 */
		void PrepareRenderObjects();

		/**
		 * This is called just before all render passes are preformed.
		 */
		void PreRender();

	private:
		typedef std::unordered_map<std::string, Reference<RenderObject>> RenderObjectMap;
		RenderObjectMap renderObjectMap;
	};

	/**
	 * This is the base class for anything that can get rendered in the scene.
	 */
	class IMZADI_API RenderObject : public ReferenceCounted
	{
	public:
		RenderObject();
		virtual ~RenderObject();

		/**
		 * This must be overridden to do the actual rendering.  That means
		 * making draw-calls.
		 * 
		 * @param[in] camera This is the camera that is being used to render.  You can get the world to camera or world to projection matrix from this.
		 * @param[in] renderPass This tells you what render pass we're doing.  If you don't render in, say, the shadow pass, (maybe you don't cast a shadow), then you can use this to bail out.
		 */
		virtual void Render(Camera* camera, RenderPass renderPass) = 0;

		/**
		 * This is used to perform a frustum culling check.  If this render
		 * object can't be seen by a frustum, then it is not rendered in
		 * the associated view.
		 */
		virtual void GetWorldBoundingSphere(Imzadi::Vector3& center, double& radius) const = 0;

		/**
		 * This can be optionally overridden to do any calculations necessary
		 * before rendering is actually performed.  Note that there are at least
		 * two rendering passes: shadow and main.  So rather than redo calculations
		 * in each pass, you could do them once here.
		 */
		virtual void Prepare();

		/**
		 * This is called before all render passes are performed.
		 */
		virtual void PreRender();

		/**
		 * This is called just after this object has been added to the scene.
		 */
		virtual void OnPostAdded();

		/**
		 * This is called just before the object is removed from the scene.
		 */
		virtual void OnPreRemoved();

		/**
		 * This gives us some control over the order in which render objects render.
		 * Rather than have a separate pass for opaque and translucent objects, alpha
		 * blending is enabled in the main pass, and this key can be used to make sure
		 * that opaque things are drawn before anything that has transparency.
		 */
		virtual int SortKey() const;

		bool IsHidden() const { return this->hide; }
		void SetHidden(bool hide) { this->hide = hide; }

		void SetName(const std::string& name) { this->name = name; }
		const std::string& GetName() const { return this->name; }

	private:
		bool hide;
		std::string name;
	};
}