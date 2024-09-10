#pragma once

#include "Assets/MovingPlatformData.h"

/**
 * This describes how a cubie is positioned and oriented in the solved
 * state relative to the whole puzzle and other cubies.  We're also a
 * platform here, because once solved, the cubies can be platformed.
 * 
 * Note that although this class is called "cubie", it can handle any
 * twisty puzzle piece for any kind of twisty puzzle with planar cuts.
 */
class RubiksCubieData : public Imzadi::MovingPlatformData
{
public:
	RubiksCubieData();
	virtual ~RubiksCubieData();

	virtual bool Load(const rapidjson::Document& jsonDoc, Imzadi::AssetCache* assetCache) override;
	virtual bool Unload() override;

	const Imzadi::Transform& GetCubieToPuzzleTransform() const { return this->cubieToPuzzle; }
	const std::string& GetMasterName() const { return this->masterName; }
	const std::string& GetPuzzleChannelName() const { return this->puzzleChannelName; }

private:
	Imzadi::Transform cubieToPuzzle;	///< This is a cubie-space to puzzle-space transform in the solved state.  The overall puzzle will have a puzzle-space to world-space transform.
	std::string masterName;				///< What master do we use?  The master helps orchestrate all the cubies.
	std::string puzzleChannelName;		///< On what channel do the cubies listen?  The master communicates to the cubies on this channel.
};