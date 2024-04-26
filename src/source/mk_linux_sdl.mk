#
# for linux 
#
# export CORE_TYPE
# export GUI_TYPE
# export BUILDDIR
# export LIBS
# export LDFLAGS
# export CDEFS
# export CFLAGS
# export CXXFLAGS
# export INSTALLDIR

CC:=gcc
CXX:=g++
LD:=g++

# FFMPEGDIR:=-I/usr/include/ffmpeg -I$(HOME)/Devel/ffmpeg

# COMMONCFLAGS:=-I/usr/local/include $(FFMPEGDIR) -I./include
COMMONCFLAGS:=-I/usr/local/include -I./include

CFLAGS:=$(CFLAGS) $(COMMONCFLAGS)
CXXFLAGS:=$(CXXFLAGS) $(COMMONCFLAGS)

EXEFILE:=x68000

DATADIR:=data

SRCDIR:=src
SRCOSD:=$(SRCDIR)/osd
SRCOSDSDL:=$(SRCOSD)/SDL
SRCOSDGTK:=$(SRCOSD)/gtk
SRCOSDLINUX:=$(SRCOSD)/linux
SRCVM:=$(SRCDIR)/vm
SRCFMGEN:=$(SRCVM)/fmgen
SRCDEP:=$(SRCVM)/x68000
SRCVID:=$(SRCDIR)/video
SRCVIDWAV:=$(SRCVID)/wave
SRCVIDFFM:=$(SRCVID)/ffmpeg
SRCVIDPNG:=$(SRCVID)/libpng
SRCGUIAGAR:=$(SRCDIR)/gui/agar
SRCGUIGTKX11=$(SRCDIR)/gui/gtk_x11
#ifeq ($(GUI_TYPE),GUI_TYPE_AGAR)
#	SRCGUI:=$(SRCGUIAGAR)
#endif
#ifeq ($(GUI_TYPE),GUI_TYPE_GTK_X11)
#	SRCGUI:=$(SRCGUIGTKX11)
#endif

RESDIR:=res
SRCRES:=$(SRCDIR)/$(RESDIR)/common

LOCALEDIR:=locale
SRCLOCALE:=$(LOCALEDIR)

EXE:=$(BUILDDIR)/$(EXEFILE)

EMUOBJS:=$(SRCDIR)/config.o \
	$(SRCDIR)/fifo.o \
	$(SRCDIR)/fileio.o \
	$(SRCDIR)/emumsg.o \
	$(SRCDIR)/common.o \
	$(SRCDIR)/depend.o \
	$(SRCDIR)/cchar.o \
	$(SRCDIR)/cmutex.o \
	$(SRCDIR)/curtime.o \
	$(SRCDIR)/cpixfmt.o \
	$(SRCDIR)/labels.o \
	$(SRCDIR)/msgs.o \
	$(SRCDIR)/simple_ini.o \
	$(SRCDIR)/debugger_bpoint.o \
	$(SRCDIR)/debugger_socket.o \
	$(SRCDIR)/debugger_symbol.o \
	$(SRCDIR)/ConvertUTF.o \
	$(SRCDIR)/utility.o

EMUOSDOBJS:=$(SRCOSDSDL)/sdl_emu.o \
	$(SRCOSDSDL)/sdl_sound.o \
	$(SRCOSDSDL)/sdl_timer.o \
	$(SRCOSDSDL)/sdl_debugger_console.o \
	$(SRCOSDSDL)/sdl_socket.o \
	$(SRCOSDSDL)/sdl_msgboard.o \
	$(SRCOSDSDL)/sdl_parseopt.o \
	$(SRCOSDSDL)/sdl_ledboxbase.o \
	$(SRCOSDSDL)/sdl_vkeyboardbase.o \
	$(SRCOSDSDL)/sdl_csurface.o \
	$(SRCOSDSDL)/sdl_cbitmap.o \
	$(SRCOSDSDL)/sdl_ccolor.o \
	$(SRCOSDLINUX)/linux_uart.o \
	$(SRCOSD)/d88_files.o \
	$(SRCOSD)/debugger_console.o \
	$(SRCOSD)/emu.o \
	$(SRCOSD)/emu_input.o \
	$(SRCOSD)/emu_input_keysym.o \
	$(SRCOSD)/emu_screen.o \
	$(SRCOSD)/emu_sound.o \
	$(SRCOSD)/keybind.o \
	$(SRCOSD)/logging.o \
	$(SRCOSD)/parseopt.o \
	$(SRCOSD)/screenmode.o \
	$(SRCOSD)/simple_clocale.o \
	$(SRCOSD)/vkeyboardbase.o \
	$(SRCOSD)/windowmode.o \
	$(SRCOSD)/opengl.o

ifeq ($(CORE_TYPE),CORE_TYPE_GTK_SDL)
  EMUDEPOBJS:=$(SRCOSDGTK)/gtk_main.o \
	$(SRCOSDGTK)/gtk_input.o \
	$(SRCOSDGTK)/gtk_input_keysym.o \
	$(SRCOSDGTK)/gtk_screenmode.o \
	$(SRCOSDGTK)/gtk_screen.o
