#include "scene.h"
#include "implicit_group.h"
#include <cgv/signal/rebind.h>
#include <cgv/base/group.h>
#include <cgv/gui/gui_driver.h>
#include <cgv/utils/tokenizer.h>
#include <cgv/utils/advanced_scan.h>
#include <cgv/type/variant.h>
#include <cgv/math/qem.h>
#include <cgv/base/register.h>
#include <cgv/utils/convert_string.h>

using namespace cgv::media::font;

#define NR_TEXT_STYLES 8

static text_style* get_text_style_table() {
	static text_style style_table[NR_TEXT_STYLES] = {
		{ 0x000000, find_font("Courier")->get_font_face(FFA_REGULAR), 12 }, 
		{ 0x00009f, find_font("Courier")->get_font_face(FFA_BOLD), 12 }, 
		{ 0xaf0000, find_font("Courier")->get_font_face(FFA_BOLD), 12 }, 
		{ 0x7f007f, find_font("Courier")->get_font_face(FFA_BOLD), 12 }, 
		{ 0x108020, find_font("Courier")->get_font_face(FFA_REGULAR), 12 },
		{ 0xaf0000, find_font("Courier")->get_font_face(FFA_BOLD), 12 },
		{ 0x777777, find_font("Courier")->get_font_face(FFA_REGULAR), 12 },
		{ 0x007777, find_font("Courier")->get_font_face(FFA_REGULAR), 12 }
	};
	return style_table;
}

using namespace cgv::math;
using namespace cgv::type;
using namespace cgv::gui;
using namespace cgv::signal;
using namespace cgv::utils;

/*
void qem_test()
{
	qem<double> Qx(vec<double>(0,0,0), normalize(vec<double>(1,1,0)));
	qem<double> Qy(vec<double>(0,1,0), normalize(vec<double>(0,1,1)));
	qem<double> Qz(vec<double>(0,0,2), normalize(vec<double>(1,0,1)));
	qem<double> Q = Qx;
	Q += Qy;
	Q += Qz;
	vec<double> p = Q.compute_minimum(vec<double>(3,3,3));
	std::cout << "minimum point at " << p 
		       << " with value " << Q.evaluate(p) << std::endl;
}*/

scene_ptr ref_scene()
{
	static scene_ptr sp(new scene("+(T[],r[](T[]))"));
	return sp;
}

void register_scene_factory(abst_scene_factory* _scene_factory)
{
	base_ptr bp = _scene_factory->create_function();
	std::cout << "register '" << _scene_factory->names << "' to " << bp->get_type_name() 
		      << "[" << bp->get_property_declarations() << "]" << std::endl;
	ref_scene()->register_factory(_scene_factory);
}

void scene::register_factory(abst_scene_factory* _scene_factory)
{
	factories.push_back(_scene_factory);
}

std::string scene::get_property_declarations()
{
	if (editor)
		return editor->get_property_declarations()+"file_name:string";
	return "file_name:string";
}

bool scene::set_void(const std::string& property, const std::string& value_type, const void* value_ptr)
{
	if (property == "file_name") {
		file_name = variant<std::string>::get(value_type, value_ptr);
		if (editor)
			editor->read(file_name);
		return true;
	}
	if (!editor)
		return false;
	return editor->set_void(property,value_type, value_ptr);
}
bool scene::get_void(const std::string& property, const std::string& value_type, void* value_ptr)
{
	if (property == "file_name") {
		set_variant(file_name,value_type, value_ptr);
		return true;
	}
	if (!editor)
		return false;
	return editor->set_void(property,value_type, value_ptr);
}
/// handle text changed events of text editor
void scene::text_changed(const char* _text, int length)
{
}
/// called when new file has been read
void scene::after_read()
{
	parse_description();
}

/// called when text has been saved
void scene::after_save()
{
	parse_description();
}

/// rebuild gui
void scene::on_update_callback()
{
	parse_description();
}

/// if the editor is closed, it needs to be deleted
void scene::on_close_editor()
{
	editor.clear();
}

/// called when nr_inserted characters have been inserted at text_pos
void scene::on_text_insertion(int text_pos, int nr_inserted)
{
	text_changed(editor->get_text(), editor->get_length());
	update_style(text_pos, nr_inserted);
}

/// called when the nr_deleted characters in deleted_text have been deleted at text position text_pos
void scene::on_text_deletion(int text_pos, int nr_deleted, const char* deleted_text)
{
	text_changed(editor->get_text(), editor->get_length());
	update_style(text_pos, 0);
}

