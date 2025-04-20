#include "gl_implicit_surface_drawable.h"
#include <cgv/utils/progression.h>
#include <cgv/signal/rebind.h>
#include <cgv/base/group.h>
#include <cgv/gui/file_dialog.h>
#include <cgv/base/register.h>
#include <cgv/utils/file.h>
#include <cgv/utils/stopwatch.h>
#include <fstream>

using namespace cgv::gui;
using namespace cgv::math;
using namespace cgv::base;
using namespace cgv::signal;
using namespace cgv::render::gl;
using namespace cgv::media;

gl_implicit_surface_drawable::gl_implicit_surface_drawable() 
{
#ifdef _DEBUG
	res = 24;
#else
	res = 64;
#endif
	box_scale = 1.2f;

	material.set_brdf_type((illum::BrdfType)(illum::BT_LAMBERTIAN | illum::BT_PHONG));
	material.ref_diffuse_reflectance() = {.0625f, .25f, .45f};
	material.ref_specular_reflectance() = {.0625f, .0625f, .0625f};
	material.ref_emission() = {.03125f, 1/5.f, 1/3.f};
	material.ref_roughness() = .0625f;
	brs.material.set_brdf_type((illum::BrdfType)(illum::BT_LAMBERTIAN | illum::BT_PHONG));
	brs.material.ref_diffuse_reflectance() = {.45f, .45f, .45f};
	brs.material.ref_specular_reflectance() = {.0625f, .0625f, .0625f};
	brs.material.ref_emission() = {.03125f, .03125f, .03125f};
	brs.material.ref_roughness() = .03125f;
}

std::string gl_implicit_surface_drawable::get_type_name() const
{
	return "implicit_surface";
}

void gl_implicit_surface_drawable::toggle_range()
{
	std::swap(map_to_zero_value, map_to_one_value);
	update_member(&map_to_zero_value);
	update_member(&map_to_one_value);
}

void gl_implicit_surface_drawable::adjust_range()
{
	// prepare private members
	pnt_type p = box.get_min_pnt();
	pnt_type d = box.get_extent();
	d(0) /= (res - 1); d(1) /= (res - 1); d(2) /= (res - 1);
	
	// prepare progression
	cgv::utils::progression prog;
	prog.init("adjust range", res, 10);
	
	// iterate through all slices
	bool set = false;
	unsigned int i, j, k;
	for (k = 0; k < res; ++k, p(2) += d(2)) {
		prog.step();
		for (j = 0, p(1) = box.get_min_pnt()(1); j < res; ++j, p(1) += d(1)) {
			for (i = 0, p(0) = box.get_min_pnt()(0); i < res; ++i, p(0) += d(0)) {
				double v = func_ptr->evaluate(p.to_vec());
				if (set) {
					if (v < map_to_zero_value)
						map_to_zero_value = v;
					if (v > map_to_one_value)
						map_to_one_value = v;

				}
				else {
					map_to_zero_value = v;
					map_to_one_value = v;
					set = true;
				}
			}
		}
	}
	update_member(&map_to_zero_value);
	update_member(&map_to_one_value);
}

void gl_implicit_surface_drawable::export_volume()
{
	std::string fn = file_save_dialog("choose vox output file", "Obj Files (vox):*.vox|All Files:*.*");
	if (fn.empty())
		return;
	std::string hd_fn = cgv::utils::file::drop_extension(fn) + ".hd";

	std::ofstream os(hd_fn.c_str());
	if (os.fail())
		return;
	os << "Size:      " << res << ", " << res << ", " << res << std::endl;
	pnt_type scaling = box.get_extent(); // / pnt_type(res, res, res);

	os << "Spacing:   " << scaling(0) << ", " << scaling(1) << ", " << scaling(2) << std::endl;
	os.close();

	std::vector<unsigned char> data;
	// prepare private members
	pnt_type p = box.get_min_pnt();
	pnt_type d = box.get_extent();
	d(0) /= (res - 1); d(1) /= (res - 1); d(2) /= (res - 1);

	// prepare progression
	cgv::utils::progression prog;
	prog.init("export volume", res, 10);

	// iterate through all slices
	bool set = false;
	unsigned int i, j, k;
	for (k = 0; k < res; ++k, p(2) += d(2)) {
		prog.step();
		for (j = 0, p(1) = box.get_min_pnt()(1); j < res; ++j, p(1) += d(1)) {
			for (i = 0, p(0) = box.get_min_pnt()(0); i < res; ++i, p(0) += d(0)) {
				double v = func_ptr->evaluate(p.to_vec());
				unsigned char value;
				if (map_to_zero_value < map_to_one_value) {
					if (v <= map_to_zero_value)
						value = 0;
					else if (v >= map_to_one_value)
						value = 255;
					else
						value = (unsigned char)(int) (255 * (v - map_to_zero_value) / (map_to_one_value - map_to_zero_value));
				}
				else {
					if (v >= map_to_zero_value)
						value = 0;
					else if (v <= map_to_one_value)
						value = 255;
					else
						value = (unsigned char)(int) (255 * (map_to_zero_value - v) / (map_to_zero_value - map_to_one_value));
				}
				data.push_back(value);
			}
		}
	}
	cgv::utils::file::write(fn, (const char*)&data.front(), data.size());
}

