cmake_minimum_required(VERSION 3.12)

# suppress -isystem option for gcc when importing SDL2
set(CMAKE_NO_SYSTEM_FROM_IMPORTED TRUE)

project(x68000)
set(VMTARGET _X68000)

#set(LINUX 1)
#set(UNIX 1)

set(SRCDIR ${CMAKE_CURRENT_LIST_DIR}/src)
set(SRCOSD ${SRCDIR}/osd)
set(SRCOSDLINUX ${SRCOSD}/linux)
set(SRCOSDGTK ${SRCOSD}/gtk)
set(SRCOSDMAC ${SRCOSD}/mac)
set(SRCOSDSDL ${SRCOSD}/SDL)
set(SRCOSDWIN ${SRCOSD}/windows)
set(SRCVM ${SRCDIR}/vm)
set(SRCFMGEN ${SRCVM}/fmgen)
set(SRCDEP ${SRCVM}/x68000)
set(SRCGUI ${SRCDIR}/gui)
set(SRCGUICOCOA ${SRCGUI}/cocoa)
set(SRCGUIGTKX11 ${SRCGUI}/gtk_x11)
set(SRCGUIWIN ${SRCGUI}/windows)
set(SRCVID ${SRCDIR}/video)
set(SRCVIDAVKIT ${SRCVID}/avkit)
set(SRCVIDCOCOA ${SRCVID}/cocoa)
set(SRCVIDFFM ${SRCVID}/ffmpeg)
set(SRCVIDPNG ${SRCVID}/libpng)
set(SRCVIDQTKIT ${SRCVID}/qtkit)
set(SRCVIDVFW ${SRCVID}/vfw)
set(SRCVIDWAV ${SRCVID}/wave)
set(SRCVIDWIN ${SRCVID}/windows)

set(RESDIR ${SRCDIR}/res/common)

