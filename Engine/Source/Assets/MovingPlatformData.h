#pragma once

#include "AssetCache.h"
#include "Math/Vector3.h"

namespace Imzadi
{
	/**
	 * This contains all the data needed to determine how a moving platform moves.
	 */
	class IMZADI_API MovingPlatformData : public Asset
	{
		// TODO: Can we also animate the orientation of the platforms?
	public:
		MovingPlatformData();
		virtual ~MovingPlatformData();

		virtual bool Load(const rapidjson::Document& jsonDoc, std::string& error, AssetCache* assetCache) override;
		virtual bool Unload() override;

		enum SplineType
		{
			LERP,
			SMOOTH
		};

		enum SplineMode
		{
			BOUNCE,
			CYCLE
		};

		void SetSplineType(SplineType type) { this->splineType = type; }
		SplineType GetSplineType() const { return this->splineType; }

		void SetSplineMode(SplineMode mode) { this->splineMode = mode; }
		SplineMode GetSplineMode() const { return this->splineMode; }

		const std::vector<Vector3>& GetSplineDeltaArray() const { return *this->splineDeltas; }
		const std::string& GetMeshFile() const { return this->meshFile; }
		const std::string& GetCollisionFile() const { return this->collisionFile; }
		double GetMoveSpeed() const { return this->moveSpeed; }

	private:

		std::vector<Vector3>* splineDeltas;
		SplineType splineType;
		SplineMode splineMode;
		std::string meshFile;
		std::string collisionFile;
		double moveSpeed;
	};
}