#pragma once

#include "Entities/MovingPlatform.h"
#include "EventSystem.h"
#include "Math/Plane.h"

class RubiksCubieEvent;

/**
 * These are moving platforms that simply move according to the
 * rules and constraints of a Rubik's Cube.  An individual cubie
 * knows when it is in the solved position and orientation by
 * comparing its current cubie-space to puzzle-space transform
 * with an initial transform of such.  Some numerical drift can
 * be compensated for by assuming each rotation of a face keeps
 * everything axis-aligned.  A cubie responds to messages from
 * the message system to know when, where and how to move.  For
 * rendering, the final object-to-world transform for each cubie
 * accounts for an orientation matrix that can be used to orient
 * the entire puzzle for examination purposes.  This, combined
 * with the current camera matrix, should be used to determine what
 * it means to rotate the right, left, and top faces, etc.  In other
 * words, the meaning of these terms is orientation-dependent.
 * 
 * During the solve, collision will be ignored, but of course, one
 * of the points of being a moving platform is that I do intend to
 * let the player walk on the cubies at some point.  The idea is that
 * once solved, the cubies disperse into a pattern forming a path to
 * the center of the cube where the treasure lay.
 */
class RubiksCubie : public Imzadi::MovingPlatform
{
public:
	RubiksCubie();
	virtual ~RubiksCubie();

	virtual bool Setup() override;
	virtual bool Shutdown() override;
	virtual bool Tick(Imzadi::TickPass tickPass, double deltaTime) override;

	uint32_t GetMasterHandle() const { return this->masterHandle; }
	bool IsSolved() const;

private:
	void HandleCubieEvent(const Imzadi::Event* event);
	void CompleteAnimationNow();

	enum Disposition
	{
		FOLLOW_MASTER,
		MOVE_TO_PLATFORMING_POSITION
	};

	uint32_t masterHandle;
	Imzadi::EventListenerHandle eventHandle;
	Imzadi::Transform solvedCubieToPuzzle;
	Imzadi::Transform currentCubieToPuzzle;
	Imzadi::Transform platformObjectToWorld;
	Imzadi::Vector3 animationAxis;
	double animationCurrentAngle;
	double animationTargetAngle;
	double animationRate;
	bool animating;
	Disposition disposition;
};

/**
 * The cubies respond to this event when asked to move and rotate.
 */
class RubiksCubieEvent : public Imzadi::Event
{
public:
	enum Rotation
	{
		CW,
		CCW
	};

	RubiksCubieEvent();
	RubiksCubieEvent(const Imzadi::Plane& cutPlane, Rotation rotation, bool animate);
	virtual ~RubiksCubieEvent();

public:
	Imzadi::Plane cutPlane;		///< This is expected to be in puzzle space.
	Rotation rotation;
	bool animate;
};