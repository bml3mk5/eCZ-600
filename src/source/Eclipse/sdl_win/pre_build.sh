#!/bin/sh

pwd

windres.exe ../../../src/res/windows/$1.rc -D$2 $3 -O coff -o $1.res
windres.exe ../../../src/res/windows/$1_gui.rc -D$2 $3 -O coff -o $1_gui.res

