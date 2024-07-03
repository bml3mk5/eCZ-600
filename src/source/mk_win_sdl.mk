#
# for Windows + MinGW + MSYS
#
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

# FFMPEGDIR:=/D/Devel/vc/ffmpeg-3.4.1-win64-dev/include

# COMMONCFLAGS:=-I/usr/local/include -I/usr/include -I$(FFMPEGDIR) -I./include
COMMONCFLAGS:=-I/usr/local/include -I/usr/include -I./include

CFLAGS:=$(CFLAGS) $(COMMONCFLAGS)
CXXFLAGS:=$(CXXFLAGS) $(COMMONCFLAGS)

EXEFILE:=x68000

DATADIR:=data

SRCDIR:=src
SRCOSD:=$(SRCDIR)/osd
SRCOSDSDL:=$(SRCOSD)/SDL
SRCOSDWIN:=$(SRCOSD)/windows
SRCVM:=$(SRCDIR)/vm
SRCFMGEN:=$(SRCVM)/fmgen
SRCDEP:=$(SRCVM)/x68000
SRCVID:=$(SRCDIR)/video
SRCVIDWAV:=$(SRCVID)/wave
SRCVIDVFW:=$(SRCVID)/vfw
SRCVIDFFM:=$(SRCVID)/ffmpeg
SRCVIDWIN:=$(SRCVID)/windows
SRCGUIAGAR:=$(SRCDIR)/gui/agar
SRCGUIWIN:=$(SRCDIR)/gui/windows
ifeq ($(GUI_TYPE),GUI_TYPE_AGAR)
	SRCGUI:=$(SRCGUIAGAR)
endif
ifeq ($(GUI_TYPE),GUI_TYPE_WINDOWS)
	SRCGUI:=$(SRCGUIWIN)
endif

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
	$(SRCOSDSDL)/sdl_input.o \
	$(SRCOSDSDL)/sdl_input_keysym.o \
	$(SRCOSDSDL)/sdl_screenmode.o \
	$(SRCOSDSDL)/sdl_screen.o \
	$(SRCOSDSDL)/sdl_timer.o \
	$(SRCOSDSDL)/sdl_debugger_console.o \
	$(SRCOSDSDL)/sdl_socket.o \
	$(SRCOSDSDL)/sdl_msgboard.o \
	$(SRCOSDSDL)/sdl_parseopt.o \
	$(SRCOSDSDL)/sdl_ledboxbase.o \
	$(SRCOSDSDL)/sdl_vkeyboardbase.o \
	$(SRCOSDSDL)/sdl_main.o \
	$(SRCOSDSDL)/sdl_csurface.o \
	$(SRCOSDSDL)/sdl_cbitmap.o \
	$(SRCOSDSDL)/sdl_ccolor.o \
	$(SRCOSDWIN)/win_uart.o \
	$(SRCOSDWIN)/win_midi.o \
	$(SRCOSD)/d88_files.o \
	$(SRCOSD)/debugger_console.o \
	$(SRCOSD)/emu.o \
	$(SRCOSD)/emu_input.o \
	$(SRCOSD)/emu_input_keysym.o \
	$(SRCOSD)/emu_screen.o \
	$(SRCOSD)/emu_sound.o \
	$(SRCOSD)/keybind.o \
	$(SRCOSD)/logging.o \
	$(SRCOSD)/osd_midi.o \
	$(SRCOSD)/parseopt.o \
	$(SRCOSD)/screenmode.o \
	$(SRCOSD)/simple_clocale.o \
	$(SRCOSD)/vkeyboardbase.o \
	$(SRCOSD)/windowmode.o \
	$(SRCOSD)/opengl.o

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
	$(SRCDEP)/midi.o \
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
	$(SRCGUIWIN)/win_ledbox.o \
	$(SRCGUIWIN)/win_vkeyboard.o \
	$(SRCGUIWIN)/win_gui.o

