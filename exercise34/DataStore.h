// This source code is property of the Computer Graphics and Visualization 
// chair of the TU Dresden. Do not distribute! 
// Copyright (C) CGV TU Dresden - All Rights Reserved
//
#pragma once

#include "Skeleton.h"
#include "Mesh.h"
#include "Animation.h"

#include <cgv/signal/signal.h>
#include <memory>

class DataStore
{
public:

	// This signal is used to report a change of the skeleton to the viewer.
	// Skeleton* is the only parameter of this signal. There are no signals
	// with no parameters.
	// Use connect(...) and connect_copy(...) to register a listener.
	// Use operator() to call a signal.
	cgv::signal::signal<std::shared_ptr<Skeleton>> skeleton_changed;

	cgv::signal::signal<std::shared_ptr<Mesh>> mesh_changed;

	cgv::signal::signal<Bone*> endeffector_changed;
	cgv::signal::signal<Bone*> base_changed;

	DataStore();

	~DataStore();

	//Used for Skeleton Visualization
	void set_skeleton(std::shared_ptr<Skeleton>);
	std::shared_ptr<Skeleton> get_skeleton() const;

	//Used for Skinning
	void set_mesh(std::shared_ptr<Mesh>);
	std::shared_ptr<Mesh> get_mesh() const;

	//Used for IK
	void set_endeffector(Bone*);
	Bone* get_endeffector() const;

	void set_base(Bone*);
	Bone* get_base() const;

	//Specifies if the next dof change is caused by the Inverse Kinematics module
	bool dof_changed_by_ik;

private:
	std::shared_ptr<Skeleton> skeleton;
	std::shared_ptr<Mesh> mesh;
	Bone* endeffector, *base;
};