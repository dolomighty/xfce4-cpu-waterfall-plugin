iconsdir = $(datadir)/icons/hicolor/16x16/apps
icons_DATA = org.xfce.panel.cpuwaterfall.png
EXTRA_DIST = $(icons_DATA)

all: org.xfce.panel.cpuwaterfall.png

org.xfce.panel.cpuwaterfall.png: ../scalable/org.xfce.panel.cpuwaterfall.svg
	inkscape -e org.xfce.panel.cpuwaterfall.png -w 16 -h 16 -z ../scalable/org.xfce.panel.cpuwaterfall.svg
