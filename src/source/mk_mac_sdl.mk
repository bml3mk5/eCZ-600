#
# for MacOSX  
#
# export GUI_TYPE
# export BUILDDIR
# export LIBS
# export LDFLAGS
# export CDEFS
# export CFLAGS
# export CXXFLAGS
# export INSTALLDIR
# export ARCH

CC:=gcc
CXX:=g++
LD:=g++

# FFMPEGDIR:=$(HOME)/Devel/ffmpeg

MACMINVER:=-mmacosx-version-min=10.9

# COMMONCFLAGS:=-I/usr/local/include -I/usr/X11/include -I$(FFMPEGDIR) -I./include
COMMONCFLAGS:=-I/usr/local/include -I/usr/X11/include -I./include

COMMONLDFLAGS:=-lm -lz -lbz2 -liconv -Wl,-framework,OpenGL -Wl,-framework,Cocoa -Wl,-framework,Carbon -Wl,-framework,CoreAudio -Wl,-framework,IOKit -Wl,-framework,AudioUnit -Wl,-framework,ForceFeedback -Wl,-framework,QTKit -Wl,-framework,AVFoundation -Wl,-framework,CoreMedia -Wl,-framework,CoreVideo -Wl,-framework,AudioToolbox -Wl,-framework,Metal

CFLAGS:=$(CFLAGS) $(COMMONCFLAGS) $(MACMINVER)
CXXFLAGS:=$(CXXFLAGS) $(COMMONCFLAGS) $(MACMINVER)
LDFLAGS:=$(LDFLAGS) $(COMMONLDFLAGS) $(MACMINVER)
# OBJC_CFLAGS:=-fobjc-call-cxx-cdtors

EXEFILE:=x68000

DATADIR:=data

SRCDIR:=src
SRCOSD:=$(SRCDIR)/osd
SRCOSDSDL:=$(SRCOSD)/SDL
SRCOSDMAC:=$(SRCOSD)/mac
SRCVM:=$(SRCDIR)/vm
SRCFMGEN:=$(SRCVM)/fmgen
SRCDEP:=$(SRCVM)/x68000
SRCVID:=$(SRCDIR)/video
SRCVIDWAV:=$(SRCVID)/wave
SRCVIDQTKIT:=$(SRCVID)/qtkit
SRCVIDAVKIT:=$(SRCVID)/avkit
SRCVIDFFM:=$(SRCVID)/ffmpeg
SRCVIDCOCOA:=$(SRCVID)/cocoa
SRCGUIAGAR:=$(SRCDIR)/gui/agar
SRCGUICOCOA:=$(SRCDIR)/gui/cocoa
ifeq ($(GUI_TYPE),GUI_TYPE_AGAR)
	SRCGUI:=$(SRCGUIAGAR)
endif
ifeq ($(GUI_TYPE),GUI_TYPE_COCOA)
	SRCGUI:=$(SRCGUICOCOA)
endif

SRCRES:=$(SRCDIR)/res/common
MACRESDIR:=$(SRCDIR)/res/macosx

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
	$(SRCOSDMAC)/mac_uart.o \
	$(SRCOSD)/d88_files.o \
	$(SRCOSD)/debugger_console.o \
	$(SRCOSD)/emu.o \
	$(SRCOSD)/emu_input.o \
	$(SRCOSD)/emu_input_keysym.o \
	$(SRCOSD)/emu_screen.o \
	$(SRCOSD)/emu_sound.o \
	$(SRCOSD)/logging.o \
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
	$(SRCDEP)/mouse.o \
	$(SRCDEP)/printer.o \
	$(SRCDEP)/rtc.o \
	$(SRCDEP)/scc.o \
	$(SRCDEP)/sprite_bg.o \
	$(SRCDEP)/sysport.o \
	$(SRCDEP)/sasi.o \
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
	$(SRCGUICOCOA)/cocoa_ledbox.o \
	$(SRCGUICOCOA)/cocoa_vkeyboard.o \
	$(SRCGUICOCOA)/cocoa_gui.o

GUIOBJSCOCOA:=$(SRCGUICOCOA)/cocoa_gui.o \
	$(SRCGUICOCOA)/cocoa_basepanel.o \
	$(SRCGUICOCOA)/cocoa_volumepanel.o \
	$(SRCGUICOCOA)/cocoa_keybindctrl.o \
	$(SRCGUICOCOA)/cocoa_keybindpanel.o \
	$(SRCGUICOCOA)/cocoa_recaudpanel.o \
	$(SRCGUICOCOA)/cocoa_recvidpanel.o \
	$(SRCGUICOCOA)/cocoa_seldrvpanel.o \
	$(SRCGUICOCOA)/cocoa_key_trans.o \
	$(SRCGUICOCOA)/cocoa_fontpanel.o \
	$(SRCGUICOCOA)/cocoa_ledbox.o \
	$(SRCGUICOCOA)/cocoa_vkeyboard.o \
	$(SRCGUICOCOA)/cocoa_joysetpanel.o \
	$(SRCGUICOCOA)/cocoa_configpanel.o

#	$(SRCGUICOCOA)/cocoa_savedatarec.o \

VIDOBJSWAV:=$(SRCVIDWAV)/wav_rec_audio.o
# VIDOBJSQTKIT:=$(SRCVIDQTKIT)/qt_rec_video.o
# VIDOBJSAVKIT:=$(SRCVIDAVKIT)/avk_rec_common.o $(SRCVIDAVKIT)/avk_rec_audio.o $(SRCVIDAVKIT)/avk_rec_video.o
# VIDOBJSFFM:=$(SRCVIDFFM)/ffm_loadlib.o $(SRCVIDFFM)/ffm_rec_base.o $(SRCVIDFFM)/ffm_rec_audio.o $(SRCVIDFFM)/ffm_rec_video.o
VIDOBJSCOCOA:=$(SRCVIDCOCOA)/cocoa_rec_video.o $(SRCVIDCOCOA)/cocoa_bitmap.o

