#pragma once

#include "Entity.h"

/**
 * This class listens for the inputs that can be used to manipulate the
 * twisty puzzle and also sense messages to which the cubies can respond.
 */
class RubiksCubeMaster : public Imzadi::Entity
{
public:
	RubiksCubeMaster();
	virtual ~RubiksCubeMaster();

	virtual bool Setup() override;
	virtual bool Shutdown() override;
	virtual bool Tick(Imzadi::TickPass tickPass, double deltaTime) override;

	void Enable(bool enable);

	void SetPuzzleChannelName(const std::string& name) { this->puzzleChannelName = name; }
	const Imzadi::Transform& GetPuzzleToWorldTransform() const { return this->puzzleToWorld; }

private:
	enum State
	{
		OFF,
		POWERING_UP,
		OPERATING,
		POWERING_DOWN
	};

	State state;
	Imzadi::Transform puzzleToWorld;
	std::string puzzleChannelName;
	Imzadi::Transform originalCameraTransform;
	Imzadi::Transform sourceCameraTransform;
	Imzadi::Transform targetCameraTransform;
	double cameraTransitionRate;
	double cameraTransitionAlpha;
};