#pragma once

#include "implicit_base.h"
#include <cgv/gui/text_editor.h>
#include "gl_implicit_surface_drawable.h"

///
class scene :
	public group,
	public gl_implicit_surface_drawable::F,
	public scene_update_handler,
	public drawable,
	public provider,
	public text_editor_callback_handler
{
private:
	bool disable_update;
	bool help_shown;
	void construct_editor();
protected:
	void show_help();
	/// store registered scene factories in a vector
	std::vector<abst_scene_factory*> factories;
	/// store the name of the current scene description file
	std::string file_name;
	/// store a pointer to the text editor
	cgv::gui::text_editor_ptr editor;
	// pass on property interface to text editor
	std::string get_property_declarations();
	bool set_void(const std::string& property, const std::string& value_type, const void* value_ptr);
	bool get_void(const std::string& property, const std::string& value_type, void* value_ptr);
	/// handle text changed events of text editor
	void text_changed(const char* _text, int length);
	/// called when new file has been read
	void after_read();
	/// called when text has been saved
	void after_save();
	/// if the editor is closed, it needs to be deleted
	void on_close_editor();
	/// rebuild gui
	void on_update_callback();
	/// called when nr_inserted characters have been inserted at text_pos
	void on_text_insertion(int text_pos, int nr_inserted);
	/// called when the nr_deleted characters in deleted_text have been deleted at text position text_pos
	void on_text_deletion(int text_pos, int nr_deleted, const char* deleted_text);
	/// update the style starting from text_pos. The text is unchanged after text_pos + min_nr_checked. Return the number of changed style characters. The default implementation does nothing, such that style A is kept for all characters and returns 0
	void update_style(int text_pos, int min_nr_checked);
public:
	/// type of implicits
	typedef implicit_base<double> implicit_type;
	/// pointer to implicit surface drawable
	gl_implicit_surface_drawable_ptr impl_draw_ptr;
	/// pointer to current function
	base_ptr func_base_ptr;
	/// current scene description
	std::string description;

	std::string get_changed_values(implicit_type* fp, implicit_type* fp_ref) const;
	void reconstruct_description();
	std::string reconstruct_description_recursive(unsigned int& i, implicit_type* func_ptr, group* g);
	/// check if factory's symbol[s] match location i in description
	bool symbol_matches_description(unsigned int i, abst_scene_factory* factory, unsigned int& offset) const;
	/// recursive part of the scene description parsing
	base_ptr parse_description_recursive(unsigned int& i, group* g);
	/// parse a scene description and construct a function pointer
	void parse_description();
	/// callback for functions that update the scene based on gui interaction
	void update_scene();
	/// callback for functions that update the scene description without the implicit function
	void update_description();
	/// registration of scene factories;
	void register_factory(abst_scene_factory* _scene_factory);
	/// construct scene from a description string
	scene(const std::string& _description = "S");
	/// called to unregister derived guis and drawables
	void unregister();
	/// overload to return the type name of this object
	std::string get_type_name() const;
	///
	void create_gui();
	/// cast evaluation to func_base_ptr
	double evaluate(const pnt_type& p) const;
	/// cast gradient evaluation to func_base_ptr
	vec_type evaluate_gradient(const pnt_type& p) const;
};

/// ref counted pointer to a scene
typedef cgv::data::ref_ptr<scene> scene_ptr;