else
  EMUDEPOBJS:=$(SRCOSDSDL)/sdl_main.o \
	$(SRCOSDSDL)/sdl_input.o \
	$(SRCOSDSDL)/sdl_input_keysym.o \
	$(SRCOSDSDL)/sdl_screenmode.o \
	$(SRCOSDSDL)/sdl_screen.o
endif

VMOBJS:=$(SRCVM)/device.o \
	$(SRCVM)/disk.o \
	$(SRCVM)/disk_parser.o \
	$(SRCVM)/event.o \
	$(SRCVM)/i8255.o \
	$(SRCVM)/harddisk.o \
	$(SRCVM)/mc68000.o \
	$(SRCVM)/mc68000cycs.o \
	$(SRCVM)/mc68000dasm.o \
	$(SRCVM)/mc68000fpu.o \
	$(SRCVM)/mc68000mmu.o \
	$(SRCVM)/mc68000ops.o \
	$(SRCVM)/noise.o \
	$(SRCVM)/parsewav.o \
	$(SRCVM)/paw_datas.o \
	$(SRCVM)/paw_defs.o \
	$(SRCVM)/paw_dft.o \
	$(SRCVM)/paw_file.o \
	$(SRCVM)/paw_format.o \
	$(SRCVM)/paw_param.o \
	$(SRCVM)/paw_parse.o \
	$(SRCVM)/paw_parsecar.o \
	$(SRCVM)/paw_parsewav.o \
	$(SRCVM)/paw_util.o \
	$(SRCVM)/sound_base.o \
	$(SRCVM)/ym2151.o \
	$(SRCVM)/debugger_base.o \
	$(SRCVM)/debugger.o

FMGENOBJS:=$(SRCFMGEN)/fmgen.o \
	$(SRCFMGEN)/fmtimer.o \
	$(SRCFMGEN)/opm.o \
	$(SRCFMGEN)/opna.o \
	$(SRCFMGEN)/psg.o
#

DEPOBJS:=$(SRCDEP)/adpcm.o \
	$(SRCDEP)/board.o \
	$(SRCDEP)/comm.o \
	$(SRCDEP)/crtc.o \
	$(SRCDEP)/display.o \
	$(SRCDEP)/dmac.o \
	$(SRCDEP)/fdc.o \
	$(SRCDEP)/floppy.o \
	$(SRCDEP)/joypad.o \
	$(SRCDEP)/keyboard.o \
	$(SRCDEP)/keyrecord.o \
	$(SRCDEP)/memory.o \
	$(SRCDEP)/mfp.o \
	$(SRCDEP)/mouse.o \
	$(SRCDEP)/printer.o \
	$(SRCDEP)/rtc.o \
	$(SRCDEP)/scc.o \
	$(SRCDEP)/sprite_bg.o \
	$(SRCDEP)/sysport.o \
	$(SRCDEP)/sxsi.o \
	$(SRCDEP)/sasi.o \
	$(SRCDEP)/scsi.o \
	$(SRCDEP)/x68000.o

GUIOBJSAGAR:=$(SRCGUIAGAR)/ag_gui_base.o \
	$(SRCGUIAGAR)/ag_gui_config.o \
	$(SRCGUIAGAR)/ag_dlg.o \
	$(SRCGUIAGAR)/ag_file_dlg.o \
	$(SRCGUIAGAR)/ag_volume_dlg.o \
	$(SRCGUIAGAR)/ag_config_dlg.o \
	$(SRCGUIAGAR)/ag_keybind_dlg.o \
	$(SRCGUIAGAR)/ag_recaud_dlg.o \
	$(SRCGUIAGAR)/ag_recvid_dlg.o \
	$(SRCGUIAGAR)/ag_seldrv_dlg.o \
	$(SRCGUIAGAR)/ag_dir_dlg.o \
	$(SRCGUIAGAR)/ag_gui.o \
	$(SRCGUIGTKX11)/x11_ledbox.o \
	$(SRCGUIGTKX11)/x11_vkeyboard.o \
	$(SRCGUIGTKX11)/gtk_x11_gui.o

GUIOBJSGTKX11=$(SRCGUIGTKX11)/gtk_x11_gui.o \
	$(SRCGUIGTKX11)/gtk_dialogbox.o \
	$(SRCGUIGTKX11)/gtk_configbox.o \
	$(SRCGUIGTKX11)/gtk_x11_key_trans.o \
	$(SRCGUIGTKX11)/gtk_keybindbox.o \
	$(SRCGUIGTKX11)/gtk_keybindctrl.o \
	$(SRCGUIGTKX11)/gtk_volumebox.o \
	$(SRCGUIGTKX11)/gtk_recaudbox.o \
	$(SRCGUIGTKX11)/gtk_recvidbox.o \
	$(SRCGUIGTKX11)/gtk_filebox.o \
	$(SRCGUIGTKX11)/gtk_folderbox.o \
	$(SRCGUIGTKX11)/gtk_aboutbox.o \
	$(SRCGUIGTKX11)/gtk_vkeyboard.o \
	$(SRCGUIGTKX11)/gtk_joysetbox.o \
	$(SRCGUIGTKX11)/gtk_hdtypebox.o \
	$(SRCGUIGTKX11)/gtk_loggingbox.o \
	$(SRCGUIGTKX11)/gtk_ledbox.o

