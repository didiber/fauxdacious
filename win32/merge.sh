#!/bin/sh

# Quick-and-dirty script for updating a Windows release folder

cd /C/aud-win32
for i in `find -type f` ; do
    if test -f /C/fauxdacious/win32/override/$i ; then
        cp /C/fauxdacious/win32/override/$i $i
    elif test -f /C/MinGW/$i ; then
        cp /C/MinGW/$i $i
    elif test -f /C/GTK/$i ; then
        cp /C/GTK/$i $i
    elif test -f /C/libs/$i ; then
        cp /C/libs/$i $i
    elif test -f /C/aud/$i ; then
        cp /C/aud/$i $i
    else
        echo Not found: $i
    fi
done

for i in `find -name *.dll` ; do strip -s $i ; done
for i in `find -name *.exe` ; do strip -s $i ; done

rm -rf /C/aud-win32/share/locale

cd /C/GTK
for i in `find ./share/locale -name gtk20.mo` ; do
    mkdir -p /C/aud-win32/${i%%/gtk20.mo}
    cp $i /C/aud-win32/$i
done

cd /C/aud
for i in `find ./share/locale -name fauxdacious.mo` ; do
    mkdir -p /C/aud-win32/${i%%/fauxdacious.mo}
    cp $i /C/aud-win32/$i
done
for i in `find ./share/locale -name fauxdacious-plugins.mo` ; do
    mkdir -p /C/aud-win32/${i%%/fauxdacious-plugins.mo}
    cp $i /C/aud-win32/$i
done
