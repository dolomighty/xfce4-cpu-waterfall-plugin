# xfce4-cpuheatmap-plugin

```
CPU Heatmap plugin for the Xfce4 panel.
This plugin displays a heatmap of all the CPU cores.

Stripped down version of the standard xfce4 cpu graph plugin.


2022-06-20 19:03:40
bugs fixati
aggiunto show/hide average bar




2022-06-20 14:54:10
listbox modalità non mostra nulla - fixed
l'intervallo di aggiornamento non è settato - fixed
colori di merda - fixed
sfondo transp - fixed
nessun comando associato - fixed

desiderata:
scelta direzione scrolling ←↓→↑
show/hide average bar

rimuovere disabled dalla listbox modalità... non serve ad una cippa
potrebbe esser riusata per la scroll dir









RELEASE:

sudo make uninstall

./autogen.sh --libdir=/usr/lib/x86_64-linux-gnu --datadir=/usr/share --disable-debug

make && sudo make install

sudo gtk-update-icon-cache -f -t /usr/share/icons/hicolor/

unset PANEL_DEBUG ; xfce4-panel -r







DEBUG:

sudo make uninstall

./autogen.sh --libdir=/usr/lib/x86_64-linux-gnu --datadir=/usr/share --enable-debug

make && sudo make install

xfce4-panel -q ; PANEL_DEBUG=1 xfce4-panel





make maintainer-clean