add_executable(${PROJECT_NAME}
	${SRCDIR}/config.cpp
	${SRCDIR}/fifo.cpp
	${SRCDIR}/fileio.cpp
	${SRCDIR}/emumsg.cpp
	${SRCDIR}/common.cpp
	${SRCDIR}/depend.cpp
	${SRCDIR}/cchar.cpp
	${SRCDIR}/cmutex.cpp
	${SRCDIR}/curtime.cpp
	${SRCDIR}/cpixfmt.cpp
	${SRCDIR}/labels.cpp
	${SRCDIR}/msgs.cpp
	${SRCDIR}/simple_ini.cpp
	${SRCDIR}/ConvertUTF.c
	${SRCDIR}/debugger_bpoint.cpp
	${SRCDIR}/debugger_socket.cpp
	${SRCDIR}/debugger_symbol.cpp
	${SRCDIR}/utility.cpp

	${SRCOSDSDL}/sdl_emu.cpp
	${SRCOSDSDL}/sdl_sound.cpp
	${SRCOSDSDL}/sdl_timer.cpp
	${SRCOSDSDL}/sdl_debugger_console.cpp
	${SRCOSDSDL}/sdl_socket.cpp
	${SRCOSDSDL}/sdl_msgboard.cpp
	${SRCOSDSDL}/sdl_parseopt.cpp
	${SRCOSDSDL}/sdl_ledboxbase.cpp
	${SRCOSDSDL}/sdl_vkeyboardbase.cpp
	${SRCOSDSDL}/sdl_csurface.cpp
	${SRCOSDSDL}/sdl_cbitmap.cpp
	${SRCOSDSDL}/sdl_ccolor.cpp

	${SRCOSD}/d88_files.cpp
	${SRCOSD}/debugger_console.cpp
	${SRCOSD}/emu.cpp
	${SRCOSD}/emu_input.cpp
	${SRCOSD}/emu_input_keysym.cpp
	${SRCOSD}/emu_screen.cpp
	${SRCOSD}/emu_sound.cpp
	${SRCOSD}/keybind.cpp
	${SRCOSD}/logging.cpp
	${SRCOSD}/osd_midi.cpp
	${SRCOSD}/parseopt.cpp
	${SRCOSD}/screenmode.cpp
	${SRCOSD}/simple_clocale.cpp
	${SRCOSD}/vkeyboardbase.cpp
	${SRCOSD}/windowmode.cpp
	${SRCOSD}/opengl.cpp

	${SRCVM}/device.cpp
	${SRCVM}/disk.cpp
	${SRCVM}/disk_parser.cpp
	${SRCVM}/event.cpp
	${SRCVM}/i8255.cpp
	${SRCVM}/harddisk.cpp
	${SRCVM}/mc68000.cpp
	${SRCVM}/mc68000cycs.cpp
	${SRCVM}/mc68000dasm.cpp
	${SRCVM}/mc68000fpu.cpp
	${SRCVM}/mc68000mmu.cpp
	${SRCVM}/mc68000ops.cpp
	${SRCVM}/noise.cpp
	${SRCVM}/parsewav.cpp
	${SRCVM}/paw_datas.cpp
	${SRCVM}/paw_defs.cpp
	${SRCVM}/paw_dft.cpp
	${SRCVM}/paw_file.cpp
	${SRCVM}/paw_format.cpp
	${SRCVM}/paw_param.cpp
	${SRCVM}/paw_parse.cpp
	${SRCVM}/paw_parsecar.cpp
	${SRCVM}/paw_parsewav.cpp
	${SRCVM}/paw_util.cpp
	${SRCVM}/sound_base.cpp
	${SRCVM}/ym2151.cpp
	${SRCVM}/debugger_base.cpp
	${SRCVM}/debugger.cpp

	${SRCFMGEN}/fmgen.cpp
	${SRCFMGEN}/fmtimer.cpp
	${SRCFMGEN}/opm.cpp
	${SRCFMGEN}/opna.cpp
	${SRCFMGEN}/psg.cpp

	${SRCDEP}/adpcm.cpp
	${SRCDEP}/board.cpp
	${SRCDEP}/comm.cpp
	${SRCDEP}/crtc.cpp
	${SRCDEP}/display.cpp
	${SRCDEP}/dmac.cpp
	${SRCDEP}/fdc.cpp
	${SRCDEP}/floppy.cpp
	${SRCDEP}/joypad.cpp
	${SRCDEP}/keyboard.cpp
	${SRCDEP}/keyrecord.cpp
	${SRCDEP}/memory.cpp
	${SRCDEP}/mfp.cpp
	${SRCDEP}/midi.cpp
	${SRCDEP}/mouse.cpp
	${SRCDEP}/printer.cpp
	${SRCDEP}/rtc.cpp
	${SRCDEP}/scc.cpp
	${SRCDEP}/sprite_bg.cpp
	${SRCDEP}/sysport.cpp
	${SRCDEP}/sxsi.cpp
	${SRCDEP}/sasi.cpp
	${SRCDEP}/scsi.cpp
	${SRCDEP}/x68000.cpp

	${SRCGUI}/gui_base.cpp
	${SRCGUI}/sdl2_ledbox.cpp
	${SRCGUI}/gui_keybinddata.cpp

	${SRCVID}/rec_audio.cpp
	${SRCVID}/rec_common.cpp
	${SRCVID}/rec_video.cpp
)

add_definitions(-D${VMTARGET} -DUSE_OPENGL -DUSE_GETTEXT -DUSE_DEBUGGER -DUSE_SDL2)

set(RESOURCE_FILES
	${RESDIR}/x68000.png
	${RESDIR}/x68000_front_32.png
	${RESDIR}/x68000_front_48.png
	${RESDIR}/x68000_front_64.png
	${RESDIR}/x68000_front_128.png
	${RESDIR}/ledbox_x68k_1.png
	${RESDIR}/ledbox_x68k_2.png
	${RESDIR}/ledparts_x68k_1.png
	${RESDIR}/x68000_keyboard_1.png
	${RESDIR}/x68000_keyboard_1_led0.png
	${RESDIR}/x68000_keyboard_1_led1.png
	${RESDIR}/x68000_keyboard_1_led2.png
	${RESDIR}/x68000_keyboard_1_led3.png
)

