#pragma once

#include "implicit_primitive.h"

/** the knot_vector class manages a vector of knot locations and provides callbacks
	in case that points are changed or appended. This is the base class for meta balls
	and skeletons.*/
template <typename T>
class knot_vector : public implicit_primitive<T>
{
public:
	typedef typename implicit_base<T>::vec_type vec_type;
	typedef typename implicit_base<T>::pnt_type pnt_type;

protected:
	// vector of knot locations
	std::vector<pnt_type> points;

	/// append a point to the knot vector and call the append callback
	void append_point(const pnt_type& p);

	// virtual functions: can be overwritten by derived classes to process the data
	/// triggers when a point is appended. Called by append_point
	virtual void append_callback(size_t pi) {}
	/// triggers when the position of a point changed. Called by on_set
	virtual void position_changed_callback(size_t pi) {}
	/// triggers when the point selected in the gui for editing changed. Called by on_set
	virtual void point_index_selection_callback() {}

	/// index of point that can currently be edited in user interface (selectable via slider control)
	unsigned int pnt_idx;

private:
	/// proxy variable used for editing points[pnt_idx] in the user interface
	pnt_type p;

public:
	/// construct with empty skeleton
	knot_vector();
	/// overload to return the type name of this object
	std::string get_type_name() const { return "knot_vector"; }
	/// reflect members to expose them to serialization
	bool self_reflect(cgv::reflect::reflection_handler& rh);
	/// implementation of updates needed after members changed
	void on_set(void* member_ptr);
	/// create gui to edit the points in the knot vector and to allow appending a new point
	void create_gui();
};
