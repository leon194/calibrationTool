if(MSVC)
	add_definitions(/W0)
endif()

include_directories(
	../include 
	../include/GL
)

add_library(freeglut_static
#	freeglut_internal.h
#	freeglut_teapot_data.h
#	glui_algebra3.h
#	glui_arcball.h
#	glui_quaternion.h
#	glui_stdinc.h

	freeglut_callbacks.c
	freeglut_cursor.c
	freeglut_display.c
	freeglut_ext.c
	freeglut_font.c
	freeglut_font_data.c
	freeglut_gamemode.c
	freeglut_geometry.c
	freeglut_glutfont_definitions.c
	freeglut_init.c
	freeglut_joystick.c
	freeglut_main.c
	freeglut_menu.c
	freeglut_misc.c
	freeglut_overlay.c
	freeglut_state.c
	freeglut_stroke_mono_roman.c
	freeglut_stroke_roman.c
	freeglut_structure.c
	freeglut_teapot.c
	freeglut_videoresize.c
	freeglut_window.c
)

add_library(gluit_lib
	glui.cc
	glui_add_controls.cc
	glui_algebra3.cc
	glui_arcball.cc
	glui_bitmap_img_data.cc
	glui_bitmaps.cc
	glui_button.cc
	glui_checkbox.cc
	glui_column.cc
	glui_control.cc
	glui_edittext.cc
	glui_listbox.cc
	glui_mouse_iaction.cc
	glui_node.cc
	glui_panel.cc
	glui_quaternion.cc
	glui_radio.cc
	glui_rollout.cc
	glui_rotation.cc
	glui_separator.cc
	glui_slider.cc
	glui_spinner.cc
	glui_statictext.cc
	glui_translation.cc
)


INSTALL(TARGETS freeglut_static
        DESTINATION ${LIBS_INSTALL_DIR}/Release
        CONFIGURATIONS Release)

INSTALL(TARGETS freeglut_static
        DESTINATION ${LIBS_INSTALL_DIR}/Debug
        CONFIGURATIONS Debug)


INSTALL(TARGETS gluit_lib
        DESTINATION ${LIBS_INSTALL_DIR}/Release
        CONFIGURATIONS Release)

INSTALL(TARGETS gluit_lib
        DESTINATION ${LIBS_INSTALL_DIR}/Debug
        CONFIGURATIONS Debug)