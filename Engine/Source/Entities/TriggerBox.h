#pragma once

#include "Entity.h"
#include "Assets/TriggerBoxData.h"

namespace Imzadi
{
	/**
	 * These are collision shapes only (and so do not render anything) but they
	 * do trigger events when some other entity enters or leaves the shape's volume.
	 */
	class IMZADI_API TriggerBox : public Entity
	{
	public:
		TriggerBox();
		virtual ~TriggerBox();

		virtual bool Setup() override;
		virtual bool Shutdown() override;
		virtual bool Tick(TickPass tickPass, double deltaTime) override;

		void SetData(TriggerBoxData* data) { this->data.Set(data); }

	protected:
		ShapeID collisionShapeID;
		TaskID collisionQueryTaskID;
		Reference<TriggerBoxData> data;
	};
}