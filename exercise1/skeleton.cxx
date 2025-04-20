#include "skeleton.h"
#include <cgv_gl/gl/gl.h>


// append an edge to the sketelon
template <typename T>
void skeleton<T>::append_edge(const edge_type& edge)
{
	edge_idx = (unsigned)edges.size();

	edges.push_back(edge);
	if (provider::find_control(edge_idx))
		provider::find_control(edge_idx)->set("max", edge_idx);

	append_edge_callback(edge_idx);
	on_set(&edge_idx);
}

/// construct with empty skeleton
template <typename T>
skeleton<T>::skeleton() : edge(0,1) {
	show_edges = true;
	edge_width = 3;
	edge_idx   = 0;
}
/// reflect members to expose them to serialization
template <typename T>
bool skeleton<T>::self_reflect(cgv::reflect::reflection_handler& rh)
{
	int m = (int)edges.size();

	// reflect statically allocated members
	bool do_continue =
		knot_vector<T>::self_reflect(rh) &&
		rh.reflect_member("m", m) &&
		rh.reflect_member("show_edges", show_edges) &&
		rh.reflect_member("edge_width", edge_width);

	// ensure dynamic allocation of edges
	if (rh.is_creative()) {
		while (edges.size() < (size_t)m)
			append_edge(edge_type(0, 1));
		while (edges.size() > (size_t)m)
			edges.pop_back();
	}

	// check whether to termine self reflection
	if (!do_continue)
		return false;

	// reflect all edge indices
	for (size_t i = 0; i < edges.size(); ++i) {
		if (!rh.reflect_member(std::string("i") + cgv::utils::to_string(i), edges[i].first))
			return false;
		if (!rh.reflect_member(std::string("j") + cgv::utils::to_string(i), edges[i].second))
			return false;
	}

	return true;
}
	
/// implementation of updates needed after members changed
template <typename T>
void skeleton<T>::on_set(void* member_ptr) {
	if (member_ptr == &edge_idx) {
		edge = edges[edge_idx];
		provider::update_member(&edge.first);
		provider::update_member(&edge.second);
		edge_index_selection_callback();
	}

	if (member_ptr == &edge.first) {
		edges[edge_idx].first = edge.first;
		edge_changed_callback(edge_idx);
	}
	if (member_ptr == &edge.second) {
		edges[edge_idx].second = edge.second;
		edge_changed_callback(edge_idx);
	}
	for (size_t i = 0; i < edges.size(); ++i) {
		if (member_ptr == &edges[i].first) {
			if (i == edge_idx) {
				edge.first = edges[i].first;
				provider::update_member(&edge.first);
			}
			edge_changed_callback(i);
		}
		if (member_ptr == &edges[i].second) {
			if (i == edge_idx) {
				edge.second = edges[i].second;
				provider::update_member(&edge.second);
			}
			edge_changed_callback(i);
		}
	}

	drawable::post_redraw();

	if (member_ptr == &show_edges || member_ptr == &edge_width) {
		provider::update_member(member_ptr);
		return;
	}
	knot_vector<T>::on_set(member_ptr);
}
/// its a drawable
template <typename T>
void skeleton<T>::draw(context& ctx)
{
	if (!show_edges)
		return;
	// Fixme: legacy OpenGL. Not good.
	glLineWidth((float)edge_width);
	glDisable(GL_LIGHTING);
	glColor3d(0.25,0.25,0.25);
	glBegin(GL_LINES);
		for (size_t i=0; i<edges.size(); i++) {
			if (((size_t)edges[i].first) >= (knot_vector<T>::points).size() ||
			    ((size_t)edges[i].second) >= (knot_vector<T>::points).size())
				continue;
			glVertex3dv((knot_vector<T>::points)[edges[i].first].data());
			glVertex3dv((knot_vector<T>::points)[edges[i].second].data());
		}
	glEnd();
	glEnable(GL_LIGHTING);
	glLineWidth(1);
}

template <typename T>
void skeleton<T>::append_callback(size_t pi)
{
	if (provider::find_control(edge.first)) {
		provider::find_control(edge.first)->set("max", pi);
		provider::find_control(edge.second)->set("max", pi);
	}
}

template <typename T>
void skeleton<T>::create_gui()
{
	knot_vector<T>::create_gui();
	if (edges.empty()) {
		append_edge(edge_type(0, 1));
	}
	provider::add_member_control(this, "show_edges", show_edges, "check");
	provider::add_member_control(this, "edge_width", edge_width, "value_slider", "min=1;max=20;log=true;ticks=true");
	provider::add_member_control(this, "edge_idx", edge_idx, "value_slider", "min=0;max=1;ticks=true");
	provider::find_control(edge_idx)->set("max",(int)edges.size()-1);

	connect_copy(provider::add_button("append")->click,
		cgv::signal::rebind(this, &skeleton<T>::append_edge, cgv::signal::_r(edge)));

	provider::add_member_control(this, "i", edge.first, "value_slider", "min=0;max=5;ticks=true");
	provider::add_member_control(this, "j", edge.second, "value_slider", "min=0;max=5;ticks=true");
	provider::find_control(edge.first)->set("max", (knot_vector<T>::points).size()-1);
	provider::find_control(edge.second)->set("max", (knot_vector<T>::points).size()-1);
}

template class skeleton<double>;
