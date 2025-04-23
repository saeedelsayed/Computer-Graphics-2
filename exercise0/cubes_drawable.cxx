// This source code is property of the Computer Graphics and Visualization chair of the
// TU Dresden. Do not distribute! 
// Copyright (C) CGV TU Dresden - All Rights Reserved
//
// The main file of the plugin. It defines a class that demonstrates how to register with
// the scene graph, drawing primitives, creating a GUI, using a config file and various
// other parts of the framework.

// Framework core
#include <cgv/base/register.h>
#include <cgv/gui/provider.h>
#include <cgv/gui/trigger.h>
#include <cgv/render/drawable.h>
#include <cgv/render/shader_program.h>
#include <cgv/render/vertex_buffer.h>
#include <cgv/render/attribute_array_binding.h>
#include <cgv/math/ftransform.h>

// Framework standard plugins
#include <cgv_gl/gl/gl.h>

// Local includes
#include "cubes_fractal.h"


// ************************************************************************************/
// Task 0.2a: Create a drawable that provides a (for now, empty) GUI and supports
//            reflection, so that its properties can be set via config file.
// 
// Task 0.2b: Utilize the cubes_fractal class to render a fractal of hierarchically
//            transformed cubes. Expose its recursion depth and color properties to GUI
//            manipulation and reflection. Set reasonable values via the config
//            file.
//
// Task 0.2c: Implement an option (configurable via GUI and config file) to use a vertex
//            array object for rendering the cubes. The vertex array functionality 
//            should support (again, configurable via GUI and config file) both
//            interleaved (as in cgv_demo.cpp) and non-interleaved attributes.

// < your code here >

