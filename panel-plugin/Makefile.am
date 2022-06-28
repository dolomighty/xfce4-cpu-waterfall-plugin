plugindir = $(libdir)/xfce4/panel/plugins
plugin_LTLIBRARIES = libcpuheatmap.la

AM_CPPFLAGS = -I${top_srcdir}

libcpuheatmap_la_CFLAGS = \
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\" \
	@LIBXFCE4UI_CFLAGS@ \
	@LIBXFCE4PANEL_CFLAGS@

libcpuheatmap_la_CXXFLAGS = $(libcpuheatmap_la_CFLAGS)

libcpuheatmap_la_LIBADD = \
	$(top_builddir)/xfce4++/util/libxfce4util_pp.la \
	@LIBXFCE4UI_LIBS@ \
	@LIBXFCE4PANEL_LIBS@ \
	-lm

libcpuheatmap_la_SOURCES = \
	draw_graph_heatmap.cc \
	draw_graph_heatmap.h \
	heatmap.cc \
	heatmap.h \
	os.cc \
	os.h \
	plugin.h \
	plugin.c \
	properties.cc \
	properties.h \
	settings.cc \
	settings.h

libcpuheatmap_la_LDFLAGS = \
	-avoid-version \
	-module \
	-no-undefined \
	-export-symbols-regex '^xfce_panel_module_(preinit|init|construct)' \
	$(PLATFORM_LDFLAGS)

#
# .desktop file
#
desktop_in_files = cpuheatmap.desktop.in

desktopdir = $(datadir)/xfce4/panel/plugins
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

EXTRA_DIST = $(desktop_in_files)

DISTCLEANFILES = $(desktop_DATA)
