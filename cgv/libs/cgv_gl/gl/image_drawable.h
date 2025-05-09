#pragma once

#include <cgv/render/drawable.h>
#include <cgv/render/shader_program.h>
#include <cgv/render/vertex_buffer.h>
#include <cgv/render/attribute_array_binding.h>

#include "lib_begin.h"

namespace cgv {
	namespace render {
		namespace gl {

//! base class of drawables that show static or animated images
class CGV_API image_drawable : public drawable
{
protected:
	std::vector<unsigned> tex_ids;
	std::vector<float> durations;
	unsigned current_image;
	std::string file_name;
	std::vector<std::string> files;
	cgv::render::vertex_buffer vbo;
	cgv::render::attribute_array_binding aab;

	double start_time;
	double anim_time;
	double aspect;

	bool animate;
	bool use_blending;
	bool wireframe;

	float tess_level;

	/// extent selection box
	ibox2 selection;

	/// whether to show the selection rectangle
	bool show_selection;
	
	/// extent of image
	int W,H;

	vec4 min_value;
	vec4 max_value;
	vec4 gamma4;

	float checker_lambda, k1, k2, k3, k4, k5, k6, p1, p2, cx, cy, s;

	// mixing 
	bool use_mixing;
	unsigned mix_with;
	float mix_param;

	// sperical projection
	bool spherical;
	vec2 pan_tilt;
	float scale;

	cgv::render::shader_program prog;
public:
	/// construct base image drawable
	image_drawable();
	/// timer event that can be used for animation, connect this to some animation trigger
	void timer_event(double t, double dt);
	/// function to read a single image
	bool read_image(const std::string& _file_name);
	/// function to read a list of images
	bool read_images(const std::string& _file_name, const std::vector<std::string>& _files);
	/// function to save all images
	bool save_images(const std::string& output_file_name);
	/// return the number of loaded subimages
	unsigned get_nr_images() const { return (unsigned) tex_ids.size(); }
	/// construct shader program
	bool init(context&);
	/// draw the image
	void draw(context&);
	/// destruct textures and shader program
	void clear(context&);
};

		}
	}
}

#include <cgv/config/lib_end.h>