/// update the style starting from text_pos. The text is unchanged after text_pos + min_nr_checked. Return the number of changed style characters. The default implementation does nothing, such that style A is kept for all characters and returns 0
void scene::update_style(int text_pos, int min_nr_checked)
{
	const char* text = editor->get_text();
	int length = editor->get_length();
	if (min_nr_checked == 0 && text_pos > 0 && text_pos < length && text[text_pos] == '\n')
		--text_pos;
	int end_pos = text_pos+min_nr_checked;
	while (text_pos > 0 && text[text_pos] != '\n')
		--text_pos;
	while (end_pos < length && text[end_pos] != '\n')
		++end_pos;

	int n = end_pos-text_pos;		
	text += text_pos;
	length -= text_pos;
	bool in_string = false;
	int i = 0;
	if (editor->get_style()[text_pos] == 'H') {
		++i;
		in_string = true;
	}
	bool not_finished = true;
	while (not_finished) {
		std::string style(editor->get_style()+text_pos, n);
		for (; i<n; ++i) {
			if (in_string) {
				style[i] = 'H';
				if (text[i] == '"')
					in_string = false;
			}
			else {
				switch (text[i]) {
				case '[' :
				case ']' : style[i] = 'B'; break;
				case '{' :
				case '}' : style[i] = 'C'; break;
				case '(' :
				case ')' : style[i] = 'D'; break;
				case '0' :
				case '1' :
				case '2' :
				case '3' :
				case '4' :
				case '5' :
				case '6' :
				case '7' :
				case '8' :
				case '9' :
				case '.' : style[i] = 'E'; break;
				case ':' : style[i] = 'F'; break;
				case '%' : 
						for (; i<n; ++i) {
							style[i] = 'G';
							if (text[i] == '\n')
								break;
						}
						break;
				case '"' :
					style[i] = 'H';
					in_string = true;
					break;
				default:   style[i] = 'A'; break;
				}
			}
		}
		char c = editor->get_style()[text_pos+n];
		not_finished = n < length && ( (c == 'H') != in_string );
		editor->set_style(text_pos,n,style.c_str());
		if (not_finished) {
			text_pos += n;
			text += n;
			length -= n;
			n = length > 1000 ? 1000 : length;
			i = 0;
		}
	}
}

scene::scene(const std::string& _description) 
	: description(_description),
	  impl_draw_ptr(new gl_implicit_surface_drawable())
{
	name = "scene";

	disable_update = false;
	help_shown = false;
	register_object(impl_draw_ptr);
	impl_draw_ptr->set_function(this);
	if (cgv::gui::get_gui_driver())
		construct_editor();
	else {
		connect_copy(cgv::gui::on_gui_driver_registration(),rebind(this, &scene::construct_editor));
	}
}

void scene::construct_editor()
{
	editor = cgv::gui::create_text_editor(660,400,"textedit",500,500);
	editor->show();
	editor->set_callback_handler(this);
	editor->set_base_path(".");
	editor->set_filter("*.isd");
	editor->set_text_styles(get_text_style_table(), NR_TEXT_STYLES);
	editor->set_text(description.c_str());
	parse_description();
}

void scene::unregister()
{
	//if (func_base_ptr)
//		unregister_object(func_base_ptr, "");
	if (impl_draw_ptr)
		unregister_object(impl_draw_ptr, "");
}

void scene::parse_description()
{
	disable_update = true;
	if (editor) {
		description = editor->get_text();
	}

	for (unsigned int j=0; j<factories.size(); ++j)
		factories[j]->init_counter();

	if (func_base_ptr) {
		remove_all_children();
		func_base_ptr.clear();
	}
	unsigned int i=0;
	func_base_ptr = parse_description_recursive(i, 0);
	post_recreate_gui();
	post_redraw();
	if (func_base_ptr) {
		append_child(func_base_ptr);
		get_context()->make_current();
		get_context()->configure_new_child(func_base_ptr);
		impl_draw_ptr->set_function(this);
	}
	disable_update = false;
}