if(APPLE)
  #
  # For MacOS
  #
  set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_LIST_DIR})

  find_package(SDL2 REQUIRED COMPONENTS SDL2)
  find_package(SDL2 REQUIRED COMPONENTS SDL2main)
  find_package(SDL2_ttf REQUIRED COMPONENTS SDL2_ttf)

  target_link_libraries(${PROJECT_NAME} PUBLIC SDL2::SDL2 SDL2::SDL2main SDL2_ttf::SDL2_ttf)

  target_sources(${PROJECT_NAME} PUBLIC
	${SRCOSDSDL}/sdl_main.cpp
	${SRCOSDSDL}/sdl_input.cpp
	${SRCOSDSDL}/sdl_input_keysym.cpp
	${SRCOSDSDL}/sdl_screenmode.cpp
	${SRCOSDSDL}/sdl_screen.cpp

	${SRCOSDMAC}/mac_uart.cpp
	${SRCOSDMAC}/mac_midi.cpp

	${SRCGUICOCOA}/cocoa_gui.mm
	${SRCGUICOCOA}/cocoa_basepanel.mm
	${SRCGUICOCOA}/cocoa_volumepanel.mm
	${SRCGUICOCOA}/cocoa_keybindctrl.mm
	${SRCGUICOCOA}/cocoa_keybindpanel.mm
	${SRCGUICOCOA}/cocoa_recaudpanel.mm
	${SRCGUICOCOA}/cocoa_recvidpanel.mm
	${SRCGUICOCOA}/cocoa_seldrvpanel.mm
#	${SRCGUICOCOA}/cocoa_savedatarec.mm
	${SRCGUICOCOA}/cocoa_key_trans.mm
	${SRCGUICOCOA}/cocoa_fontpanel.mm
	${SRCGUICOCOA}/cocoa_ledbox.mm
	${SRCGUICOCOA}/cocoa_vkeyboard.mm
	${SRCGUICOCOA}/cocoa_joysetpanel.mm
	${SRCGUICOCOA}/cocoa_hdtypepanel.mm
	${SRCGUICOCOA}/cocoa_configpanel.mm
	${SRCGUICOCOA}/cocoa_loggingpanel.mm
	${SRCGUICOCOA}/cocoa_midlatepanel.mm

	${SRCVIDWAV}/wav_rec_audio.cpp
#	${SRCVIDFFM}/ffm_loadlib.cpp
#	${SRCVIDFFM}/ffm_rec_base.cpp
#	${SRCVIDFFM}/ffm_rec_audio.cpp
#	${SRCVIDFFM}/ffm_rec_video.cpp
#	${SRCVIDQTKIT}/qt_rec_video.mm
#	${SRCVIDAVKIT}/avk_rec_common.mm
#	${SRCVIDAVKIT}/avk_rec_video.mm
#	${SRCVIDAVKIT}/avk_rec_audio.mm
	${SRCVIDCOCOA}/cocoa_rec_video.mm
	${SRCVIDCOCOA}/cocoa_bitmap.mm
  )

  set(AppleLibs
    -Wl,-framework,Cocoa
    -Wl,-framework,Carbon
    -Wl,-framework,CoreAudio
    -Wl,-framework,IOKit
    -Wl,-framework,AudioUnit
    -Wl,-framework,ForceFeedback
    -Wl,-framework,QTKit
    -Wl,-framework,AVFoundation
    -Wl,-framework,CoreMedia
    -Wl,-framework,CoreVideo
    -Wl,-framework,AudioToolbox
    -Wl,-framework,Metal
    -Wl,-framework,OpenGL
    -Wl,-framework,CoreMidi
    -lm
    -lz
    -lbz2
    -liconv
  )

  target_compile_definitions(${PROJECT_NAME} PUBLIC GUI_TYPE_COCOA)
  target_compile_options(${PROJECT_NAME} PUBLIC -Wall -O2)
  target_include_directories(${PROJECT_NAME} PUBLIC $ENV{HOME}/Devel/ffmpeg ${SRCDIR}/include)
  target_link_libraries(${PROJECT_NAME} PUBLIC ${AppleLibs})
  target_link_options(${PROJECT_NAME} PUBLIC )

  install(DIRECTORY ${SRCDIR}/res/macosx/Contents DESTINATION Release/${PROJECT_NAME}.app)
  install(TARGETS ${PROJECT_NAME} DESTINATION Release/${PROJECT_NAME}.app/Contents/MacOS BUNDLE)
  install(FILES ${RESOURCE_FILES} DESTINATION Release/${PROJECT_NAME}.app/Contents/Resources)
  install(DIRECTORY locale DESTINATION Release/${PROJECT_NAME}.app/Contents/Resources)

