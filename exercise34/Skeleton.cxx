// This source code is property of the Computer Graphics and Visualization 
// chair of the TU Dresden. Do not distribute! 
// Copyright (C) CGV TU Dresden - All Rights Reserved
//
#include "Skeleton.h"

#include <fstream>
#include <map>
#include <string>
#include <sstream>

#include "math_helper.h"

Skeleton::Skeleton()
{
	origin.identity();
}

Skeleton::~Skeleton()
{
	if (root)
		delete root;
}

Bone* Skeleton::get_root() const { return root; }
const Mat4& Skeleton::get_origin() const { return origin; }
Vec3 Skeleton::get_origin_vec() const { return Vec3(origin(0, 3), origin(1, 3), origin(2,3)); }
void Skeleton::set_origin(const Vec3& v) { origin = translate(v); }
void Skeleton::set_origin(const Mat4& m) { origin = m; }

enum ParseState
{
	Ignore,
	Root,
	BoneData,
	Hierarchy,
};

bool Skeleton::fromASFFile(const std::string& filename)
{
	origin.identity();

	std::ifstream fin;
	reset_bounding_box();
	ParseState state = Ignore;

	Bone* current_node = nullptr;

	bones.clear();

	int n_dofs;

	try
	{
#ifdef _WIN32
		std::wstring wfilename = cgv::utils::str2wstr(filename);
		fin.open(wfilename);
#else
		fin.open(filename);
#endif
		if (!fin.good())
			return false;
		while (!fin.eof())
		{
			char buf[CHARS_PER_LINE];
			fin.getline(buf, CHARS_PER_LINE);
			std::string str(buf);
			str = trim(str); //remove whitespaces
			if (str.find('#') == 0)
				continue; //don't handle comments

			if (str.find(':') == 0)
			{
				if (str.find(":version") == 0)
					version = str.substr(9);
				else if (str.find(":name") == 0)
					name = str.substr(6);
				else if (str.find(":root") == 0)
				{
					state = Root;
					current_node = new Bone();
					current_node->set_name("root");
					bones["root"] = current_node;
				}
				else if (str.find(":bonedata") == 0)
					state = BoneData;
				else if (str.find(":hierarchy") == 0)
					state = Hierarchy;
				else state = Ignore;
				continue;
			}
			switch (state)
			{
			case Ignore:
				//ignore this line
				break;
			case Root:
				if (str.find("order") == 0)
				{
					std::string dofs = str.substr(6);
					std::string dofstr;
					std::stringstream ss(dofs);
					n_dofs = 0;
					while (!ss.eof())
					{
						ss >> dofstr;
						AtomicTransform* dof;
						if (dofstr.find("RX") == 0)
							dof = new AtomicXRotationTransform();
						else if (dofstr.find("RY") == 0)
							dof = new AtomicYRotationTransform();
						else if (dofstr.find("RZ") == 0)
							dof = new AtomicZRotationTransform();
						else if (dofstr.find("TX") == 0)
							dof = new AtomicXTranslationTransform();
						else if (dofstr.find("TY") == 0)
							dof = new AtomicYTranslationTransform();
						else if (dofstr.find("TZ") == 0)
							dof = new AtomicZTranslationTransform();
						dof->set_value(0);
						current_node->add_dof(dof);
						++n_dofs;
					}
				}				
				break;
			case BoneData:
				if (str.find("begin") == 0)
					current_node = new Bone();
				else if (str.find("name") == 0)
				{
					std::string name = str.substr(5);
					current_node->set_name(name);
					bones[name] = current_node;
				}
				else if (str.find("direction") == 0)
				{
					std::string direction = str.substr(10);
					float dx, dy, dz;
					std::stringstream ss(direction);
					ss >> dx >> dy >> dz;
					current_node->set_direction_in_world_space(Vec3(dx, dy, dz));
				}
				else if (str.find("length") == 0)
				{
					std::string length = str.substr(7);
					float l;
					std::stringstream ss(length);
					ss >> l;
					current_node->set_length(l);
				}
				else if (str.find("axis") == 0)
				{
					std::string axis = str.substr(5);
					float a[3];
					std::string order;
					std::stringstream ss(axis);
					ss >> a[0] >> a[1] >> a[2] >> order;
					for (int i = 0; i < 3; ++i)
					{
						AtomicRotationTransform* t;
						if (order.at(i) == 'X')
							t = new AtomicXRotationTransform();
						else if (order.at(i) == 'Y')
							t = new AtomicYRotationTransform();
						else if (order.at(i) == 'Z')
							t = new AtomicZRotationTransform();
						t->set_value(a[i]);
						current_node->add_axis_rotation(t);
					}
				}
				else if (str.find("dof") == 0)
				{
					std::string dofs = str.substr(4);
					std::string dofstr;
					std::stringstream ss(dofs);
					n_dofs = 0;
					while (!ss.eof())
					{
						ss >> dofstr;
						AtomicTransform* dof;
						if (dofstr.find("rx") == 0)
							dof = new AtomicXRotationTransform();
						else if (dofstr.find("ry") == 0)
							dof = new AtomicYRotationTransform();
						else if (dofstr.find("rz") == 0)
							dof = new AtomicZRotationTransform();
						current_node->add_dof(dof);
						++n_dofs;
					}
				}
				else if (str.find("limits") == 0)
				{
					std::string limits = str.substr(7);
					for (int i = 0; i < n_dofs; ++i)
					{
						if (i > 0)
						{
							fin.getline(buf, CHARS_PER_LINE);
							limits = std::string(buf);
							limits = trim(limits);
						}

						limits = limits.substr(limits.find('(') + 1);
						std::stringstream ss(limits);
						float lower, upper;
						ss >> lower >> upper;
						current_node->get_dof(n_dofs - i - 1)->set_limits(lower, upper);
					}

				}
				break;
			case Hierarchy:
				if (str.find("begin") == std::string::npos && str.find("end") == std::string::npos)
				{
					std::stringstream ss(str);
					std::string parentstr;
					ss >> parentstr;
					Bone* parent = bones[parentstr];
					std::string childstr;
					while (!ss.eof())
					{
						ss >> childstr;
						Bone* child = bones[childstr];
						parent->add_child(child);
					}
				}
			}
		}
		fin.close();
		root = bones["root"];

		//root->revert_dofs();

		postprocess(root, Vec3(0, 0, 0));
		return true;
	}
	catch (...)
	{
		fin.close();
		return false;
	}
}

