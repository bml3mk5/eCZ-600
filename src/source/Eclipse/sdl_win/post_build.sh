#!/bin/sh

pwd

DEST=.
RESDIR=res
RESSRC=$1/../../src/$RESDIR/common
LANGDIR=locale
LANGSRC=$1/../../$LANGDIR

mkdir -p $DEST/$RESDIR/
cp -p $RESSRC/*.* $DEST/$RESDIR/
mkdir -p $DEST/$LANGDIR/
cp -p $LANGSRC/*.xml $DEST/$LANGDIR/
for i in `ls -1 $LANGSRC`; do
  if [ -d $LANGSRC/$i/LC_MESSAGES ]; then
    mkdir -p $DEST/$LANGDIR/$i/LC_MESSAGES
    cp -p $LANGSRC/$i/LC_MESSAGES/*.mo $DEST/$LANGDIR/$i/LC_MESSAGES
  fi
done
