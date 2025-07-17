## codex\_fauxdacious\_video\_v1

Dieser Prompt ermöglicht es, das **Video-Plugin** in Fauxdacious lückenlos und plattformübergreifend zu generieren.

````text
codex_fauxdacious_video_v1:

Erstelle ein neues Fauxdacious-Plugin namens „video" mit vollständiger Video-Integration und Untertitel-Support. Folge exakt dieser Anleitung:

1. Ordnerstruktur:
   - plugins/video/
     - plugin.mk
     - video-plugin.c
     - video-plugin.h
     - video-ui.c

2. Autotools-Integration:
   - In `configure.ac`:
     ```m4
     AC_ARG_ENABLE(video-plugin,
       AS_HELP_STRING([--enable-video-plugin],
         [Enable GStreamer-based video output plugin]),
       [enable_video_plugin=yes], [enable_video_plugin=no])

     if test "$enable_video_plugin" = yes; then
       PKG_CHECK_MODULES([VIDEO], [gstreamer-1.0 gstreamer-video-1.0])
       AC_DEFINE([ENABLE_VIDEO_PLUGIN], [1],
         [Enable video output plugin])
     fi
     ```
   - In `Makefile.am` (Root):
     ```makefile
     if ENABLE_VIDEO_PLUGIN
     SUBDIRS += plugins/video
     endif
     ```

3. plugin.mk (in `plugins/video`):
   ```makefile
   PLUGIN_NAME = video
   PLUGIN_SRCS = video-plugin.c video-ui.c
   PLUGIN_INCS = -I$(top_builddir)/src
   PLUGIN_CFLAGS = $(shell pkg-config --cflags gstreamer-1.0 gstreamer-video-1.0)
   PLUGIN_LDFLAGS = $(shell pkg-config --libs gstreamer-1.0 gstreamer-video-1.0)
   include $(top_srcdir)/buildsys.mk
````

4. Plugin-Initialisierung (`video-plugin.c`):

   ```c
   #include "fauxdcore/plugin.h"
   #ifdef ENABLE_VIDEO_PLUGIN
   #include <gst/gst.h>
   #endif

   static void *video_init(void) {
   #ifdef ENABLE_VIDEO_PLUGIN
       gst_init(NULL, NULL);
       plugin_register_output("video", video_output_open, video_output_close);
   #endif
       return NULL;
   }

   static void video_cleanup(void *handle) {
   #ifdef ENABLE_VIDEO_PLUGIN
       /* gst_deinit() bei Bedarf */
   #endif
   }

   FAUD_EXPORT struct PluginDescriptor const *get_plugin_descriptor(void) {
       static struct PluginDescriptor desc = {
           .name    = "Video Output",
           .version = "1.0",
           .init    = video_init,
           .cleanup = video_cleanup,
       };
       return &desc;
   }
   ```

5. Header-Datei (`video-plugin.h`):

   ```c
   #pragma once
   #ifdef ENABLE_VIDEO_PLUGIN
   int video_output_open(struct OutputHandle **out, const char *filename);
   void video_output_close(struct OutputHandle *out);
   #endif
   ```

6. UI-Binding und Rendering (`video-ui.c`):

   ```c
   #ifdef ENABLE_VIDEO_PLUGIN
   #include "video-plugin.h"
   #include <gst/gst.h>
   #include <gst/video/videooverlay.h>
   #include <gtk/gtk.h>

   struct VideoOutput {
       GstElement *pipeline;
       GtkWidget  *container;
   };

   int video_output_open(struct OutputHandle **out, const char *filename) {
       struct VideoOutput *vo = g_malloc0(sizeof(*vo));
       gchar *uri = g_strdup_printf("file://%s", filename);
       vo->pipeline = gst_parse_launch(
           g_strdup_printf("playbin uri=%s video-sink=gtksink name=vsink", uri), NULL);
       g_free(uri);

       GtkWidget *widget = plugin_create_video_container();
       vo->container = widget;
       GstElement *vs = gst_bin_get_by_name(GST_BIN(vo->pipeline), "vsink");
       GstVideoOverlay *overlay = GST_VIDEO_OVERLAY(vs);
       gst_video_overlay_set_window_handle(overlay,
           GDK_WINDOW_XID(gtk_widget_get_window(widget)));

       gst_element_set_state(vo->pipeline, GST_STATE_PLAYING);
       *out = (struct OutputHandle*)vo;
       return 0;
   }

   void video_output_close(struct OutputHandle *out) {
       struct VideoOutput *vo = (struct VideoOutput*)out;
       gst_element_set_state(vo->pipeline, GST_STATE_NULL);
       gst_object_unref(vo->pipeline);
       gtk_widget_destroy(vo->container);
       g_free(vo);
   }
   #endif
   ```

7. UI-Hooks in `src/gui/main-window.c`:

   * Toggle-Button „Video anzeigen“ für `plugin_create_video_container()`
   * Subtitle-Dropdown:

     ```c
     static void on_subtitle_select(GtkComboBox *cb, gpointer data) {
         const gchar *sub = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(cb));
         g_object_set(G_OBJECT(video_handle->pipeline), "suburi", sub, NULL);
     }
     ```

8. Build & Test:

   ```bash
   autoreconf -fi
   ./configure --prefix=/usr --enable-video-plugin
   make -j$(nproc)
   sudo make install
   ```

9. Debian-Packaging (`debian/control`):

   ```plain
   Package: fauxdacious-video-plugin
   Depends: gstreamer1.0-plugins-base, gstreamer1.0-plugins-good
   Description: GStreamer-based video output plugin for Fauxdacious
    Integrierte Videoausgabe mit Untertitelunterstützung mittels GStreamer.
   ```

```

---

### Bemerkung
- **Datei im Repo**: Lege diese Markdown-Datei an, damit Codex bei jedem Build stets die aktuellen Anweisungen hat.  
- **Änderungen**: Passe die Datei an, wenn sich der Build-Prozess oder Abhängigkeiten ändern.  
- **Codex-Verhalten**: Codex liest diese Datei bei jeder Anfrage neu ein und verwendet sie als Referenz, daher bleibt sie immer aktuell.

```
