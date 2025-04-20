// This source code is property of the Computer Graphics and Visualization 
// chair of the TU Dresden. Do not distribute! 
// Copyright (C) CGV TU Dresden - All Rights Reserved
//
#pragma once

#include "AtomicTransform.h"

#include <deque>
#include <memory>


// Represents a bone in a hierarchical skeleton
class Bone
{	
public:
	Bone();
	~Bone();

	//Adds a new degree of freedom to the bone. The bone maintains separate groups of
	//translation transforms and other transforms. All translation transforms go first 
	//in the dof list, followed by all other transforms. Every new transform is added
	//to the beginning of the respective group.
	void add_dof(AtomicTransform* dof);

	//Returns the number of degrees of freedom of this bone.
	int dof_count() const;

	//Returns the degree of freedom with the given index.
	std::shared_ptr<AtomicTransform> get_dof(int dofIndex) const;

	//Sets the identifying name of the bone
	void set_name(const std::string& name);

	//Returns the identifying name of the bone
	const std::string& get_name() const;

	//Sets a unit vector that describes the bone's direction in world coordinates 
	//(independent of the hierarchy)
	void set_direction_in_world_space(const Vec3& direction);

	//Returns a unit vector that describes the bone's direction in world coordinates
	//(independent of the hierarchy)
	const Vec3& get_direction_in_world_space() const;

	//Sets the bone's geometric length
	void set_length(float length);

	//Returns the bone's geometric length
	float get_length() const;

	//Modifies the orientation of the bone's local coordinate system by setting
	//  O <= T * O,
	//where O is the bone's local coordinate system transform and T is the new transform.
	void add_axis_rotation(AtomicRotationTransform* transform);

	//Adds a new child to this bone and sets the parent of the child to the current bone.
	void add_child(Bone* child);

	//Returns the child with index i.
	Bone* child_at(int i) const;

	//Returns the number of children of this bone.
	int childCount() const;

	//Specifies the parent of this bone. The root bone has a null parent.
	void set_parent(Bone* parent);

	//Returns the parent of this bone. The root bone has a null parent.
	Bone* get_parent() const;

	//Calculates all relevant matrices from the given information in the ASF file.
	void calculate_matrices();

	//Calculates a matrix that transforms the parent's local coordinate system to the current bone's local coordinate system (model transform). 
	//It includes all dofs of the bone. Implemented in Task 3.1
	Mat4 calculate_transform_prev_to_current_with_dofs();

	//Calculates a matrix that transforms the parent's local coordinate system to the current bone's local coordinate system (model transform).
	//It does not include any of the bone's dofs. Implemented in Task 3.1.
	Mat4 calculate_transform_prev_to_current_without_dofs();

	//Returns a matrix that represents a translation from the current bone to the next bone in the current bone's local coordinate system.
	//Implemented in Task 3.1.
	const Mat4& get_translation_transform_current_joint_to_next() const;

	//Returns a matrix that represents a rotation from the previous bone to the current bone in the previous bone's local coordinate system.
	//Implemented in Task 3.1.
	const Mat4& get_orientation_transform_prev_joint_to_current() const;

	//Returns the zero-vector (with w-component 1)
	Vec4 get_bone_local_root_position() const;

	//Returns the position of the bone's tip in the bone's coordinate system. Available after implementing Task 3.1.
	Vec4 get_bone_local_tip_position() const;

	//Returns the system transform that transforms positions from the global coordinate system to the bone's local coordinate system.
	//Available after implementing Task 3.6.
	const Mat4& get_binding_pose_matrix() const;

private:
	//The following attributes are read from the ASF file
	std::deque<std::shared_ptr<AtomicTransform>> dofs; //Degrees of freedom for the bone
	std::string name; //The bone's name
	Vec3 direction_in_world_space; //The bone's direction in world space (unit vector)
	float length; //The bone's length
	std::deque<AtomicTransform*> orientation; //The model transform from global space to bone space; multiply together from left to right

	std::vector<Bone*> children; //child bones
	Bone* parent; //parent bone	


	// ------------------------------
	// Calculated attributes

    // Model transformation that rotates to the current bone from the previous bone (in the previous bone's coordinate system); Task 3.1
	Mat4 orientationModelTransformPrevJointToCur;
    // Model transformation that translates to the next bone from the current bone (in the current bone's coordinate system); Task 3.1
    Mat4 translationModelTransformCurJointToNext;

    // System transformation that rotates bone-local coordinates into the global system, i.e. yields globally oriented coordinates from bone-local coordinates.
    // Available from the beginning.
    Mat4 orientationSystemTransformLocalToGlobal;
    // Model transformation that rotates onto the current bone from the global orientation (in global coordinates).
    // Automatically tracks the corresponding system transformation. Available from the beginning.
    Mat4 &orientationModelTransformGlobalToLocal = orientationSystemTransformLocalToGlobal;

    // System transformation that rotates global coordinates into the bone-local system, i.e. yields locally oriented coordinates from global coordinates.
    // Available from the beginning.
    Mat4 orientationSystemTransformGlobalToLocal;
    // Model transformation that rotates onto the global orientation from the current bone (in the current bone's coordinate system).
    // Automatically tracks the corresponding system transformation. Available from the beginning.
    Mat4 &orientationModelTransformLocalToGlobal = orientationSystemTransformGlobalToLocal;


    // ------------------------------
    // For skinning

    // System transformation that translates bone-local coordinates into the global system, i.e. yields the globally translated coordinates from bone-local coordinates; Task 4.5
    Mat4 translationSystemTransformLocalToGlobal;
    // Model transformation that translates to the current bone from the global system (in global coordinates); Task 4.5
    // Automatically tracks the corresponding system transformation.
    Mat4 &translationModelTransformGlobalToLocal = translationSystemTransformLocalToGlobal;

    // System transformation that transforms (orientation and translation) bone-local coordinates into the global system, i.e. yields the globally transformed coordinates from bone-local coordinates; Task 4.5
    Mat4 systemTransformLocalToGlobal;
    // Model transformation that transforms (orientation and translation) onto the current bone from the global system (in global coordinates); Task 4.5
    // Automatically tracks the corresponding system transformation.
    Mat4 &modelTransformGlobalToLocal = systemTransformLocalToGlobal;

    // System transformation that transforms (orientation and translation) global coordinates into the bone-local system, i.e. yields local coordinates from global coordinates; Task 4.5
    Mat4 systemTransformGlobalToLocal;
    // Model transformation that transforms (orientation and translation) onto the global system from the current bone (in coordinates relative to the current bone); Task 4.5
    // Automatically tracks the corresponding system transformation.
    Mat4 &modelTransformLocalToGlobal = systemTransformGlobalToLocal;

	int translationTransforms; //The number of translation transforms that have been added as dof
};
