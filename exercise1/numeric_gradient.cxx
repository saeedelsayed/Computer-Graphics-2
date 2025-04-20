#include "implicit_group.h"

/// superimposes a numerical gradient evaluation over its children (can be bypassed by
/// setting ::numerical accordingly)
template <typename T>
class numeric_gradient : public implicit_group<T>
{
public:
	typedef typename implicit_base<T>::vec_type vec_type;
	typedef typename implicit_base<T>::pnt_type pnt_type;

protected:
	/// store the numeric_gradient
	T epsilon;
	/// whether to compute numerically the gradient
	bool numerical;
public:
	/// return type name
	std::string get_type_name() const { return "numeric_gradient"; }
	/// initialize level to 0
	numeric_gradient() : epsilon(1e-6), numerical(false) { implicit_base<T>::gui_color = 0x00FFFF; }
	///
	bool self_reflect(cgv::reflect::reflection_handler& rh)
	{
		return
			rh.reflect_member("epsilon", epsilon) &&
			rh.reflect_member("numerical", numerical) &&
			implicit_group<T>::self_reflect(rh);
	}
	T evaluate(const pnt_type& p) const {
		if (group::get_nr_children() == 0)
			return 1;
		return implicit_group<T>::get_implicit_child(0)->evaluate(p);
	}
	vec_type evaluate_gradient(const pnt_type& p) const {
		if (group::get_nr_children() == 0)
			return vec_type(0,0,0);
		if (numerical) {
			T inv_2_eps = (T)(0.5/epsilon);
			return vec_type(
				inv_2_eps*(evaluate(pnt_type(p(0)+epsilon,p(1),p(2))) - 
				           evaluate(pnt_type(p(0)-epsilon,p(1),p(2)))),
				inv_2_eps*(evaluate(pnt_type(p(0),p(1)+epsilon,p(2))) - 
				           evaluate(pnt_type(p(0),p(1)-epsilon,p(2)))),
				inv_2_eps*(evaluate(pnt_type(p(0),p(1),p(2)+epsilon)) - 
				           evaluate(pnt_type(p(0),p(1),p(2)-epsilon))));
		}
		return implicit_group<T>::get_implicit_child(0)->evaluate_gradient(p);
	}
	void create_gui()
	{
		provider::add_member_control(this, "epsilon", epsilon, "value_slider", "min=0.000000001;max=0.1;step=0.000000001;ticks=true;log=true");
		provider::add_member_control(this, "numerical", numerical, "check");
		implicit_group<T>::create_gui();
	}
};

scene_factory_registration<numeric_gradient<double> >sfr_numeric_gradient("numeric_gradient");