bool scene::symbol_matches_description(unsigned int i, abst_scene_factory* factory, unsigned int& offset) const
{
	if (factory->names.size() == 1) {
		if (description[i] == factory->names[0]) {
			offset = 1;
			return true;
		}
		return false;
	}
	std::vector<cgv::utils::token> tokens;
	cgv::utils::split_to_tokens(factory->names, tokens, ";,", false);
	for (unsigned j = 0; j < tokens.size(); ++j) {
		std::string symbol = to_string(tokens[j]);
		if (i + symbol.size() <= description.size())
			if (description.substr(i, symbol.size()) == symbol) {
				offset = (unsigned)symbol.size();
				return true;
			}
	}
	return false;
}

base_ptr scene::parse_description_recursive(unsigned int& i, group* g)
{
	base_ptr bp;
	std::string group_defs;
	while (i < (unsigned int)description.size()) {
		bool used_factory = false;
		unsigned int offset = 0;
		for (unsigned int j=0; j<factories.size(); ++j) {
			if (symbol_matches_description(i, factories[j], offset)) {
				bp = factories[j]->create_function();
				bp->get_interface<implicit_type>()->set_update_handler(this);
				used_factory = true;
				break;
			}
		}
		if (used_factory) {
			i += offset;
			group_defs = "";
			continue;
		}
		switch (description[i]) {
		case '[' :
			{
				unsigned int t = i+1;
				for (++i; i < description.size() && description[i] != ']'; ++i) {
				}
				std::string defs = description.substr(t,i-t);
				group_defs = defs;
				bp->multi_set(defs);
				break;
			}
		case '<' :
			{
				unsigned int t = i+1;
				for (++i; i < description.size() && description[i] != '>'; ++i) {
				}
				std::string name = description.substr(t,i-t);
				bp->get_named()->set_name(name);
				break;
			}
		case '(' :
			if (bp) {
				group* new_g = bp->get_interface<group>();
				if (new_g) {
					++i;
					parse_description_recursive(i, new_g);
					if (!group_defs.empty())
						new_g->multi_set(group_defs);
				}
			}
			break;
		case ',' :
			if (bp && g)
				g->append_child(bp);
			break;
		case ')' :
			if (bp && g)
				g->append_child(bp);
			return bp;
		case '%' : 
				for (; i<description.size(); ++i) {
					if (description[i] == '\n')
						break;
				}
				break;
		default:
			++i;
			continue;
		}
		++i;
	}
	return bp;
}

void scene::reconstruct_description()
{
	unsigned int i=0;
	std::string d = reconstruct_description_recursive(i, func_base_ptr->get_interface<implicit_type>(), 0);
	if (editor)
		editor->set_text(d);
	description = d;
}

void scene::show_help()
{
	std::cout << "HELP ON SCENE DESCRIPTIONS:" << std::endl;
	std::vector<char> symbols;
	unsigned int j;
	for (j=0; j<factories.size(); ++j) {
		std::cout << "   " << factories[j]->names;
		base_ptr bp = factories[j]->create_function();
		if (bp->get_group())
			std::cout << "()";
		else
			std::cout << "  ";
		std::cout << " ... ";
		std::cout << bp->get_type_name()
			      << "[" << bp->get_property_declarations() << "]" << std::endl;
	}
	std::cout << std::endl;
}


std::string scene::get_changed_values(implicit_type* fp, implicit_type* fp_ref) const
{
	base* bp = fp->get_base();
	base* bp_ref = fp_ref->get_base();

	std::string decs;
	std::string prop_decs = bp->get_property_declarations();
	std::vector<token> toks;
	bite_all(tokenizer(prop_decs).set_ws(";"), toks);
	for (unsigned int i=0; i<toks.size(); ++i) {
		std::vector<token> toks1;
		tokenizer(toks[i]).set_ws(":").bite_all(toks1);
		if (toks1.size() != 2) {
			std::cerr << "found invalid property declaration: >" 
						 << to_string(toks[i]).c_str() << "<" << std::endl;
			continue;
		}
		std::string v;
		std::string v1;
		if (!bp->get_void(to_string(toks1[0]), "string", &v)) {
			std::cerr << "property " << to_string(toks1[0]).c_str() << " not found" << std::endl; 
			continue;
		}
		if (bp_ref) {
			if (!bp_ref->get_void(to_string(toks1[0]), "string", &v1)) {
				v1 = "";
				// hard coded special case of flags whether group children are visible in gui
				int i;
				if (to_string(toks1[0]).substr(0, 5) == "child" && cgv::utils::is_integer(to_string(toks1[0]).substr(5), i)) {
					v1 = "true";
				}
			}
		}
		if (v1 != v) {
			if (!decs.empty())
				decs += ";";
			decs += to_string(toks1[0])+"=";
			if (toks1[1] == "string") {
				decs += '"';
				decs += v;
				decs += '"';
			}
			else
				decs += v;
		}
	}
	return decs;
}