/// callback used to save to obj file
void gl_implicit_surface_drawable::save_interactive()
{
	std::string fn = file_save_dialog("choose obj output file", "Obj Files (obj):*.obj|All Files:*.*");
	if (fn.empty())
		return;
	std::ofstream os(fn.c_str());
	if (os.fail())
		return;
	obj_out = &os;
	normal_index = 0;
	surface_extraction();
	obj_out = 0;
}

void gl_implicit_surface_drawable::surface_extraction()
{
	double time;
	cgv::utils::stopwatch sw(&time);
	gl_implicit_surface_drawable_base::surface_extraction();
	time = sw.get_elapsed_time();
	std::cout << "[CONTOURING] Surface extraction finished in " << time << "s." << std::endl;
	update_member(&nr_faces);
	update_member(&nr_vertices);
}

void gl_implicit_surface_drawable::build_display_list()
{
	if (find_view(nr_faces)) {
		find_view(nr_faces)->update();
		find_view(nr_vertices)->update();
	}
}

void gl_implicit_surface_drawable::resolution_change()
{
	if (find_control(ix)) {
		find_control(ix)->set("max",res-1);
		find_control(iy)->set("max",res-1);
		find_control(iz)->set("max",res-1);
	}
	post_rebuild();
}

/// you must overload this for gui creation
void gl_implicit_surface_drawable::create_gui()
{
	if (begin_tree_node("Tesselation", triangulate)) {
		align("\a");
		connect_copy(add_button("save to obj")->click, rebind(this, &gl_implicit_surface_drawable::save_interactive));
		add_member_control(this, "triangulate", triangulate, "check");
		add_view("nr_vertices", nr_vertices);
		add_view("nr_faces", nr_faces);
		end_tree_node(triangulate);
		align("\b");
	}

	if (begin_tree_node("Volume Export", map_to_zero_value)) {
		align("\a");
		connect_copy(add_button("toggle range")->click, rebind(this, &gl_implicit_surface_drawable::toggle_range));
		connect_copy(add_button("adjust range")->click, rebind(this, &gl_implicit_surface_drawable::adjust_range));
		add_member_control(this, "map to zero", map_to_zero_value, "value_slider");
		add_member_control(this, "map to one", map_to_one_value, "value_slider");
		connect_copy(add_button("save to vox")->click, rebind(this, &gl_implicit_surface_drawable::export_volume));
		end_tree_node(map_to_zero_value);
		align("\b");
	}

	if (begin_tree_node("Contouring", contouring_type)) {
		align("\a");
		add_member_control(this, "normal computation", normal_computation_type, "gradient,face,corner,corner_gradient");
		add_member_control(this, "gradient normals", show_gradient_normals, "check");
		add_member_control(this, "mesh normals", show_mesh_normals, "check");
		add_member_control(this, "threshold", normal_threshold, "value_slider", "min=-1;max=1;ticks=true");
		add_member_control(this, "contouring", contouring_type, "dropdown", "enums='marching cubes,dual contouring'");
		add_member_control(this, "consistency_threshold", consistency_threshold, "value_slider", "min=0.00001;max=1;log=true;ticks=true");
		add_member_control(this, "max_nr_iters", max_nr_iters, "value_slider", "min=1;max=20;ticks=true");
		add_member_control(this, "res", res, "value_slider", "min=4;max=100;log=true;ticks=true");
		add_member_control(this, "epsilon", epsilon, "value_slider", "min=0;max=0.001;log=true;ticks=true");
		add_member_control(this, "grid_epsilon", grid_epsilon, "value_slider", "min=0;max=0.5;log=true;ticks=true");
		end_tree_node(contouring_type);
		align("\b");
	}

	if (begin_tree_node("Visualization", show_wireframe)) {
		align("\a");
		add_member_control(this, "show_&wireframe", show_wireframe, "check", "shortcut='W'");
		//add_member_control(this, "ambient", material.ref_ambient(), "color<float,RGBA>");
		static bool surface_mat_gui;
		if (begin_tree_node("Surface Material", surface_mat_gui)) {
			align("\a");
			add_gui("material", material);
			end_tree_node(surface_mat_gui);
			align("\b");
		}
		static bool box_mat_gui;
		if (begin_tree_node("Box Material", box_mat_gui)) {
			align("\a");
			add_gui("material", brs.material);
			end_tree_node(box_mat_gui);
			align("\b");
		}
		add_member_control(this, "&box", show_box, "check", "w=50;shortcut='b'", " ");
		add_member_control(this, "scale", box_scale, "value_slider", "w=120;min=0.01;max=100;ticks=true;log=true");
		add_gui("box", box, "",	"options='w=100;min=-10;max=10;step=0.01;ticks=true;align=\"BL\"';align_col=' '"); align("\n");
		add_member_control(this, "sampling_&grid", show_sampling_grid, "check", "shortcut='g'");
		add_member_control(this, "sampling_&points", show_sampling_locations, "check", "shortcut='p'");
		add_member_control(this, "mini_box", show_mini_box, "check");
		add_member_control(this, "ix",ix,"value_slider", "min=0;max=10;ticks=true");
		add_member_control(this, "iy",iy,"value_slider", "min=0;max=10;ticks=true");
		add_member_control(this, "iz",iz,"value_slider", "min=0;max=10;ticks=true");
		end_tree_node(show_wireframe);
		align("\b");
	}
}