elseif(LINUX)
  #
  # For LINUX
  #
  set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_LIST_DIR})

  find_package(PkgConfig REQUIRED)
  pkg_check_modules(GTK REQUIRED gtk+-3.0)
  find_package(SDL2 REQUIRED COMPONENTS SDL2)
  find_package(SDL2 REQUIRED COMPONENTS SDL2main)
  find_package(SDL2_ttf COMPONENTS SDL2_ttf)

  target_link_libraries(${PROJECT_NAME} PUBLIC SDL2::SDL2 SDL2::SDL2main)

  target_sources(${PROJECT_NAME} PUBLIC
	${SRCOSDGTK}/gtk_main.cpp
	${SRCOSDGTK}/gtk_input.cpp
	${SRCOSDGTK}/gtk_input_keysym.cpp
	${SRCOSDGTK}/gtk_screenmode.cpp
	${SRCOSDGTK}/gtk_screen.cpp

	${SRCOSDLINUX}/linux_uart.cpp
	${SRCOSDLINUX}/alsa_midi.cpp

	${SRCGUIGTKX11}/gtk_x11_gui.cpp
	${SRCGUIGTKX11}/gtk_dialogbox.cpp
	${SRCGUIGTKX11}/gtk_configbox.cpp
	${SRCGUIGTKX11}/gtk_x11_key_trans.cpp
	${SRCGUIGTKX11}/gtk_keybindctrl.cpp
	${SRCGUIGTKX11}/gtk_keybindbox.cpp
	${SRCGUIGTKX11}/gtk_volumebox.cpp
	${SRCGUIGTKX11}/gtk_recaudbox.cpp
	${SRCGUIGTKX11}/gtk_recvidbox.cpp
	${SRCGUIGTKX11}/gtk_filebox.cpp
	${SRCGUIGTKX11}/gtk_folderbox.cpp
	${SRCGUIGTKX11}/gtk_aboutbox.cpp
	${SRCGUIGTKX11}/gtk_vkeyboard.cpp
	${SRCGUIGTKX11}/gtk_joysetbox.cpp
	${SRCGUIGTKX11}/gtk_hdtypebox.cpp
	${SRCGUIGTKX11}/gtk_loggingbox.cpp
	${SRCGUIGTKX11}/gtk_midlatebox.cpp
	${SRCGUIGTKX11}/gtk_ledbox.cpp

	${SRCVIDWAV}/wav_rec_audio.cpp
#	${SRCVIDFFM}/ffm_loadlib.cpp
#	${SRCVIDFFM}/ffm_rec_base.cpp
#	${SRCVIDFFM}/ffm_rec_audio.cpp
#	${SRCVIDFFM}/ffm_rec_video.cpp
	${SRCVIDPNG}/png_rec_video.cpp
	${SRCVIDPNG}/png_bitmap.cpp
  )

  target_compile_definitions(${PROJECT_NAME} PUBLIC USE_GTK GUI_TYPE_GTK_X11)
  target_compile_options(${PROJECT_NAME} PUBLIC -Wall -O2)
  target_include_directories(${PROJECT_NAME} PUBLIC ${GTK_INCLUDE_DIRS} ${SRCDIR}/include)
  target_link_libraries(${PROJECT_NAME} PUBLIC ${GTK_LIBRARIES} SDL2_ttf m z GL pthread freetype rt dl X11 png asound)
# target_link_options(${PROJECT_NAME} PUBLIC )

  install(PROGRAMS ${CMAKE_BINARY_DIR}/${PROJECT_NAME} DESTINATION Release)
  install(FILES ${RESOURCE_FILES} DESTINATION Release/res)
  install(DIRECTORY locale DESTINATION Release)

