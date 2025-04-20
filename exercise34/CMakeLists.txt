
# compile a list of source files for each specific source type the CGV CMake build system knows about
set(SOURCES
	Animation.cxx
	AtomicTransform.cxx
	Bone.cxx
	DataStore.cxx
	IHasBoundingBox.cxx
	IKViewer.cxx
	Init.cxx
	Mesh.cxx
	Skeleton.cxx
	SkeletonViewer.cxx
	SkinnedMeshViewer.cxx
)
set(HEADERS
	Animation.h
	AnimationFrameBone.h
	AtomicTransform.h
	Bone.h
	common.h
	DataStore.h
	IHasBoundingBox.h
	IKViewer.h
	math_helper.h
	Mesh.h
	Skeleton.h
	SkeletonViewer.h
	SkinnedMeshViewer.h
)
set(SHADERS
	skinning.glfs
	skinning.glgs
	skinning.glvs
)

# add our target to the CGV CMake build system
cgv_add_target(task34_animation
	TYPE plugin    NO_EXECUTABLE
	SOURCES        ${SOURCES}
	HEADERS        ${HEADERS}
	SHADER_SOURCES ${SHADERS}

	DEPENDENCIES
		cgv_viewer glew fltk cgv_utils cgv_type cgv_reflect cgv_data cgv_signal cgv_base cgv_media cgv_gui
		cgv_render cgv_os cgv_gl cg_fltk crg_stereo_view cg_ext cg_meta cmi_io

	ADDITIONAL_CMDLINE_ARGS
		"config:\"${CMAKE_CURRENT_LIST_DIR}/config.def\""
)
