SHARED_LIB = ${LIB_PREFIX}fauxdgui${LIB_SUFFIX}
LIB_MAJOR = 6
LIB_MINOR = 0
LIB_PATCH = 0

SRCS = about.cc \
       confirm.cc \
       eq-preset.cc \
       equalizer.cc \
       file-opener.cc \
       gtk-compat.cc \
       images.c \
       ui_infoarea.cc \
       infopopup.cc \
       infowin.cc \
       init.cc \
       jump-to-time.cc \
       jump-to-track.cc \
       jump-to-track-cache.cc \
       list.cc \
       menu.cc \
       pixbufs.cc \
       playlists.cc \
       plugin-menu.cc \
       plugin-prefs.cc \
       plugin-view.cc \
       prefs-widget.cc \
       prefs-window.cc \
       preset-browser.cc \
       queue-manager.cc \
       scaled-image.cc \
       settings-portal.cc \
       search-select.cc \
       status.cc \
       urilist.cc \
       url-opener.cc \
       util.cc

INCLUDES = gtk-compat.h \
           libfauxdgui.h \
           libfauxdgui-gtk.h \
           ui_infoarea.h \
           list.h \
           menu.h

CLEAN = images.c images.h

include ../../buildsys.mk
include ../../extra.mk

includesubdir = libfauxdgui

LD = ${CXX}

CPPFLAGS := -I.. -I../.. \
            ${CPPFLAGS} \
            ${GLIB_CFLAGS} \
            ${GTK_CFLAGS} \
            ${SDL_CFLAGS}

CFLAGS += ${LIB_CFLAGS}

LIBS := -L../libfauxdcore -lfauxdcore \
        ${LIBS} ${SDL_LIBS} -lm \
        ${GLIB_LIBS} \
        ${GTK_LIBS}

pre-depend: images.c images.h

images.h: images.gresource.xml
	glib-compile-resources --sourcedir=../../images --generate-header --target=images.h images.gresource.xml
images.c: images.gresource.xml
	glib-compile-resources --sourcedir=../../images --generate-source --target=images.c images.gresource.xml