elseif(MSVC)
  #
  # For Visual C++
  #
  set(DevelopDir D:\\Devel\\vc)
  set(SDL2Dir ${DevelopDir}\\SDL2-2.28.5-static)
  set(SDL2IncludeDir ${SDL2Dir}\\include)
  set(SDL2StaticLibDirX64Debug ${SDL2Dir}\\lib14\\x64\\Debug)
  set(SDL2StaticLibDirX64Release ${SDL2Dir}\\lib14\\x64\\Release)
  set(SDL2LibsDebug SDL2.lib SDL2main.lib)
  set(SDL2LibsRelease SDL2.lib SDL2main.lib)
  set(SDL2TTFDir ${DevelopDir}\\SDL2_ttf-2.20.2-static)
  set(SDL2TTFIncludeDir ${SDL2TTFDir})
  set(SDL2TTFStaticLibDirX64Debug ${SDL2TTFDir}\\lib14\\x64\\Debug)
  set(SDL2TTFStaticLibDirX64Release ${SDL2TTFDir}\\lib14\\x64\\Release)
  set(SDL2TTFLibsDebug SDL2_ttf.lib)
  set(SDL2TTFLibsRelease SDL2_ttf.lib)
  set(FFmpegDir ${DevelopDir}\\ffmpeg-4.4.1-win64-dev)
  set(FFmpegIncludeDir ${FFmpegDir}\\include)

  target_sources(${PROJECT_NAME} PUBLIC
	${SRCOSDSDL}/sdl_main.cpp
	${SRCOSDSDL}/sdl_input.cpp
	${SRCOSDSDL}/sdl_input_keysym.cpp
	${SRCOSDSDL}/sdl_screenmode.cpp
	${SRCOSDSDL}/sdl_screen.cpp

	${SRCOSDWIN}/win_uart.cpp
	${SRCOSDWIN}/win_midi.cpp

	${SRCGUIWIN}/win_dialogbox.cpp
	${SRCGUIWIN}/win_aboutbox.cpp
	${SRCGUIWIN}/win_filebox.cpp
	${SRCGUIWIN}/win_folderbox.cpp
	${SRCGUIWIN}/win_fontbox.cpp
	${SRCGUIWIN}/win_volumebox.cpp
	${SRCGUIWIN}/win_configbox.cpp
	${SRCGUIWIN}/win_keybindbox.cpp
	${SRCGUIWIN}/win_keybindctrl.cpp
	${SRCGUIWIN}/win_key_trans.cpp
	${SRCGUIWIN}/win_recaudbox.cpp
	${SRCGUIWIN}/win_recvidbox.cpp
	${SRCGUIWIN}/win_seldrvbox.cpp
	${SRCGUIWIN}/winfont.cpp
	${SRCGUIWIN}/win_ledbox.cpp
	${SRCGUIWIN}/win_vkeyboard.cpp
	${SRCGUIWIN}/win_joysetbox.cpp
	${SRCGUIWIN}/win_hdtypebox.cpp
	${SRCGUIWIN}/win_loggingbox.cpp
	${SRCGUIWIN}/win_midlatebox.cpp
	${SRCGUIWIN}/win_gui.cpp

	${SRCVIDWAV}/wav_rec_audio.cpp
#	${SRCVIDVFW}/vfw_rec_video.cpp
#	${SRCVIDFFM}/ffm_loadlib.cpp
#	${SRCVIDFFM}/ffm_rec_base.cpp
#	${SRCVIDFFM}/ffm_rec_audio.cpp
#	${SRCVIDFFM}/ffm_rec_video.cpp

	${SRCVIDWIN}/win_rec_video.cpp
	${SRCVIDWIN}/win_bitmap.cpp

	${SRCDIR}/res/windows/x68000.rc
	${SRCDIR}/res/windows/x68000_gui.rc

	${SRCDIR}/res/windows/style_x86.manifest
  )

  target_compile_definitions(${PROJECT_NAME} PUBLIC GUI_TYPE_WINDOWS)
  target_include_directories(${PROJECT_NAME} PUBLIC ${SRCDIR} ${SDL2IncludeDir} ${SDL2TTFIncludeDir} ${FFmpegIncludeDir})

  target_link_directories(${PROJECT_NAME} PUBLIC ${SDL2StaticLibDirX64Release} ${SDL2TTFStaticLibDirX64Release})
  target_link_libraries(${PROJECT_NAME} PUBLIC ${SDL2LibsRelease} ${SDL2TTFLibsRelease} winmm.lib opengl32.lib dxguid.lib ws2_32.lib Iphlpapi.lib Shlwapi.lib version.lib imm32.lib)
  target_link_options(bml3mk5 PUBLIC /SUBSYSTEM:WINDOWS)

  # copy resource files
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:${PROJECT_NAME}>/res)
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${RESOURCE_FILES} $<TARGET_FILE_DIR:${PROJECT_NAME}>/res)
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/locale $<TARGET_FILE_DIR:${PROJECT_NAME}>/locale)

