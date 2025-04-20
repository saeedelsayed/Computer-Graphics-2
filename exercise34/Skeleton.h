// This source code is property of the Computer Graphics and Visualization 
// chair of the TU Dresden. Do not distribute! 
// Copyright (C) CGV TU Dresden - All Rights Reserved
//
#pragma once

#include "Bone.h"
#include "common.h"
#include "IHasBoundingBox.h"

#include <vector>


class Skeleton : public IHasBoundingBox
{
public:
	Skeleton();
	~Skeleton();

	//Loads skeleton data from an ASF file.
	bool fromASFFile(const std::string& filename);

	//Writes the current skeleton to a Pinocchio file.
	void write_pinocchio_file(const std::string& filename);

	//Loads a Pinocchio skeleton file and adapts the current skeleton according to this file.
	void read_pinocchio_file(std::string filename);

	//Fille the matrices vector with the skinning matrices for all bones in DFS order
	void get_skinning_matrices(std::vector<Mat4>& matrices);

	//Returns the skeleton's root bone
	Bone* get_root() const;

	const Mat4& get_origin() const;
	Vec3 get_origin_vec() const;
	void set_origin(const Vec3&);
	void set_origin(const Mat4&);

	Bone* find_bone(const std::string& name) const;

private:
	Mat4 origin;
	std::string version;
	std::string name;
	Bone* root;
	void postprocess(Bone* node, const Vec3& global_position);

	
	
	

	std::map<const std::string, Bone*> bones;
};