std::string scene::reconstruct_description_recursive(unsigned int& i, implicit_type* func_ptr, group* g)
{
	cgv::base::base_ptr bp_ref;
//	implicit_type* func_ptr_ref = 0;
	std::string desc;
	unsigned int i0 = i;
	unsigned int ci = 0;
	if (g) {
		if (g->get_nr_children() > 0)
			func_ptr = g->get_child(ci)->get_interface<implicit_type>();
		else
			func_ptr = 0;
	}
	while (i < (unsigned int)description.size()) {
		bool used_factory = false;
		unsigned offset = 0;
		for (unsigned int j=0; j<factories.size(); ++j) {
			if (symbol_matches_description(i, factories[j], offset)) {
				bp_ref = factories[j]->create_function();
				used_factory = true;
				break;
			}
		}
		if (used_factory) {
			i += offset;
			continue;
		}
		switch (description[i]) {
		case '[' :
			{
				unsigned int t = i+1;
				++i;
				desc += description.substr(i0,i-i0);
				for (i; i < description.size() && description[i] != ']'; ++i) {
				}
				i0 = i;
				if (g)
					desc += get_changed_values(g->get_child(ci)->get_interface<implicit_type>(), bp_ref->get_interface<implicit_type>());
				else
					desc += get_changed_values(func_ptr, bp_ref->get_interface<implicit_type>());
				break;
			}
		case '<' :
			{
				unsigned int t = i+1;
				for (++i; i < description.size() && description[i] != '>'; ++i) {
				}
				break;
			}
		case '(' :
			if (func_ptr) {
				group* g = dynamic_cast<group*>(func_ptr);
				if (g) {
					++i;
					desc += description.substr(i0,i-i0);
					desc += reconstruct_description_recursive(i, func_ptr, g);
					i0 = i;
				}
			}
			break;
		case ',' :
			++ci;
			if (g && g->get_nr_children() > ci)
				func_ptr = g->get_child(ci)->get_interface<implicit_type>();
			else
				func_ptr = 0;
			bp_ref = 0;
			break;
		case ')' :
			return desc + description.substr(i0,i-i0);
		default:
			++i;
			continue;
		}
		++i;
	}
	return desc + description.substr(i0,i-i0);
}

/// callback for functions that update the scene based on gui interaction
void scene::update_scene()
{
	if (!help_shown) {
		help_shown = true;
		show_help();
	}
	if (!disable_update) {
		reconstruct_description();
		impl_draw_ptr->post_rebuild();
	}
}

/// callback for functions that update the scene description without the implicit function
void scene::update_description()
{
	if (!help_shown) {
		help_shown = true;
		show_help();
	}
	if (!disable_update)
		reconstruct_description();
}

/// overload to return the type name of this object
std::string scene::get_type_name() const 
{
	return "scene"; 
}

/// cast evaluation to func_base_ptr
double scene::evaluate(const pnt_type& p) const
{
	if (func_base_ptr)
		return func_base_ptr->get_interface<implicit_type>()->evaluate(
			implicit_base<double>::pnt_type(p.x(), p.y(), p.z())
		);
	return 0;
}

/// cast gradient evaluation to func_base_ptr
scene::vec_type scene::evaluate_gradient(const pnt_type& p) const
{
	if (func_base_ptr)
	{
		vec_type g = func_base_ptr->get_interface<implicit_type>()->evaluate_gradient(
			implicit_base<double>::pnt_type(p.x(), p.y(), p.z())
		).to_vec();
		return g;
	}
	return vec_type(0, 0, 0);
}

///
void scene::create_gui()
{
	add_decorator("scene", "heading");
	if (func_base_ptr)
		inline_object_gui(func_base_ptr);
}

struct scene_registration
{
	static std::string options;
	static void scene_reg(gui_driver_ptr) { register_object(ref_scene(), options); }
	scene_registration(const std::string& options = "")
	{
		if (get_gui_driver()) {
			register_object(ref_scene(), options);
		}
		else {
			this->options = options;
			connect(on_gui_driver_registration(), &scene_registration::scene_reg);
		}
	}
};

std::string scene_registration::options;

scene_registration scene_reg("");
