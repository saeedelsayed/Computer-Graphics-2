
# compile a list of source files for each specific source type the CGV CMake build system knows about
set(SOURCES
	box.cxx
	csg.cxx
	cylinder.cxx
	distance_surface.cxx
	gl_implicit_surface_drawable.cxx
	implicit_base.cxx
	implicit_group.cxx
	implicit_primitive.cxx
	knot_vector.cxx
	numeric_gradient.cxx
	scene.cxx
	skeleton.cxx
	sphere.cxx
	transform.cxx
)
set(HEADERS
	distance_surface.h
	gl_implicit_surface_drawable.h
	implicit_base.h
	implicit_group.h
	implicit_primitive.h
	knot_vector.h
	scene.h
	skeleton.h
)

# add our target to the CGV CMake build system
cgv_add_target(task1_implicits
	TYPE plugin    NO_EXECUTABLE
	SOURCES        ${SOURCES}
	HEADERS        ${HEADERS}

	DEPENDENCIES
		cgv_viewer glew fltk cgv_utils cgv_type cgv_reflect cgv_data cgv_signal cgv_base cgv_media cgv_gui
		cgv_render cgv_os cgv_gl cg_fltk crg_stereo_view cg_ext cg_meta cmi_io

	ADDITIONAL_CMDLINE_ARGS
		"config:\"${CMAKE_CURRENT_LIST_DIR}/config.def\""
)
