#pragma once

#include "knot_vector.h"

template <typename T>
class skeleton : public knot_vector<T>
{
public:
	typedef typename knot_vector<T>::vec_type vec_type;
	typedef typename knot_vector<T>::pnt_type pnt_type;
	/// edge is a pair of indices (use int for now as Framework reflection is currently still wonky
	/// with unsigned types)
	typedef std::pair<int, int> edge_type;

protected:
	/// append an edge to the sketelon
	void append_edge(const edge_type& edge);
	/// list of all edges in the skeleton
	std::vector<edge_type> edges;
	/// index of edge that can currently be edited in user interface (selectable via slider control)
	unsigned int edge_idx;

private:
	/// proxy variable used for editing edges[edge_idx] in the user interface
	edge_type edge;

protected:
	// virtual functions: can be overwritten by derived classes to process the data
	/// triggers when an edge is appended. Called by append_edge
	virtual void append_edge_callback(size_t ei) {}
	/// triggers when an edge connectivity (i.e. one of its two indices) changed. Called by on_set
	virtual void edge_changed_callback(size_t ei) {}
	/// triggers when the edge selected in the gui for editing changed. Called by on_set
	virtual void edge_index_selection_callback() {}
	/// render the skeleton edges in the 3D view
	bool show_edges;
	/// line width for rendering the skeleton edges
	float edge_width;

public:
	/// construct with empty skeleton
	skeleton();
	/// overload to return the type name of this object
	std::string get_type_name() const { return "skeleton"; }
	/// reflect members to expose them to serialization
	bool self_reflect(cgv::reflect::reflection_handler& rh);
	/// implementation of updates needed after members changed
	void on_set(void* member_ptr);
	/// its a drawable
	void draw(cgv::render::context& ctx);
	/// overload append point callback to configure gui of current edge
	void append_callback(size_t pi);
	/// extend knot vector gui with support for appending and editing edges
	void create_gui();
};
