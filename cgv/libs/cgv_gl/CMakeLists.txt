
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")
file(GLOB_RECURSE PPP_SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.ph")
file(GLOB_RECURSE HEADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.h")
file(GLOB_RECURSE SHADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "glsl/*.gl*")

cgv_add_target(cgv_gl
	TYPE library
	SOURCES ${SOURCES}
	HEADERS ${HEADERS}
	PPP_SOURCES ${PPP_SOURCES}
	SHADER_SOURCES ${SHADERS}
	DEPENDENCIES
		OpenGL::GL ${CGV_OGL_PLATFORM_DEPS} glew json cgv_utils cgv_type cgv_data cgv_base cgv_signal cgv_math
		cgv_media cgv_render cgv_gui cgv_reflect_types

	OVERRIDE_SHARED_EXPORT_DEFINE CGV_RENDER_GL_EXPORTS
	OVERRIDE_FORCE_STATIC_DEFINE  CGV_RENDER_GL_FORCE_STATIC
)

if (APPLE)
	target_compile_definitions(cgv_gl PRIVATE __APPLGLSU__ GL_SILENCE_DEPRECATION)
	target_compile_definitions(cgv_gl_static PRIVATE __APPLGLSU__ GL_SILENCE_DEPRECATION)
endif ()

install(DIRECTORY glsl/. DESTINATION ${CGV_LIBS_INCLUDE_DEST}/cgv_gl/glsl)
