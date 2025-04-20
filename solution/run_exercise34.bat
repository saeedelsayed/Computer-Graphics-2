@ECHO OFF
cd ..
.\solution\bin\windows\cgv_viewer64_143.exe plugin:cg_fltk "type(shader_config):shader_path='%cd%\solution\glsl'" plugin:cg_ext plugin:cg_meta plugin:cmi_io plugin:crg_stereo_view plugin:task34_animation "config:'%cd%\solution\config_ex34.cfg'"
