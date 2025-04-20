#include "implicit_primitive.h"
#include <cgv_reflect_types/media/color.h>


template <typename T>
void implicit_primitive<T>::on_set(void* member_ptr)
{
	implicit_base<T>::update_scene();
	provider::update_member(member_ptr);
}

/// reflect members to expose them to serialization
template <typename T>
bool implicit_primitive<T>::self_reflect(cgv::reflect::reflection_handler& rh)
{
	return
		rh.reflect_member("cr", implicit_base<T>::color[0]) &&
		rh.reflect_member("cg", implicit_base<T>::color[1]) &&
		rh.reflect_member("cb", implicit_base<T>::color[2]) &&
		rh.reflect_member("ca", implicit_base<T>::color[3]);
}

template <typename T>
std::string implicit_primitive<T>::get_type_name() const
{
	return "implicit_primitive";
}

template <typename T>
void implicit_primitive<T>::create_gui()
{
}

template class implicit_primitive<double>;
