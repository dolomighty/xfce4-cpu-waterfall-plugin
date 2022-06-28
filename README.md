# xfce4-cpu-waterfall-plugin

    CPU Waterfall plugin for the Xfce4 panel.
    This plugin displays a waterfall of all the CPU cores.
    Derived from the standard xfce4 cpu graph plugin.



    2022-06-28 22:29:20
    rename heatmap→waterfall (come era al principio)
    nuova icona
    corretti problemi nella catena di gen/install delle icone
    rendering: separatore strips
    TODO: toploaders nel tooltip




    2022-06-20 19:03:40
    bugs fixati
    aggiunto show/hide average bar



    2022-06-20 14:54:10
    listbox -modalità- non mostra nulla - fixed
    l'intervallo di aggiornamento non è settato - fixed
    colori di merda - fixed
    sfondo transp - fixed
    nessun comando associato - fixed

    desiderata:
    scelta direzione scrolling ←↓→↑
    show/hide average bar

    rimuovere disabled dalla listbox modalità... non serve ad una cippa
    potrebbe esser riusata per la scroll dir








```

RELEASE:

sudo make uninstall

./autogen.sh --libdir=/usr/lib/x86_64-linux-gnu --datadir=/usr/share --disable-debug

make && sudo make install

unset PANEL_DEBUG ; xfce4-panel -r







DEBUG:

sudo make uninstall

./autogen.sh --libdir=/usr/lib/x86_64-linux-gnu --datadir=/usr/share --enable-debug

make && sudo make install

xfce4-panel -q ; PANEL_DEBUG=1 xfce4-panel





make maintainer-clean




