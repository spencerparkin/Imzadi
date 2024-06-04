#include "Entity.h"
#include "Math/Vector3.h"
#include "Math/Quaternion.h"
#include "Math/Matrix3x3.h"
#include "Scene.h"
#include "RenderObjects/RenderMeshInstance.h"
#include "Shape.h"

/**
 * An instance of this class is the protagonist of our game saga.
 */
class Hero : public Entity
{
public:
	Hero();
	virtual ~Hero();

	virtual bool Setup() override;
	virtual bool Shutdown(bool gameShuttingDown) override;
	virtual bool Tick(double deltaTime) override;
	virtual bool GetTransform(Collision::Transform& transform) override;

	void SetRestartLocation(const Collision::Vector3& restartLocation) { this->restartLocation = restartLocation; }
	void SetRestartOrientation(const Collision::Quaternion& restartOrientation) { this->restartOrientation = restartOrientation; }

private:
	Collision::Vector3 restartLocation;
	Collision::Quaternion restartOrientation;
	Collision::ShapeID shapeID;
	Reference<RenderMeshInstance> renderMesh;
	uint32_t cameraHandle;
	double maxMoveSpeed;
};