VIDOBJSWAV:=$(SRCVIDWAV)/wav_rec_audio.o
# VIDOBJSFFM:=$(SRCVIDFFM)/ffm_loadlib.o $(SRCVIDFFM)/ffm_rec_base.o $(SRCVIDFFM)/ffm_rec_audio.o $(SRCVIDFFM)/ffm_rec_video.o
VIDOBJSPNG:=$(SRCVIDPNG)/png_rec_video.o $(SRCVIDPNG)/png_bitmap.o

# VIDOBJS:=$(VIDOBJSWAV) $(VIDOBJSFFM) $(VIDOBJSPNG) $(SRCVID)/rec_audio.o $(SRCVID)/rec_video.o
VIDOBJS:=$(VIDOBJSWAV) $(VIDOBJSPNG) $(SRCVID)/rec_audio.o $(SRCVID)/rec_video.o

ifeq ($(GUI_TYPE),GUI_TYPE_AGAR)
	GUIOBJS:=$(GUIOBJSAGAR)
endif
ifeq ($(GUI_TYPE),GUI_TYPE_GTK_X11)
	GUIOBJS:=$(GUIOBJSGTKX11)
endif

GUIOBJS:=$(GUIOBJS) $(SRCDIR)/gui/gui_base.o \
	$(SRCDIR)/gui/sdl2_ledbox.o \
	$(SRCDIR)/gui/gui_keybinddata.o

OBJS_BASE:=$(DEPOBJS) $(VMOBJS) $(FMGENOBJS) $(EMUOBJS) $(EMUOSDOBJS) $(EMUDEPOBJS) $(GUIOBJS) $(VIDOBJS)
DEPS_BASE:=$(OBJS_BASE:%.o=%.d)
OBJS:=$(OBJS_BASE:%=$(BUILDDIR)/%)
DEPS:=$(DEPS_BASE:%=$(BUILDDIR)/%)

MAKEFILEDEP:=$(BUILDDIR)/Makefile.dep

#
#
#

all: exe

exe: $(BUILDDIR) $(EXE)

depend: $(MAKEFILEDEP)

$(MAKEFILEDEP): $(BUILDDIR) $(DEPS)
	cat $(DEPS) > $@

$(EXE): $(OBJS)
	$(LD) -o $@ $(OBJS) $(LIBS) $(LDFLAGS)

$(BUILDDIR)/%.o: %.cpp
	$(CXX) $(CDEFS) $(CXXFLAGS) -c $< -o $@

$(BUILDDIR)/%.d: %.cpp
	-$(CXX) $(CDEFS) $(CXXFLAGS) -MM $< | sed 's#^.*\.o:#$@:#' | sed 's/\.d:/.o:/' > $@

$(BUILDDIR)/%.o: %.c
	$(CC) $(CDEFS) $(CFLAGS) -c $< -o $@

$(BUILDDIR)/%.d: %.c
	-$(CC) $(CDEFS) $(CFLAGS) -MM $< | sed 's#^.*\.o:#$@:#' | sed 's/\.d:/.o:/' > $@

install: exe
	mkdir -p $(INSTALLDIR)
	cp -p $(EXE) $(INSTALLDIR)
#	cp -p $(DATADIR)/?*.* $(INSTALLDIR)
	mkdir -p $(INSTALLDIR)/$(RESDIR)
	cp -p $(SRCRES)/*.* $(INSTALLDIR)/$(RESDIR)
	mkdir -p $(INSTALLDIR)/$(LOCALEDIR)
	cp -p $(SRCLOCALE)/*.xml $(INSTALLDIR)/$(LOCALEDIR)
	for i in $(LOCALEDIR)/*/*; do if [ -d $$i ]; then \
		mkdir -p $(INSTALLDIR)/$$i; cp -p $$i/*.mo $(INSTALLDIR)/$$i; \
	fi; done

$(BUILDDIR):
	mkdir -p $(BUILDDIR)/$(SRCFMGEN)
	mkdir -p $(BUILDDIR)/$(SRCDEP)
	mkdir -p $(BUILDDIR)/$(SRCOSDSDL)
	mkdir -p $(BUILDDIR)/$(SRCOSDGTK)
	mkdir -p $(BUILDDIR)/$(SRCOSDLINUX)
	mkdir -p $(BUILDDIR)/$(SRCGUIAGAR)
	mkdir -p $(BUILDDIR)/$(SRCGUIGTKX11)
	mkdir -p $(BUILDDIR)/$(SRCVIDWAV)
	mkdir -p $(BUILDDIR)/$(SRCVIDFFM)
	mkdir -p $(BUILDDIR)/$(SRCVIDPNG)

clean:
	rm -rf $(BUILDDIR)

include $(MAKEFILEDEP)
