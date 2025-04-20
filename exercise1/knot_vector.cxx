#include "knot_vector.h"
#include <cgv/utils/scan.h>
#include <cgv_gl/gl/gl.h>


template <typename T> 
void knot_vector<T>::append_point(const pnt_type& p)
{
	pnt_idx = int(points.size());

	points.push_back(p);
	if (provider::find_control(pnt_idx))
		provider::find_control(pnt_idx)->set("max", pnt_idx);

	append_callback(pnt_idx);
	on_set(&pnt_idx);
}

	/// construct with empty skeleton
template <typename T>
knot_vector<T>::knot_vector() : p(0, 0, 0)
{
	pnt_idx = 0;
}

/// reflect members to expose them to serialization
template <typename T>
bool knot_vector<T>::self_reflect(cgv::reflect::reflection_handler& rh)
{
	int n = (int)points.size(); // reflection is currently wonky with unsigned types

	// reflect statically allocated members
	bool do_continue =
		rh.reflect_member("n", n) &&
		implicit_primitive<T>::self_reflect(rh);

	// ensure dynamic allocation of points
	if (rh.is_creative()) {
		while (points.size() < (size_t)n)
			append_point(pnt_type(0, 0, 0));
		while (points.size() > (size_t)n)
			points.pop_back();
	}

	// check whether to termine self reflection
	if (!do_continue)
		return false;

	// reflect all point positions
	for (size_t i = 0; i < points.size(); ++i) {
		if (!rh.reflect_member(std::string("x") + cgv::utils::to_string(i), points[i].x()))
			return false;
		if (!rh.reflect_member(std::string("y") + cgv::utils::to_string(i), points[i].y()))
			return false;
		if (!rh.reflect_member(std::string("z") + cgv::utils::to_string(i), points[i].z()))
			return false;
	}

	return true;
}
	
/// implementation of updates needed after members changed
template <typename T>
void knot_vector<T>::on_set(void* member_ptr)
{
	if (member_ptr == &pnt_idx) {
		p = points[pnt_idx];
		provider::update_member(&p(0));
		provider::update_member(&p(1));
		provider::update_member(&p(2));
		point_index_selection_callback();
	}
	for (unsigned c = 0; c < 3; ++c) {
		if (member_ptr == &p(c)) {
			points[pnt_idx](c) = p(c);
			position_changed_callback(pnt_idx);
		}
		for (size_t i = 0; i < points.size(); ++i) {
			if (member_ptr == &points[i](c)) {
				if (i == pnt_idx) {
					p(c) = points[i](c);
					provider::update_member(&p(c));
					position_changed_callback(i);
				}
			}
		}
	}
	provider::update_member(member_ptr);
	implicit_base<T>::update_scene();
}

template <typename T>
void knot_vector<T>::create_gui()
{
	if (points.empty()) {
		append_point(pnt_type(0, 0, 0));
		append_point(pnt_type(1, 0, 0));
	}
	provider::add_member_control(this, "pnt_idx", pnt_idx, "value_slider", "min=0;max=1;ticks=true");
	provider::find_control(pnt_idx)->set("max", (int)points.size() - 1);

	connect_copy(provider::add_button("append")->click,
		cgv::signal::rebind(this, &knot_vector<T>::append_point, cgv::signal::_r(p)));

	provider::add_member_control(this, "x", p(0), "value_slider", "min=-5;max=5;ticks=true");
	provider::add_member_control(this, "y", p(1), "value_slider", "min=-5;max=5;ticks=true");
	provider::add_member_control(this, "z", p(2), "value_slider", "min=-5;max=5;ticks=true");
}

template class knot_vector<double>;
