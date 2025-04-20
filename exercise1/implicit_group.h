#pragma once

#include "implicit_base.h"
#include <cgv/base/group.h>

using namespace cgv::base;

enum GroupColorMode
{
	GCM_REPLACE,
	GCM_COMPOSE,
	GCM_CHILD_0,
	GCM_CHILD_1,
	GCM_CHILD_2
};

/** base implementation for all group nodes*/
template <typename T>
class implicit_group : public group, public implicit_base<T>
{
public:
	typedef typename implicit_base<T>::clr_type clr_type;
	typedef typename implicit_base<T>::vec_type vec_type;
	typedef typename implicit_base<T>::pnt_type pnt_type;
protected:
	/// access to implicit base interface of children
	implicit_base<T>* get_implicit_child(unsigned i);
	/// const access to implicit base interface of children
	const implicit_base<T>* get_implicit_child(unsigned i) const;
	/// store for each child a flag whether the child is visible in the gui
	std::vector<int> child_visible_in_gui;
	/// the way the color is computed
	GroupColorMode color_mode;
	/// overload to compose the colors of the function children
	virtual clr_type compose_color(const pnt_type& p) const;
public:
	/// convert to cgv::base::base pointer
	cgv::base::base* get_base() { return this; }
	/// reflect members to expose them to serialization
	bool self_reflect(cgv::reflect::reflection_handler& rh);
	/// calls the update_scene method of scene_updater
	void on_set(void* member_ptr);
	/// append a new child and extract trivariate function
	unsigned int append_child(base_ptr child);
	/// returns "implicit_group"
	std::string get_type_name() const;
	/// evaluation of surface color based on color_mode
	clr_type evaluate_color(const pnt_type& p) const;
	/// passes on init to the children
	bool init(context&);
	/// passes on the update handler to the children
	void set_update_handler(scene_update_handler* uh);
	/// create gui of children. Call this inside implementations of create_gui of derived classes.
	void create_gui();
};
