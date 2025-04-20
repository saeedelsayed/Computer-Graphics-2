#pragma once

#include <cgv/base/base.h>
#include <cgv/media/color.h>
#include <cgv/gui/provider.h>
#include <cgv/render/drawable.h>

using namespace cgv::base;
using namespace cgv::math;
using namespace cgv::render;
using namespace cgv::gui;

#define sqr(x) ((x)*(x))

template <typename T>
class implicit_group;

struct scene_update_handler
{
	virtual void update_scene() = 0;
	virtual void update_description() = 0;
};


/** base implementation for all group nodes*/
template <typename T>
class implicit_base : 
	public drawable,
	public provider
{
public:
	/// coordinate type
	typedef T crd_type;
	/// type of primitive color
	typedef cgv::media::color<float, cgv::media::RGB, cgv::media::OPACITY> clr_type;
	/// type of 3d vector
	typedef cgv::dvec3 vec_type;
	/// type of 3d point
	typedef cgv::dvec3 pnt_type;

protected:
	scene_update_handler * update_handler;
	/// to be called if scene has changed due to gui interaction
	void update_scene();
	/// callback for functions that update the scene description without the implicit function
	void update_description();
	/// color of implicit primitive
	clr_type color;
	/// gui color
	int gui_color;
	/// give group access to color and gui_color
	friend class implicit_group<T>;

public:
	/// set new scene update handler
	virtual void set_update_handler(scene_update_handler* uh);
	/// constructor sets default gui color
	implicit_base();
	/// returns "implicit_primitive"
	std::string get_type_name() const;
	/// convert to cgv::base::base pointer
	virtual cgv::base::base* get_base() = 0;
	/// interface for evaluation of implicit function
	virtual crd_type evaluate(const pnt_type& p) const = 0;
	/// interface for evaluation of the gradient with central differences based default implementation
	virtual vec_type evaluate_gradient(const pnt_type& p) const;
	/// interface for the evaluation of surface color
	virtual clr_type evaluate_color(const pnt_type& p) const;
};


struct abst_scene_factory
{
	std::string names;
	virtual void init_counter() = 0;
	virtual base_ptr create_function() = 0;
};

extern void register_scene_factory(abst_scene_factory* _scene_factory);

template <typename T>
struct scene_factory : public abst_scene_factory
{
	static unsigned int& ref_counter() {
		static unsigned int counter = 1;
		return counter;
	}
	static std::string& ref_base_name() {
		static std::string base_name;
		return base_name;
	}
	scene_factory(const std::string& _names, const std::string& base_name = "") {
		names = _names;
		ref_base_name() = base_name;
		if (base_name.empty()) {
			T dummy;
			ref_base_name() = dummy.get_base()->get_type_name();
		}
	}
	void init_counter() {
		ref_counter() = 1;
	}
	base_ptr create_function() {
		T* f = new T;
		f->set_name(ref_base_name() + "_" + cgv::utils::to_string(ref_counter()));
		++ref_counter();
		return f;
	};
};

/** use this registration struct to register a factory for your
implementation of an implicit function.
*/
template <typename T>
struct scene_factory_registration
{
	scene_factory_registration(const std::string& _names, const std::string& _base_name = "") {
		register_scene_factory(new scene_factory<T>(_names, _base_name));
	}
};