bool gl_implicit_surface_drawable::self_reflect(cgv::reflect::reflection_handler& rh)
{
	return 
//		rh.reflect_member("box", box) &&
	//	rh.reflect_member("contouring_type", contouring_type) &&
		rh.reflect_member("normal_threshold", normal_threshold) &&
		rh.reflect_member("consistency_threshold", consistency_threshold) &&
		rh.reflect_member("max_nr_iters", max_nr_iters) &&
//		rh.reflect_member("normal_computation_type", normal_computation_type) &&
		rh.reflect_member("ix", ix) &&
		rh.reflect_member("iy", iy) &&
		rh.reflect_member("iz", iz) &&
		rh.reflect_member("show_wireframe", show_wireframe) &&
		rh.reflect_member("show_sampling_grid", show_sampling_grid) &&
		rh.reflect_member("show_sampling_locations", show_sampling_locations) &&
		rh.reflect_member("show_box", show_box) &&
		rh.reflect_member("show_mini_box", show_mini_box) &&
		rh.reflect_member("show_gradient_normals", show_gradient_normals) &&
		rh.reflect_member("show_mesh_normals", show_mesh_normals) &&
		rh.reflect_member("epsilon", epsilon) &&
		rh.reflect_member("grid_epsilon", grid_epsilon) &&
		rh.reflect_member("material_roughness", material.ref_roughness());
}

template <class T>
bool in_member (void *ptr, const T& member) {
	return ((char*)ptr)-((char*)&member) < sizeof(T);
}

void gl_implicit_surface_drawable::on_set(void* p)
{
	if (p == &box_scale) {
		if (find_control(box.ref_min_pnt()[0])) {
			for (int i = 0; i < 3; ++i) {
				find_control(box.ref_min_pnt()[i])->set("min", -box_scale);
				find_control(box.ref_min_pnt()[i])->set("max", box_scale);
				find_control(box.ref_max_pnt()[i])->set("min", -box_scale);
				find_control(box.ref_max_pnt()[i])->set("max", box_scale);
			}
		}
	}
	if (p == &res)
		resolution_change();
	else if (p == &contouring_type || p == &res || p == &normal_threshold || p == &consistency_threshold || 
		 p == &max_nr_iters || p == &normal_computation_type || p == &epsilon ||
		 p == &grid_epsilon || (p >= &box && p < &box+1) )
		   post_rebuild();
	else if (p == &ix || p == &iy || p == &iz || p == &show_wireframe || p == &show_sampling_grid ||
	    p == &show_sampling_locations || p == &show_box || p == &show_mini_box || 
		 p == &show_gradient_normals || p == &show_mesh_normals)
			post_redraw();
	else if (in_member(p, material) || in_member(p, brs.material))
		post_redraw();

	update_member(p);
}
