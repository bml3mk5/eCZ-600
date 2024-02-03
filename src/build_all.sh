#!/bin/sh

make_all() {
	if [ "$3" = "" ]; then
		make -f Makefile.$1 $2_clean
	fi
	make -f Makefile.$1 $2_install
}

svn update
cd source
UNAME=`uname -s`
UNAMEU=`echo $UNAME | cut -c 1-5`
if [ "$UNAME" = "Darwin" ]; then
	make_all mac_cocoa2 st $1
	make_all mac_cocoa2_dbgr st $1
fi
if [ "$UNAME" = "Linux" ]; then
	make_all linux_gtk_sdl2 sh $1
	make_all linux_gtk_sdl2_dbgr sh $1
fi
if [ "$UNAMEU" = "MINGW" ]; then
	make_all win_gui2 st $1
#	make_all win_gui2_dbgr st $1
fi