void Skeleton::postprocess(Bone* node, const Vec3& global_position)
{
	//For display adaptation
	auto bone_offset_in_global_system = node->get_direction_in_world_space() * node->get_length();
	auto bone_tip_in_global_system = global_position + bone_offset_in_global_system;
	add_point(bone_tip_in_global_system);

	node->calculate_matrices();
	int n = node->childCount();
	for (int i = 0; i < n; ++i)
	{
		postprocess(node->child_at(i), bone_tip_in_global_system);
	}
}

void Skeleton::write_pinocchio_file(const std::string& filename)
{
	std::ofstream o;
#ifdef _WIN32
	std::wstring wfilename = cgv::utils::str2wstr(filename);
	o.open(wfilename, std::ios::out);
#else
	o.open(filename, std::ios::out);
#endif	

	if (o)
	{
		/*Task 4.1: Write Pinocchio file into o */
	}
	o.close();
}



void Skeleton::read_pinocchio_file(std::string filename)
{
	std::ifstream o;
#ifdef _WIN32
	std::wstring wfilename = cgv::utils::str2wstr(filename);
	o.open(wfilename, std::ios::in);
#else
	o.open(filename, std::ios::in);
#endif
	if (o)
	{
		/*Task 4.3: Read Pinocchio file */
	}

	o.close();

	postprocess(root, get_origin_vec());
}



void Skeleton::get_skinning_matrices(std::vector<Mat4>& matrices)
{
	/*Task 4.5: Calculate skinning matrices */
}



Bone* Skeleton::find_bone(const std::string& name) const
{ 
	auto it = bones.find(name); 
	if (it == bones.end())
		return nullptr;
	else
		return it->second;
}
