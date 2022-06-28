iconsdir = $(datadir)/icons/hicolor/128x128/apps
icons_DATA = ../scalable/org.xfce.panel.cpuheatmap.svg
EXTRA_DIST = $(icons_DATA)

all: org.xfce.panel.cpuheatmap.png
	
org.xfce.panel.cpuheatmap.png: ../scalable/org.xfce.panel.cpuheatmap.svg
	inkscape -e org.xfce.panel.cpuheatmap.png -w 128 -h 128 -z ../scalable/org.xfce.panel.cpuheatmap.svg