// The CGV framework cubes drawable class for Task 0.2
class cubes_drawable
    : public cgv::base::base,      // This class supports reflection
    public cgv::gui::provider,   // Instances of this class provide a GUI
    public cgv::render::drawable // Instances of this class can be rendered
{
protected:
    // Cubes fractal instance to render the fractal
    cubes_fractal fractal;

    // Properties we expose via reflection

    // Recursion depth for the fractal
    unsigned int recursion_depth;

    // Color of the root cube
    cgv::media::color<float> root_color;

    // Rendering mode options
    enum VertexArrayMode { BUILT_IN, INTERLEAVED, NON_INTERLEAVED } render_mode;

    // Geometry buffers for custom tesselation
    struct vertex {
        cgv::vec3 pos;
        cgv::vec3 normal;
    };
    std::vector<vertex> vertices;
    std::vector<cgv::vec3> positions;
    std::vector<cgv::vec3> normals;
    cgv::render::vertex_buffer vb_interleaved;
    cgv::render::vertex_buffer vb_positions;
    cgv::render::vertex_buffer vb_normals;
    cgv::render::attribute_array_binding interleaved_array;
    cgv::render::attribute_array_binding non_interleaved_array;

public:
    // Default constructor
    cubes_drawable()
        : recursion_depth(3),
        root_color(0.7f, 0.5f, 0.3f),
        render_mode(BUILT_IN)
    {
    }

    // Should be overwritten to sensibly implement the cgv::base::named interface
    std::string get_type_name(void) const
    {
        return "cubes_drawable";
    }

    // Part of the cgv::base::base interface for reflection
    bool self_reflect(cgv::reflect::reflection_handler& rh)
    {
        // This is currently necessary to trick the framework into treating enums like
        // integers for the purpose of reflection
        unsigned* render_mode_uint = (unsigned*)&render_mode;

        // Reflect the properties
        return
            rh.reflect_member("recursion_depth", recursion_depth) &&
            rh.reflect_member("root_color_r", root_color[0]) &&
            rh.reflect_member("root_color_g", root_color[1]) &&
            rh.reflect_member("root_color_b", root_color[2]) &&
            rh.reflect_member("render_mode", *render_mode_uint);
    }

    // Handle property changes
    void on_set(void* member_ptr)
    {
        // Make sure the GUI reflects the new state, in case the write access did not
        // originate from GUI interaction
        update_member(member_ptr);

        // Trigger a redraw if the drawable node is active
        if (this->is_visible())
            post_redraw();
    }

    // Required interface for cgv::gui::provider
    void create_gui(void)
    {
        // Section header
        add_decorator("Cubes Fractal Properties", "heading", "level=1");

        // Recursion depth control
        add_member_control(this, "recursion depth", recursion_depth, "value_slider", "min=0;max=6;ticks=true");

        // Root cube color
        add_member_control(this, "root color", root_color);

        // Rendering mode selection
        add_member_control(
            this, "render mode", render_mode, "dropdown",
            "enums='BUILT_IN,INTERLEAVED,NON_INTERLEAVED'"
        );
    }

    // Initialize unit cube geometry
    void init_unit_cube_geometry(void)
    {
        // Define the 8 vertices of a cube
        const cgv::vec3 cube_vertices[8] = {
            cgv::vec3(-1, -1, -1), // 0: bottom-left-back
            cgv::vec3(1, -1, -1),  // 1: bottom-right-back
            cgv::vec3(1, 1, -1),   // 2: top-right-back
            cgv::vec3(-1, 1, -1),  // 3: top-left-back
            cgv::vec3(-1, -1, 1),  // 4: bottom-left-front
            cgv::vec3(1, -1, 1),   // 5: bottom-right-front
            cgv::vec3(1, 1, 1),    // 6: top-right-front
            cgv::vec3(-1, 1, 1)    // 7: top-left-front
        };

        // Define normals for each face
        const cgv::vec3 face_normals[6] = {
            cgv::vec3(0, 0, -1),  // back face
            cgv::vec3(0, 0, 1),   // front face
            cgv::vec3(-1, 0, 0),  // left face
            cgv::vec3(1, 0, 0),   // right face
            cgv::vec3(0, -1, 0),  // bottom face
            cgv::vec3(0, 1, 0)    // top face
        };

        // Define faces (6 faces, 4 vertices per face for GL_QUADS)
        // Each entry is (vertex_index, normal_index)
        const std::pair<int, int> face_definitions[6][4] = {
            // back face (z = -1)
            {{0, 0}, {1, 0}, {2, 0}, {3, 0}},
            // front face (z = 1)
            {{4, 1}, {7, 1}, {6, 1}, {5, 1}},
            // left face (x = -1)
            {{0, 2}, {3, 2}, {7, 2}, {4, 2}},
            // right face (x = 1)
            {{1, 3}, {5, 3}, {6, 3}, {2, 3}},
            // bottom face (y = -1)
            {{0, 4}, {4, 4}, {5, 4}, {1, 4}},
            // top face (y = 1)
            {{3, 5}, {2, 5}, {6, 5}, {7, 5}}
        };

        // Clear previous data
        vertices.clear();
        positions.clear();
        normals.clear();

        // Generate interleaved and non-interleaved data
        for (int face = 0; face < 6; ++face) {
            for (int v = 0; v < 4; ++v) {
                int vertex_idx = face_definitions[face][v].first;
                int normal_idx = face_definitions[face][v].second;

                // For interleaved storage
                vertex vert;
                vert.pos = cube_vertices[vertex_idx];
                vert.normal = face_normals[normal_idx];
                vertices.push_back(vert);

                // For non-interleaved storage
                positions.push_back(cube_vertices[vertex_idx]);
                normals.push_back(face_normals[normal_idx]);
            }
        }
    }

    // Initialize vertex buffers and arrays
    bool init(cgv::render::context& ctx)
    {
        // Keep track of success
        bool success = true;

        // Initialize the geometry
        init_unit_cube_geometry();

        // Get a reference to the surface shader for attribute indices
        cgv::render::shader_program& shader = ctx.ref_surface_shader_program();

        // Create the interleaved buffer and array
        success = vb_interleaved.create(ctx, &(vertices[0]), vertices.size()) && success;
        success = interleaved_array.create(ctx) && success;

        // Set up the interleaved attribute arrays
        cgv::render::type_descriptor vec3type =
            cgv::render::element_descriptor_traits<cgv::vec3>::get_type_descriptor(vertices[0].pos);

        success = interleaved_array.set_attribute_array(
            ctx, shader.get_position_index(), vec3type, vb_interleaved,
            0, // position is at start of the struct
            vertices.size(), // number of elements
            sizeof(vertex) // stride
        ) && success;

        success = interleaved_array.set_attribute_array(
            ctx, shader.get_normal_index(), vec3type, vb_interleaved,
            sizeof(cgv::vec3), // normal follows position
            vertices.size(), // number of elements
            sizeof(vertex) // stride
        ) && success;

        // Create the non-interleaved buffers and array
        success = vb_positions.create(ctx, &(positions[0]), positions.size()) && success;
        success = vb_normals.create(ctx, &(normals[0]), normals.size()) && success;
        success = non_interleaved_array.create(ctx) && success;

        // Set up the non-interleaved attribute arrays
        success = non_interleaved_array.set_attribute_array(
            ctx, shader.get_position_index(), vec3type, vb_positions,
            0, // no offset in the positions buffer
            positions.size(), // number of elements
            sizeof(cgv::vec3) // stride
        ) && success;

        success = non_interleaved_array.set_attribute_array(
            ctx, shader.get_normal_index(), vec3type, vb_normals,
            0, // no offset in the normals buffer
            normals.size(), // number of elements
            sizeof(cgv::vec3) // stride
        ) && success;

        return success;
    }

    // Draw the scene
    void draw(cgv::render::context& ctx)
    {
        // Get the surface shader
        cgv::render::shader_program& shader = ctx.ref_surface_shader_program();

        // Enable shader program
        shader.enable(ctx);

        // Configure vertex array for the fractal based on rendering mode
        switch (render_mode) {
        case INTERLEAVED:
            fractal.use_vertex_array(&interleaved_array, vertices.size(), GL_QUADS);
            break;
        case NON_INTERLEAVED:
            fractal.use_vertex_array(&non_interleaved_array, positions.size(), GL_QUADS);
            break;
        case BUILT_IN:
        default:
            fractal.use_vertex_array(nullptr, 0, GL_QUADS);
            break;
        }

        // Save modelview matrix
        ctx.push_modelview_matrix();

        // Draw the fractal with the configured parameters
        fractal.draw_recursive(ctx, root_color, recursion_depth);

        // Restore modelview matrix
        ctx.pop_modelview_matrix();

        // Disable shader program
        shader.disable(ctx);
    }
};


// [END] Tasks 0.2a, 0.2b and 0.2c
// ************************************************************************************/


// ************************************************************************************/
// Task 0.2a: register an instance of your drawable.

// < your code here >
cgv::base::object_registration<cubes_drawable> reg_cubes_drawable("cubes_drawable");

