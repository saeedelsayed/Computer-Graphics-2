// This source code is property of the Computer Graphics and Visualization 
// chair of the TU Dresden. Do not distribute! 
// Copyright (C) CGV TU Dresden - All Rights Reserved
//
#include "SkeletonViewer.h"

#include <cgv/utils/ostream_printf.h>
#include <cgv/gui/file_dialog.h>
#include <cgv/gui/dialog.h>
#include <cgv/gui/key_event.h>
#include <cgv/render/view.h>
#include <cgv/base/find_action.h>
#include <cgv/media/illum/surface_material.h>

#include "math_helper.h"

using namespace cgv::utils;

cgv::render::shader_program Mesh::prog;

// The constructor of this class
SkeletonViewer::SkeletonViewer(DataStore* data)
	: node("Skeleton Viewer"), data(data)
	/* Bonus task: initialize members relevant for animation */
{
	connect(data->skeleton_changed, this, &SkeletonViewer::skeleton_changed);

	connect(get_animation_trigger().shoot, this, &SkeletonViewer::timer_event);

	// Prepare surface material
	material.set_brdf_type((cgv::media::illum::BrdfType)(cgv::media::illum::BT_LAMBERTIAN | cgv::media::illum::BT_PHONG));
	material.ref_specular_reflectance() = { .03125f, .03125f, .03125f };
	material.ref_roughness() = .03125f;
}

//draws a part of a skeleton, represented by the given root node
void SkeletonViewer::draw_skeleton_subtree(
	Bone* node, const Mat4& parent_system_transf_local_to_global, context& ctx, int level
){
	////
	// Task 3.2, 3.3, 4.3: Visualize the skeleton
}

void SkeletonViewer::timer_event(double, double dt)
{
	////
	// Bonus task: implement animation */
}

void SkeletonViewer::start_animation()
{
	////
	// Bonus task: implement animation
}

void SkeletonViewer::stop_animation()
{
	////
	// Bonus task: implement animation
}

void SkeletonViewer::skeleton_changed(std::shared_ptr<Skeleton> s) 
{
	// This function is called whenever the according signal of the
	// data store has been called.

	//Rebuild the tree-view
	generate_tree_view_nodes();

	//Fit view to skeleton
	std::vector<cgv::render::view*> view_ptrs;
	cgv::base::find_interface<cgv::render::view>(get_node(), view_ptrs);
	if (view_ptrs.empty()) {
		// If there is no view, we cannot update it
		cgv::gui::message("could not find a view to adjust!!");
	}
	else {
		Vec3 center = (s->getMin() + s->getMax()) * 0.5;
		view_ptrs[0]->set_focus(center.x(), center.y(), center.z());
		// Set the scene's size at the focus point
		view_ptrs[0]->set_y_extent_at_focus(s->getMax().y() - s->getMin().y());
	}

	//connect signals
	recursive_connect_signals(s->get_root());

	post_redraw();
}

void SkeletonViewer::recursive_connect_signals(Bone* b)
{
	for (int i = 0; i < b->dof_count(); ++i)
		connect(b->get_dof(i)->changed_signal, this, &SkeletonViewer::dof_changed);
	for (int i = 0; i < b->childCount(); ++i)
		recursive_connect_signals(b->child_at(i));
}

void SkeletonViewer::dof_changed(double)
{
	if (!data->dof_changed_by_ik)
		data->set_endeffector(nullptr);

	post_redraw();
}

void SkeletonViewer::generate_tree_view_nodes()
{
	tree_view->remove_all_children();
	gui_to_bone.clear();

	if (!data->get_skeleton() || !data->get_skeleton()->get_root())
		return;
	generate_tree_view_nodes(tree_view, data->get_skeleton()->get_root());
}

void SkeletonViewer::generate_tree_view_nodes(gui_group_ptr parent, Bone* bone)
{
	if (bone->childCount() == 0)
	{
		//If this is a leaf, use a button
		auto button = parent->add_button(bone->get_name(), "", "");
		gui_to_bone[button] = bone;
	}
	else
	{
		//If this is not a leaf, use a group
		auto g = parent->add_group(bone->get_name(), "", "", "");
		gui_to_bone[g] = bone;
		for (int i = 0; i < bone->childCount(); ++i)
			generate_tree_view_nodes(g, bone->child_at(i));
	}
}

void SkeletonViewer::start_choose_base()
{
	Bone* b = data->get_endeffector();
	data->set_endeffector(nullptr);
	data->set_base(b);
}

void SkeletonViewer::tree_selection_changed(base_ptr p, bool select)
{
	bone_group->remove_all_children();

	if (select)		
	{
		Bone* bone = gui_to_bone.at(p);
		generate_bone_gui(bone);
		data->set_endeffector(bone);
	}
	else
	{
		data->set_endeffector(nullptr);
	}
}

std::string SkeletonViewer::get_parent_type() const
{
	return "layout_group";
}