# VIDOBJS:=$(VIDOBJSWAV) $(VIDOBJSQTKIT) $(VIDOBJSAVKIT) $(VIDOBJSFFM) $(VIDOBJSCOCOA) $(SRCVID)/rec_audio.o $(SRCVID)/rec_video.o
VIDOBJS:=$(VIDOBJSWAV) $(VIDOBJSCOCOA) $(SRCVID)/rec_audio.o $(SRCVID)/rec_video.o

ifeq ($(GUI_TYPE),GUI_TYPE_AGAR)
	GUIOBJS:=$(GUIOBJSAGAR)
endif
ifeq ($(GUI_TYPE),GUI_TYPE_COCOA)
	GUIOBJS:=$(GUIOBJSCOCOA)
endif

GUIOBJS:=$(GUIOBJS) $(SRCDIR)/gui/gui_base.o \
	$(SRCDIR)/gui/sdl2_ledbox.o \
	$(SRCDIR)/gui/gui_keybinddata.o


OBJS_BASE:=$(DEPOBJS) $(VMOBJS) $(FMGENOBJS) $(EMUOBJS) $(EMUOSDOBJS) $(GUIOBJS) $(VIDOBJS)
DEPS_BASE:=$(DEPOBJS) $(VMOBJS) $(FMGENOBJS) $(EMUOBJS) $(EMUOSDOBJS) $(GUIOBJS) $(VIDOBJS)
OBJS:=$(OBJS_BASE:%=$(BUILDDIR)/%)
DEPS:=$(DEPS_BASE:%.o=$(BUILDDIR)/%.d)

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
	$(CXX) $(CDEFS) $(ARCH) $(CXXFLAGS) -c $< -o $@

$(BUILDDIR)/%.d: %.cpp
	-$(CXX) $(CDEFS) $(CXXFLAGS) -MM $< | sed 's#^.*\.o:#$@:#' | sed 's/\.d:/.o:/' > $@

$(BUILDDIR)/%.o: %.c
	$(CC) $(CDEFS) $(ARCH) $(CFLAGS) -c $< -o $@

$(BUILDDIR)/%.d: %.c
	-$(CC) $(CDEFS) $(CFLAGS) -MM $< | sed 's#^.*\.o:#$@:#' | sed 's/\.d:/.o:/' > $@

$(BUILDDIR)/%.o: %.mm
	$(CXX) $(CDEFS) $(ARCH) $(CXXFLAGS) $(OBJC_CFLAGS) -c $< -o $@

$(BUILDDIR)/%.d: %.mm
	-$(CXX) $(CDEFS) $(CXXFLAGS) -MM $< | sed 's#^.*\.o:#$@:#' | sed 's/\.d:/.o:/' > $@

$(BUILDDIR)/%.o: %.m
	$(CC) $(CDEFS) $(ARCH) $(CFLAGS) $(OBJC_CFLAGS) -c $< -o $@

$(BUILDDIR)/%.d: %.m
	-$(CC) $(CDEFS) $(CFLAGS) -MM $< | sed 's#^.*\.o:#$@:#' | sed 's/\.d:/.o:/' > $@

install: exe
	mkdir -p $(INSTALLDIR)/$(EXEDIR)
	cp -p $(EXE) $(INSTALLDIR)/$(EXEDIR)
	SetFile -t APPL $(INSTALLDIR)/$(EXEDIR)/$(EXEFILE)
#	cp -p $(DATADIR)/?*.* $(INSTALLDIR)/
	mkdir -p $(INSTALLDIR)/$(RESDIR)
	(cp -p $(SRCRES)/*.* $(INSTALLDIR)/$(RESDIR); exit 0)
	(cp -pR $(MACRESDIR)/ $(INSTALLDIR)/; exit 0)
	mkdir -p $(INSTALLDIR)/$(RESDIR)/$(LOCALEDIR)
	(cp -p $(SRCLOCALE)/*.xml $(INSTALLDIR)/$(RESDIR)/$(LOCALEDIR); exit 0)
	for i in $(SRCLOCALE)/*/*; do if [ -d $$i ]; then \
		mkdir -p $(INSTALLDIR)/$(RESDIR)/$$i; cp -p $$i/*.mo $(INSTALLDIR)/$(RESDIR)/$$i; \
	fi; done

$(BUILDDIR):
	mkdir -p $(BUILDDIR)/$(SRCFMGEN)
	mkdir -p $(BUILDDIR)/$(SRCDEP)
	mkdir -p $(BUILDDIR)/$(SRCOSDSDL)
	mkdir -p $(BUILDDIR)/$(SRCOSDMAC)
	mkdir -p $(BUILDDIR)/$(SRCGUIAGAR)
	mkdir -p $(BUILDDIR)/$(SRCGUICOCOA)
	mkdir -p $(BUILDDIR)/$(SRCVIDWAV)
	mkdir -p $(BUILDDIR)/$(SRCVIDQTKIT)
	mkdir -p $(BUILDDIR)/$(SRCVIDAVKIT)
	mkdir -p $(BUILDDIR)/$(SRCVIDFFM)
	mkdir -p $(BUILDDIR)/$(SRCVIDCOCOA)

clean:
	rm -rf $(BUILDDIR)

include $(MAKEFILEDEP)