GUIOBJSWIN:=$(SRCGUIWIN)/win_dialogbox.o \
	$(SRCGUIWIN)/win_aboutbox.o \
	$(SRCGUIWIN)/win_filebox.o \
	$(SRCGUIWIN)/win_folderbox.o \
	$(SRCGUIWIN)/win_fontbox.o \
	$(SRCGUIWIN)/win_volumebox.o \
	$(SRCGUIWIN)/win_configbox.o \
	$(SRCGUIWIN)/win_keybindbox.o \
	$(SRCGUIWIN)/win_keybindctrl.o \
	$(SRCGUIWIN)/win_key_trans.o \
	$(SRCGUIWIN)/win_recaudbox.o \
	$(SRCGUIWIN)/win_recvidbox.o \
	$(SRCGUIWIN)/win_seldrvbox.o \
	$(SRCGUIWIN)/winfont.o \
	$(SRCGUIWIN)/win_ledbox.o \
	$(SRCGUIWIN)/win_vkeyboard.o \
	$(SRCGUIWIN)/win_joysetbox.o \
	$(SRCGUIWIN)/win_hdtypebox.o \
	$(SRCGUIWIN)/win_loggingbox.o \
	$(SRCGUIWIN)/win_midlatebox.o \
	$(SRCGUIWIN)/win_gui.o

VIDOBJSWAV:=$(SRCVIDWAV)/wav_rec_audio.o
# VIDOBJSVFW:=$(SRCVIDVFW)/vfw_rec_video.o
# VIDOBJSFFM:=$(SRCVIDFFM)/ffm_loadlib.o $(SRCVIDFFM)/ffm_rec_base.o $(SRCVIDFFM)/ffm_rec_audio.o $(SRCVIDFFM)/ffm_rec_video.o
VIDOBJSWIN:=$(SRCVIDWIN)/win_rec_video.o $(SRCVIDWIN)/win_bitmap.o

# VIDOBJS:=$(VIDOBJSWAV) $(VIDOBJSVFW) $(VIDOBJSFFM) $(VIDOBJSWIN) $(SRCVID)/rec_audio.o $(SRCVID)/rec_video.o
VIDOBJS:=$(VIDOBJSWAV) $(VIDOBJSWIN) $(SRCVID)/rec_audio.o $(SRCVID)/rec_video.o

ifeq ($(GUI_TYPE),GUI_TYPE_AGAR)
	GUIOBJS:=$(GUIOBJSAGAR)
endif
ifeq ($(GUI_TYPE),GUI_TYPE_WINDOWS)
	GUIOBJS:=$(GUIOBJSWIN)
endif

GUIOBJS:=$(GUIOBJS) $(SRCDIR)/gui/gui_base.o \
	$(SRCDIR)/gui/sdl2_ledbox.o \
	$(SRCDIR)/gui/gui_keybinddata.o

SRCWINRES:=$(SRCDIR)/$(RESDIR)/windows
WINRESOBJS:=$(SRCWINRES)/x68000.res
ifeq ($(GUI_TYPE),GUI_TYPE_WINDOWS)
	WINRESOBJS:=$(WINRESOBJS) $(SRCWINRES)/x68000_gui.res
endif

OBJS_BASE:=$(DEPOBJS) $(VMOBJS) $(FMGENOBJS) $(EMUOBJS) $(EMUOSDOBJS) $(GUIOBJS) $(VIDOBJS) $(WINRESOBJS)
DEPS_BASE:=$(DEPOBJS) $(VMOBJS) $(FMGENOBJS) $(EMUOBJS) $(EMUOSDOBJS) $(GUIOBJS) $(VIDOBJS) 
OBJS:=$(OBJS_BASE:%=$(BUILDDIR)/%)
DEPS:=$(DEPS_BASE:%.o=$(BUILDDIR)/%.d)

WINDRES:=windres.exe

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

$(BUILDDIR)/%.res: %.rc
	$(WINDRES) $< -O coff $(CDEFS) -o $@

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
	mkdir -p $(BUILDDIR)/$(SRCOSDWIN)
	mkdir -p $(BUILDDIR)/$(SRCGUIAGAR)
	mkdir -p $(BUILDDIR)/$(SRCGUIWIN)
	mkdir -p $(BUILDDIR)/$(SRCVIDWAV)
	mkdir -p $(BUILDDIR)/$(SRCVIDVFW)
	mkdir -p $(BUILDDIR)/$(SRCVIDFFM)
	mkdir -p $(BUILDDIR)/$(SRCVIDWIN)
	mkdir -p $(BUILDDIR)/$(SRCWINRES)

clean:
	rm -rf $(BUILDDIR)

include $(MAKEFILEDEP)
