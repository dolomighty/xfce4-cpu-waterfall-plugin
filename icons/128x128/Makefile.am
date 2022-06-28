iconsdir = $(datadir)/icons/hicolor/128x128/apps
icons_DATA = org.xfce.panel.cpuwaterfall.png
EXTRA_DIST = $(icons_DATA)

all: org.xfce.panel.cpuwaterfall.png
	
org.xfce.panel.cpuwaterfall.png: ../scalable/org.xfce.panel.cpuwaterfall.svg
	inkscape -e org.xfce.panel.cpuwaterfall.png -w 128 -h 128 -z ../scalable/org.xfce.panel.cpuwaterfall.svg

