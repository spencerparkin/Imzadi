#pragma once

#include "Scene.h"
#include "Assets/Font.h"
#include "Math/Transform.h"
#include <d3d12.h>

namespace Imzadi
{
	class IMZADI_API TextRenderObject : public RenderObject
	{
	public:
		TextRenderObject();
		virtual ~TextRenderObject();

		virtual void Render(Camera* camera, RenderPass renderPass) override;
		virtual void GetWorldBoundingSphere(Imzadi::Vector3& center, double& radius) const override;
		virtual int SortKey() const override;
		virtual void Prepare() override;

		enum Flag
		{
			REBUILD_VERTEX_BUFFER		= 0x00000001,		///< This indicates that the vertex buffer for the text needs to be regenerated.  It is used internally and never needs to be set by the user.
			ALWAYS_FACING_CAMERA		= 0x00000002,		///< This indicates that we need to bill-board the text quads so that they face the camera.  Ignored if sticking with camera.
			CONSTANT_SIZE				= 0x00000004,		///< This indicates that the text should be the same size no matter how far away or close it is to the camera.  Ignored if sticking with camera.
			ALWAYS_ON_TOP				= 0x00000008,		///< This indicates that nothing should draw over the top of the text.
			STICK_WITH_CAMERA			= 0x00000010,		///< If used, our transform goes from object space to camera space, not the typical object space to world space.
			STICK_WITH_CAMERA_PROJ		= 0x00000020,		///< This overrides the STICK_WITH_CAMERA flag, and, if used, our transform goes from object space to projection space.
			LEFT_JUSTIFY				= 0x00000040,		///< Left-justify the text in object space when generating the vertex buffer.  Should not be set if any other justification flag is set.
			RIGHT_JUSTIFY				= 0x00000080,		///< Right-justify the text in object space when generating the vertex buffer.  Should not be set if any other justification flag is set.
			CENTER_JUSTIFY				= 0x00000100		///< Center-justify the text in object space when generating the vertex buffer.  Should not be set if any other justification flag is set.
		};

		/**
		 * Use this method to configure how you want the text to be rendered, apart from
		 * the front and what text is actually displayed, of course.
		 * 
		 * @param[in] givenFlags See the @ref Flag enum for possible values here.
		 */
		void SetFlags(uint32_t givenFlags);

		/**
		 * Return the flags currently being used to render the text.
		 */
		uint32_t GetFlags() const;

		/**
		 * Set the text that should be rendered by this render object.  Note that
		 * you can confidently call this function every frame, even if your string
		 * doesn't change very much.  The text geometry isn't recomputed unless
		 * the text changes here.
		 * 
		 * @param[in] givenText Render this text according to how this render object is configured.
		 */
		void SetText(const std::string& givenText);

		/**
		 * Return the string being rendered by this render object.
		 */
		const std::string& GetText() const;

		/**
		 * Set the color used to render the text.
		 */
		void SetColor(const Vector3& color);

		/**
		 * Get the color used to render the text.
		 */
		const Vector3& GetColor() const;

		/**
		 * Try to load a font by the given name and then cache it on this text render
		 * object for use while rendering.  This could cause a hiccup during rendering
		 * unless all possibly-needed fonts are simply pre-cached, which I'll probably do.
		 * 
		 * @param[in] fontName A font by this name is searched for in the engine's font folder.
		 * @return True is returned if the font was successfully found and loaded; false, otherwise.
		 */
		bool SetFont(const std::string& fontName);

		/**
		 * Return the font being used to render this text object.
		 */
		Font* GetFont();

		/**
		 * Set the transform for the text.  This takes it from object
		 * space to world space, or object space to camera space, or
		 * even object space to projection space, depending on how the
		 * render object is configured.  See the TextRenderObject::Flag enum.
		 */
		void SetTransform(const Transform& transform);

		/**
		 * Get the transform being used to render this text object.
		 */
		const Transform& GetTransform() const;

	private:
		double CalculateStringWidth();

		Reference<Font> font;
#if 0
		ID3D11Buffer* vertexBuffer;
#endif
		std::string text;
		Vector3 color;
		uint32_t flags;
		Transform objectToTargetSpace;
	};

	/**
	 * This text render object is used to display the engine's current frame-rate.
	 */
	class FPSRenderObject : public TextRenderObject
	{
	public:
		FPSRenderObject();
		virtual ~FPSRenderObject();

		virtual void Prepare() override;

	private:
		std::list<double> deltaTimeList;
		uint32_t deltaTimeListMaxSize;
	};
}