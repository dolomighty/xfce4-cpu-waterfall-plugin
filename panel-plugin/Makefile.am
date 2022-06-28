plugindir = $(libdir)/xfce4/panel/plugins
plugin_LTLIBRARIES = libcpuwaterfall.la

AM_CPPFLAGS = -I${top_srcdir}

libcpuwaterfall_la_CFLAGS = \
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\" \
	@LIBXFCE4UI_CFLAGS@ \
	@LIBXFCE4PANEL_CFLAGS@

libcpuwaterfall_la_CXXFLAGS = $(libcpuwaterfall_la_CFLAGS)

libcpuwaterfall_la_LIBADD = \
	$(top_builddir)/xfce4++/util/libxfce4util_pp.la \
	@LIBXFCE4UI_LIBS@ \
	@LIBXFCE4PANEL_LIBS@ \
	-lm

libcpuwaterfall_la_SOURCES = \
	draw_waterfall.cc \
	draw_waterfall.h \
	waterfall.cc \
	waterfall.h \
	os.cc \
	os.h \
	plugin.h \
	plugin.c \
	properties.cc \
	properties.h \
	settings.cc \
	settings.h

libcpuwaterfall_la_LDFLAGS = \
	-avoid-version \
	-module \
	-no-undefined \
	-export-symbols-regex '^xfce_panel_module_(preinit|init|construct)' \
	$(PLATFORM_LDFLAGS)

#
# .desktop file
#
desktop_in_files = cpuwaterfall.desktop.in

desktopdir = $(datadir)/xfce4/panel/plugins
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

EXTRA_DIST = $(desktop_in_files)

DISTCLEANFILES = $(desktop_DATA)
