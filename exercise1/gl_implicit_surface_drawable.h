#pragma once

#include <cgv_gl/gl/gl_implicit_surface_drawable_base.h>
#include <cgv/base/base.h>
#include <cgv/gui/provider.h>

/** drawable that visualizes implicit surfaces by contouring them with marching cubes or
    dual contouring. */
class gl_implicit_surface_drawable : 
	public cgv::base::base, 
	public cgv::gui::provider,
	public cgv::render::gl::gl_implicit_surface_drawable_base
{
public:
	typedef cgv::math::fvec<double, 3> vec_type;
	typedef cgv::math::fvec<double, 3> pnt_type;
private:
	float box_scale;
protected:
	double map_to_zero_value;
	double map_to_one_value;
	void toggle_range();
	void adjust_range();
	void export_volume();

	void save_interactive();
	void resolution_change();
	void surface_extraction();
	void build_display_list();
public:
	/// standard constructor does not initialize the function pointer so that nothing is drawn
	gl_implicit_surface_drawable();
	void on_set(void* member_ptr);
	bool self_reflect(cgv::reflect::reflection_handler& rh);
	std::string get_type_name() const;
	void create_gui();
};

typedef cgv::data::ref_ptr<gl_implicit_surface_drawable> gl_implicit_surface_drawable_ptr;
