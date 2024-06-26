#pragma once

#include "Scene.h"
#include "Assets/SkyDome.h"

namespace Imzadi
{
	/**
	 * These can render a sky-dome.  They're not lit and they don't cast a shadow.
	 * They just surround everything.  The cube-texture used here can also be used
	 * for environment reflection.
	 */
	class IMZADI_API SkyDomeRenderObject : public RenderObject
	{
	public:
		SkyDomeRenderObject();
		virtual ~SkyDomeRenderObject();

		virtual void Render(Camera* camera, RenderPass renderPass) override;
		virtual void GetWorldBoundingSphere(Vector3& center, double& radius) const override;

		void SetSkyDome(SkyDome* skyDome) { this->skyDome.Set(skyDome); }
		SkyDome* GetSkyDome() { return this->skyDome.Get(); }

	private:
		Reference<SkyDome> skyDome;
	};
}