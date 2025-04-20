// This source code is property of the Computer Graphics and Visualization 
// chair of the TU Dresden. Do not distribute! 
// Copyright (C) CGV TU Dresden - All Rights Reserved
//
#pragma once

#include "common.h"
#include "IHasBoundingBox.h"

#include <libs/cgv_gl/gl/gl.h>

#include <cgv/render/shader_code.h>
#include <cgv/render/shader_program.h>

#include <string>

class Mesh : public IHasBoundingBox
{
public:
	Mesh();
	~Mesh();

	//Compiles all used shaders.
	static bool init_shaders(cgv::render::context& ctx);

	//Loads geometry data from an OBJ file.
	bool read_obj(const char* filename);

	//Loads attachment data from a Pinocchio file.
	void read_attachment(std::string filename);

	//Draws the mesh
	void draw(cgv::render::context& ctx);

	//Sets the matrices used for skinning.
	void set_skinning_matrices(const std::vector<Mat4>& matrices);

private:
	static cgv::render::shader_program prog;

	std::vector<cgv::math::fvec<float, 3>> positions;
	std::vector<unsigned int> indices;	

	GLuint indexBuffer;
	GLuint positionBuffer;
	GLuint boneIndexBuffer;
	GLuint boneWeightBuffer;
	GLuint vao;

	GLfloat* skinning_matrices;
	int n_bones;

	bool has_attachment;
};