else()
  #
  # For MSYS2 + MINGW64
  #
  set(CMAKE_C_COMPILER gcc)
  set(CMAKE_CXX_COMPILER g++)

  set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_LIST_DIR})

  find_package(SDL2 REQUIRED COMPONENTS SDL2)
  find_package(SDL2 REQUIRED COMPONENTS SDL2main)
  find_package(SDL2_ttf REQUIRED COMPONENTS SDL2_ttf)

  target_link_libraries(${PROJECT_NAME} PUBLIC SDL2::SDL2 SDL2::SDL2main SDL2_ttf)

  target_sources(${PROJECT_NAME} PUBLIC
	${SRCOSDSDL}/sdl_main.cpp
	${SRCOSDSDL}/sdl_input.cpp
	${SRCOSDSDL}/sdl_input_keysym.cpp
	${SRCOSDSDL}/sdl_screenmode.cpp
	${SRCOSDSDL}/sdl_screen.cpp

	${SRCOSDWIN}/win_uart.cpp
	${SRCOSDWIN}/win_midi.cpp

	${SRCGUIWIN}/win_dialogbox.cpp
	${SRCGUIWIN}/win_aboutbox.cpp
	${SRCGUIWIN}/win_filebox.cpp
	${SRCGUIWIN}/win_folderbox.cpp
	${SRCGUIWIN}/win_fontbox.cpp
	${SRCGUIWIN}/win_volumebox.cpp
	${SRCGUIWIN}/win_configbox.cpp
	${SRCGUIWIN}/win_keybindbox.cpp
	${SRCGUIWIN}/win_keybindctrl.cpp
	${SRCGUIWIN}/win_key_trans.cpp
	${SRCGUIWIN}/win_recaudbox.cpp
	${SRCGUIWIN}/win_recvidbox.cpp
	${SRCGUIWIN}/win_seldrvbox.cpp
	${SRCGUIWIN}/winfont.cpp
	${SRCGUIWIN}/win_ledbox.cpp
	${SRCGUIWIN}/win_vkeyboard.cpp
	${SRCGUIWIN}/win_joysetbox.cpp
	${SRCGUIWIN}/win_hdtypebox.cpp
	${SRCGUIWIN}/win_loggingbox.cpp
	${SRCGUIWIN}/win_midlatebox.cpp
	${SRCGUIWIN}/win_gui.cpp

	${SRCVIDWAV}/wav_rec_audio.cpp
#	${SRCVIDVFW}/vfw_rec_video.cpp
#	${SRCVIDFFM}/ffm_loadlib.cpp
#	${SRCVIDFFM}/ffm_rec_base.cpp
#	${SRCVIDFFM}/ffm_rec_audio.cpp
#	${SRCVIDFFM}/ffm_rec_video.cpp

	${SRCVIDWIN}/win_rec_video.cpp
	${SRCVIDWIN}/win_bitmap.cpp

	${SRCDIR}/res/windows/x68000.rc
	${SRCDIR}/res/windows/x68000_gui.rc
  )

  set(CMAKE_RC_COMPILER_INIT windres.exe)
  enable_language(RC)

  set(WindowsLibs
    -lwinmm
    -limm32
    -lopengl32
    -lvfw32
    -lcomctl32
    -lshlwapi
    -lws2_32
    -lglu32
    -lexpat
    -lpcre2-16
    -lpng
    -ltiff
    -ljbig
    -lz
    -lrpcrt4
    -loleaut32
    -lole32
    -luuid
    -luxtheme
    -lwinspool
    -lshell32
    -lcomctl32
    -ladvapi32
    -lversion
    -lws2_32
    -lgdi32
    -loleacc
    -lwinhttp
    -lsetupapi
    -lgdiplus
  )

  SET(CMAKE_RC_COMPILE_OBJECT
    "<CMAKE_RC_COMPILER> -i <SOURCE> <DEFINES> <FLAGS> <INCLUDES> -O coff -o <OBJECT>")

  target_compile_definitions(${PROJECT_NAME} PUBLIC WINVER=0x500 GUI_TYPE_WINDOWS USE_RC_MANIFEST main=SDL_main)
  target_compile_options(${PROJECT_NAME} PUBLIC -Wall -O2)
  target_include_directories(${PROJECT_NAME} PUBLIC /usr/local/include /usr/include /D/Devel/vc/ffmpeg-3.4.1-win64-dev/include ${SRCDIR}/include)
  target_link_libraries(${PROJECT_NAME} PUBLIC ${WindowsLibs})
  target_link_options(${PROJECT_NAME} PUBLIC -Wl,--subsystem,windows -mwindows)

  install(PROGRAMS ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.exe DESTINATION Release)
  install(FILES ${RESOURCE_FILES} DESTINATION Release/res)
  install(DIRECTORY locale DESTINATION Release)

endif()

