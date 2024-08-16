#pragma once

#include "RenderMeshInstance.h"
#include "Assets/Animation.h"
#include "Assets/SkinnedRenderMesh.h"

namespace Imzadi
{
	class IMZADI_API AnimatedMeshInstance : public RenderMeshInstance
	{
	public:
		AnimatedMeshInstance();
		virtual ~AnimatedMeshInstance();

		virtual void Render(Camera* camera, RenderPass renderPass) override;

		void SetTransitionTime(double transitionTime) { this->transitionTime = transitionTime; }
		double GetTransitionTime() const { return this->transitionTime; }

		bool AdvanceAnimation(double deltaTime, bool canLoop);
		bool SetAnimation(const std::string& animationName);
		Animation* GetAnimation() { return this->animation.Get(); }
		void ClearTransition();

		/**
		 * Alternative to @ref AdvanceAnimation, just orient the skeleton and deform the
		 * mesh according to the given location value.
		 * 
		 * @param[in] alpha This should be a valud in the range [0,1].
		 * @return True is returned on success; false, otherwise.
		 */
		bool SetAnimationLocation(double alpha);

		/**
		 * Alternative to @ref AdvanceAnimation, just orient the skeleton and deform the
		 * mesh according to the given index.  The purpose of this routine is to check
		 * the mesh and skeleton while taking any interpolation calculations out of
		 * consideration.
		 * 
		 * @param[in] i This specifies which key-frame to use.
		 * @return True is returned on success; false, otherwise.
		 */
		bool SetAnimationLocation(int i);

		void SetSkinnedMesh(SkinnedRenderMesh* skinnedMesh) { this->skinnedMesh.Set(skinnedMesh); }
		SkinnedRenderMesh* GetSkinnedMesh() { return this->skinnedMesh.Get(); }

		static void SetRenderSkeletons(bool render);
		static bool GetRenderSkeletons();

	private:
		static bool renderSkeletons;

		double transitionTime;
		double currentTransitionTime;
		KeyFrame transitionalKeyFrame;
		KeyFrame currentKeyFrame;
		Reference<Animation> animation;
		Animation::Cursor cursor;
		Reference<SkinnedRenderMesh> skinnedMesh;
	};
}