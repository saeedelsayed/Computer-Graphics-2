// This source code is property of the Computer Graphics and Visualization 
// chair of the TU Dresden. Do not distribute! 
// Copyright (C) CGV TU Dresden - All Rights Reserved
//
#pragma once

#include "common.h"
#include "AnimationFrameBone.h"
#include "Skeleton.h"

#include <string>

class Animation
{
public:
	bool read_amc_file(std::string filename, Skeleton*);
	int frame_count() const;

	void apply_frame(int frame) const;


private:
	//Contains a std::vector<AnimationFrameBone> for each frame, which contains animation data for a set of bones.
	std::vector<std::vector<AnimationFrameBone>> frames;
};
