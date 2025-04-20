// This source code is property of the Computer Graphics and Visualization 
// chair of the TU Dresden. Do not distribute! 
// Copyright (C) CGV TU Dresden - All Rights Reserved
//
#include "Animation.h"

#include <fstream>

bool Animation::read_amc_file(std::string filename, Skeleton* s)
{
	frames.clear();

	std::ifstream fin;
#ifdef _WIN32
	std::wstring wfilename = cgv::utils::str2wstr(filename);
	fin.open(wfilename);
#else
	fin.open(filename);
#endif
	if (!fin.good())
		return false;

	try{
		while (!fin.eof())
		{
			char buf[CHARS_PER_LINE];
			fin.getline(buf, CHARS_PER_LINE);
			std::string str(buf);
			str = trim(str); //remove whitespaces

			if (str.empty())
				continue;
			if (str.find('#') == 0)
				continue; //don't handle comments
			if (str.find(':') == 0)
				continue; //don't handle control sequences

			if (std::isdigit(str[0]))
			{
				//new frame
				frames.push_back(std::vector<AnimationFrameBone>());
				continue;
			}

			//must be a bone line
			std::stringstream ss(str);
			std::string bone_name;
			ss >> bone_name;

			Bone* bone = s->find_bone(bone_name);
			if (bone == nullptr)
				return false;

			std::vector<double> dof_values(bone->dof_count());			
			for (int i = 0; i < bone->dof_count(); ++i)
			{
				double value;
				ss >> value;
				dof_values[i] = value;
			}

			frames.back().push_back(AnimationFrameBone());
			frames.back().back().bone = bone;
			frames.back().back().dof_values.resize(bone->dof_count());
			for (int i = 0; i < bone->dof_count(); ++i)
			{
				frames.back().back().dof_values[i] = dof_values[bone->get_dof(i)->get_index_in_amc()];
			}
		}
		fin.close();
	}
	catch (...)
	{
		fin.close();
		return false;
	}
	return true;
}

int Animation::frame_count() const { return (int)frames.size(); }

void Animation::apply_frame(int frame) const
{
	/*Bonus task: apply animated DoF scalar parameter from given frame to the skeleton. */
}
