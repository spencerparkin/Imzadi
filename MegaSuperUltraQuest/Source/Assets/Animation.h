#pragma once

#include "AssetCache.h"
#include "Skeleton.h"

class KeyFrame;

/**
 * An animation is a sequence of key-frames.  Each key-frame specifies a moment in
 * time along with a set of bone orientations.  Each of these points to a named bone.
 * An animation is used to drive a skeleton (which, in turn, drives a mesh.)
 */
class Animation : public Asset
{
public:
	Animation();
	virtual ~Animation();

	struct Cursor
	{
		int i;
		double timeSeconds;
		bool loop;
	};

	virtual bool Load(const rapidjson::Document& jsonDoc, AssetCache* assetCache) override;
	virtual bool Unload() override;
	virtual bool Save(rapidjson::Document& jsonDoc) const override;

	void SetName(const std::string& name) { this->name = name; }
	const std::string& GetName() const { return this->name; }

	/**
	 * Delete all key-frames and empty our list of such.
	 */
	void Clear();

	/**
	 * Append a key-frame to our list of such.  Note that we do nothing to ensure
	 * here that the key-frames are in chronological order.  Once all keys are added,
	 * you can call the TimeSort method if you believe it necessary.
	 * 
	 * Note that we take ownership of the given allocation, which we expect to be
	 * an allocation on the heap (not the stack.)
	 */
	void AddKeyFrame(KeyFrame* keyFrame);

	/**
	 * Make sure all key-frames listed in this animation are in chronological order.
	 * This is necessary for most algorithms to work properly in this class.
	 */
	void TimeSort();

	/**
	 * Use a binary search to quickly find the adjacent pair of key-frames that are
	 * bounding (as tightly as possible) the given time index.  The caller can then
	 * pose a skeleton by interpolating between these two key-frames.
	 * 
	 * Note that this method assumes the list of key frames in this animation are sorted.
	 * If this is not the case, we leave our result as undefined.
	 * 
	 * @param[out] keyFrameA This is the lower-bound key-frame.
	 * @param[out] keyFrameB This is the upper-bound key-frame.
	 * @param[in] timeSeconds This is the time (in seconds) along this animation's time-line of where to search.
	 * @return False is returned if the given time-index is out of bounds for the entire animation; true, otherwise.
	 */
	bool FindKeyFramesFromTime(const KeyFrame*& keyFrameA, const KeyFrame*& keyFrameB, double timeSeconds) const;

	/**
	 * Use a cursor to quickly find the adjacent pair of key-frames associated
	 * with the given cursor.  The caller can then pose a skeleton by interpolating
	 * between these two key-frames.
	 * 
	 * Note that here again we assume that the list of key-frames in this animation
	 * is sorted.  If not so, we are undefined.
	 * 
	 * @param[in] cursor This is a structure the indicates where in the animation a player currently is at.
	 * @param[out] keyFrameA This is the lower-bound key-frame.
	 * @param[out] keyFrameB This is the upper-bound key-frame.
	 * @return True is returned on success; false, otherwise.
	 */
	bool GetKeyFramesFromCursor(const Cursor& cursor, const KeyFrame*& keyFrameA, const KeyFrame*& keyFrameB) const;

	/**
	 * Modify the given cursor to advance it forward or backward in time according to the given time delta.
	 * 
	 * @param[in,out] cursor This is the cursor to advance.
	 * @param[in] deltaTimeSeconds This is the amount of time (in seconds) to advance.
	 * @return True is returned on success; false, otherwise.
	 */
	bool AdvanceCursor(Cursor& cursor, double deltaTimeSeconds) const;

private:
	
	std::string name;
	std::vector<KeyFrame*> keyFrameArray;
};

/**
 * This class contains information about how to pose all or a subset of any skeleton's bones
 * at a certain point in time.
 */
class KeyFrame
{
public:
	KeyFrame();
	virtual ~KeyFrame();

	double GetTime() const { return this->timeSeconds; }
	void SetTime(double timeSeconds) { this->timeSeconds = timeSeconds; }

	void Clear();

	void Sort();

	bool Load(const rapidjson::Value& keyFrameValue);
	bool Save(rapidjson::Value& keyFrameValue, rapidjson::Document& jsonDoc) const;

	/**
	 * Calculate this key-frame as the linear interpolation of the two given key-frames by
	 * the given time index.  Note that there is no need for the two given key-frames to
	 * come from the same animation.  Typically, that will be the case.  But it need not be,
	 * such as in the case of blending from one animation to another.
	 */
	bool Interpolate(const KeyFrame* keyFrameA, const KeyFrame* keyFrameB, double timeSeconds);

	/**
	 * Set the current pose of bones in the given skeleton.  Note that we do nothing here
	 * to check that the set of bones here matches the set of bones in the given skeleton.
	 * In fact, for these sets to mismatch in any way is a feature, not an error, because
	 * you may want an animation to drive a sub-set of bones in a given skeleton.  This way,
	 * you can, potentially, have multiple, different animations operating on a single
	 * skeleton simultaneously.  For this to be reasonable, these animations would, presumably,
	 * operate on disjoint subsets of the skeleton's bones.  If a bone in this key-frame is
	 * not found in the given skeleton, then we just ignore it.
	 * 
	 * @param[in,out] skeleton This is the skeleton who's bones are posed by this key-frame.
	 * @return We return the number of bones posed in the given skeleton.
	 */
	int Pose(Skeleton* skeleton) const;

	/**
	 * Take the current pose of the given skeleton and make a key-frame from it.
	 */
	void MakePose(const Skeleton* skeleton);

private:

	double timeSeconds;

	struct PoseInfo
	{
		std::string boneName;
		BoneState boneState;
	};

	std::vector<PoseInfo> poseInfoArray;
};