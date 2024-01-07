# SHARP X68000 Emulator 'eCZ-600'

#### Copyright(C) Common Source Code Project, Sasaji 2011-2024 All Rights Reserved.

## Files

    docs/ .................. Documents
    source/
      include/ ............. Include files
      lib/ ................. Libraries
      locale/ .............. Localization(gettext)
        list.xml ........... Language list
        ja/ ................ Japanese
          LC_MESSAGES/
            x68000.po ...... Translation file
            x68000.mo ...... Translation file(Converted)
      src/ ................. Source files
        extra/ ............. Extra sources
        gui/ ............... GUI sources
          cocoa/ ........... Mac Cocoa
          gtk_x11/ ......... Gtk+
          windows/ ......... Windows GUI
        osd/ ............... OS dependency sources
          gtk/ ............. Gtk+
          linux/ ........... linux
          mac/ ............. Mac
          SDL/ ............. SDL
          windows/ ......... Windows
        res/ ............... Resource files
        video/ ............. Recording video and sound
          cocoa/ ........... Mac Cocoa
          libpng/ .......... LibPNG
          wave/ ............ Wave format
          windows/ ......... windows
        vm/ ................ VM main program sources
      patch/ ............... Patch files
        SDL-1.2.15-mac-keyboard.patch ...
                             Possible to use the fn key on SDL mac.
        SDL-2.0.8-mac-keyboard.patch ...
                             Possible to use the fn key on SDL2 mac.
      tools/ ............... Tools
      Eclipse/ ............. Eclipse project files
        sdl_linux/ ......... SDL linux
        sdl_win/ ........... SDL Pleiades(Eclipse Japanese ver.)
      VC++2010/
        x68000_sdl.vcxproj .. Project file for SDL VC++2010
        x68000.vcxproj ..... Project file for VC++2010
      VC++2019/
        x68000_sdl.vcxproj .. Project file for SDL VC++2019
        x68000.vcxproj ..... Project file for VC++2019
      Xcode/ ............... Project for Xcode
      Makefile.xxx ......... Makefiles
      README_SDL.md ........ SDL version document (Japanese)
      README_WIN.md ........ VC++ version document (Japanese)


## How to compile

 * [Windows(VC++)](src/source/README_WIN.md)

 * [SDL](src/source/README_SDL.md)


## Discraimer

* This software is freeware. However, the copyright is not waived.
  Sasaji has the copyright of the execution module. 
  Each author which created the source code also has the copyright.
* No warranty: We are not responsible for any damage caused by this software.
  Please use this software at your own risk.


------------------------------------------------------------------------------
  Sasaji (sasaji@s-sasaji.ddo.jp)
  http://s-sasaji.ddo.jp/bml3mk5/
  (Twitter: http://twitter.com/bml3mk5)
------------------------------------------------------------------------------

