// This source code is property of the Computer Graphics and Visualization 
// chair of the TU Dresden. Do not distribute! 
// Copyright (C) CGV TU Dresden - All Rights Reserved
//
// The main file of the plugin. It defines a global register variable
// "global_initializer" that is initialized when the plugin is loaded.

#include <cgv/base/register.h>
#include "DataStore.h"
#include "SkeletonViewer.h"
#include "SkinnedMeshViewer.h"
#include "IKViewer.h"

using namespace cgv::base;

struct Initializer
{
	DataStore* data;

	Initializer()
	{	
		data = new DataStore();

		register_object(base_ptr(new SkeletonViewer(data)), "");
		register_object(base_ptr(new IKViewer(data)), "");
		//register_object(base_ptr(new SkinnedMeshViewer(data)), "");
	}

	~Initializer()
	{
		delete data;
	}

} global_initializer;
