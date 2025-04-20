#pragma once

#include "node.h"
#include <vector>

#include "lib_begin.h"

namespace cgv {
	namespace base {

class CGV_API group;

/// ref counted pointer to a node
typedef data::ref_ptr<group, true> group_ptr;
/// ref counted pointer to a node
typedef data::ref_ptr<const group, true> const_group_ptr;

/** The group class is a node with children. */
class CGV_API group : public node
{
protected:
	/// declare group_ptr to be a friend class
	friend class data::ref_ptr<group,true>;
	/// store a list of children
	std::vector<base_ptr> children;
	/// check if the base class is a node and set the parent of the node
	void link(base_ptr b);
	/// check if the base class is a node and clear the parent of the node
	void unlink(base_ptr b);
public:
	/// construct from name
	group(const std::string& name = "");
	/// return the number of children
	unsigned int get_nr_children() const;
	/// return the i-th child
	base_ptr get_child(unsigned int i) const;
	/// create and append an instance of a child node and return pointer to appended child; optionally set name and get index
	template<typename T>
	data::ref_ptr<T> create_and_append_child(const std::string& name = "", unsigned int* index = nullptr) {
		static_assert(std::is_base_of<node, T>::value, "T must inherit from node");
		data::ref_ptr<T> ptr(new T());
		ptr->set_name(name);
		unsigned int i = append_child(ptr);
		if(index)
			*index = i;
		return ptr;
	}
	/// append child and return index of appended child
	virtual unsigned int append_child(base_ptr child);
	/// remove all elements of the vector that point to child, return the number of removed children
	virtual unsigned int remove_child(base_ptr child);
	/// remove all children
	virtual void remove_all_children();
	/// insert a child at the given position
	virtual void insert_child(unsigned int i, base_ptr child);
	/// cast upward to group
	group_ptr get_group();
	/// cast upward to const group
	const_group_ptr get_group_const();
	/// overload to return the type name of this object
	std::string get_type_name() const;
};

template <> struct cast_helper<group>
{
	inline static group_ptr cast(base* b) { return b->get_group(); }
};
template <> struct cast_const_helper<group>
{
	inline static const_group_ptr cast_const(const base* b) { return b->get_group_const(); }
};


#if _MSC_VER >= 1400
CGV_TEMPLATE template class CGV_API data::ref_ptr<group>;
CGV_TEMPLATE template class CGV_API std::vector<base_ptr>;
#endif

	}
}

#include <cgv/config/lib_end.h>