void SkeletonViewer::load_skeleton()
{
	std::string filename = cgv::gui::file_open_dialog("Open", "Skeleton Files (*.asf):*.asf");
	if (!filename.empty())
	{
		auto s = std::make_shared<Skeleton>();
		if (s->fromASFFile(filename))
		{
			data->set_skeleton(s);
			data->set_endeffector(nullptr);
			data->set_base(s->get_root());
		}
		else
		{
			cgv::gui::message("Could not load specified ASF file.");
		}
	}
}

void SkeletonViewer::write_pinocchio()
{
	if (!data->get_skeleton())
	{
		cgv::gui::message("An ASF skeleton has to be loaded first.");
		return;
	}

	std::string filename = cgv::gui::file_save_dialog("Save", "Pinocchio Skeleton (*.txt):*.txt");
	if (!filename.empty())
	{
		data->get_skeleton()->write_pinocchio_file(filename);
	}
}

void SkeletonViewer::load_pinocchio()
{
	if (!data->get_skeleton())
	{
		cgv::gui::message("An ASF skeleton has to be loaded first.");
		return;
	}

	std::string filename = cgv::gui::file_open_dialog("Open", "Pinocchio skeleton (*.out):*.out");
	if (!filename.empty())
	{
		data->get_skeleton()->read_pinocchio_file(filename);		
		skeleton_changed(data->get_skeleton());
	}
}

void SkeletonViewer::load_animation()
{
	if (!data->get_skeleton())
	{
		cgv::gui::message("An ASF skeleton has to be loaded first.");
		return;
	}

	std::string filename = cgv::gui::file_open_dialog("Open", "Animation File (*.amc):*.amc");
	if (!filename.empty())
	{
		/*Bonus task: load animation from selected file */
	}
}

// Perform initialization
bool SkeletonViewer::init(context &ctx)
{
	// Cause context re-creation in compatibility profile in case we're in core, to enable more ergonomic drawing code
	// (legacy OpenGL immediate mode) for drawing the indicators in task 3.3
	// - obtain reflection-enabled interface to the context
	auto &ctx_as_base = dynamic_cast<base&>(ctx);
	// - activate compatibility profile (may cause context re-creation and thus recursion into this ::init()
	ctx_as_base.set("core_profile", false);

	// Done!
	return true;
}

// Create the gui elements
void SkeletonViewer::create_gui()
{	
	//Bone tree view
	parent_group->multi_set("layout='table';rows=3;spacings='normal';");	

	tree_view = add_group("", "tree_group", "h=300;column_heading_0='Bones';column_width_0=-1", "f");	
	bone_group = add_group("", "align_group", "h=150", "f");
	
	auto dock_group = add_group("", "dockable_group", "", "fF");

	connect(tree_view->on_selection_change, this, &SkeletonViewer::tree_selection_changed);

	generate_tree_view_nodes();

	//Other GUI elements
	auto gui_group = dock_group->add_group("", "align_group", "", "f");

	connect_copy(gui_group->add_button("Load ASF skeleton", "", "\n")->click,
		rebind(this, &SkeletonViewer::load_skeleton));	

	connect_copy(gui_group->add_button("Load Animation", "", "\n")->click,
		rebind(this, &SkeletonViewer::load_animation));

	connect_copy(gui_group->add_button("Start Animation", "", "\n")->click,
		rebind(this, &SkeletonViewer::start_animation));

	connect_copy(gui_group->add_button("Stop Animation", "", "\n")->click,
		rebind(this, &SkeletonViewer::stop_animation));

	connect_copy(gui_group->add_button("Choose IK Base", "", "\n")->click,
		rebind(this, &SkeletonViewer::start_choose_base));
		
	connect_copy(gui_group->add_button("Write Pinocchio skeleton", "", "\n")->click,
		rebind(this, &SkeletonViewer::write_pinocchio));

	connect_copy(gui_group->add_button("Load Pinocchio skeleton", "", "\n")->click,
		rebind(this, &SkeletonViewer::load_pinocchio));
}

void SkeletonViewer::generate_bone_gui(Bone* bone)
{
	// Add bone-specific gui elements to bone_group.
	// Use the layout "\n" to specify vertical alignment

	bone_group->add_view("Selected Bone", bone->get_name());

	for (int i = 0; i < bone->dof_count(); ++i)
	{
		auto dof = bone->get_dof(i);
		auto slider = bone_group->add_control<double>(dof->get_name(), dof.get(), "value_slider");
		slider->set("min", dof->get_lower_limit());
		slider->set("max", dof->get_upper_limit());
	}
}

void SkeletonViewer::draw(context& ctx)
{
	////
	// Task 3.3: If you require it, extend this method with additional logic

	if (data->get_skeleton() != nullptr)
		draw_skeleton_subtree(data->get_skeleton()->get_root(), data->get_skeleton()->get_origin(), ctx, 0);
}
