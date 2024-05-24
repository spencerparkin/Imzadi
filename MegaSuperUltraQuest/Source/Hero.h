#include "Entity.h"
#include "Math/Vector3.h"
#include "Scene.h"
#include "RenderMesh.h"

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
	virtual void Tick(double deltaTime) override;
	virtual bool GetTransform(Collision::Transform& transform) override;

	void SetRestartLocation(const Collision::Vector3& restartLocation) { this->restartLocation = restartLocation; }

private:
	Collision::Vector3 restartLocation;
	Reference<RenderMeshInstance> renderMesh;
};