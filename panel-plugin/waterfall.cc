/*  waterfall.cc
 *  Part of xfce4-cpuwaterfall-plugin
 *
 *  Copyright (c) Alexander Nordfelth <alex.nordfelth@telia.com>
 *  Copyright (c) gatopeich <gatoguan-os@yahoo.com>
 *  Copyright (c) 2007-2008 Angelo Arrifano <miknix@gmail.com>
 *  Copyright (c) 2007-2008 Lidiriel <lidiriel@coriolys.org>
 *  Copyright (c) 2010 Florian Rivoal <frivoal@gmail.com>
 *  Copyright (c) 2021 Jan Ziak <0xe2.0x9a.0x9b@xfce.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

/* The fixes file has to be included before any other #include directives */
#include "xfce4++/util/fixes.h"
#include "waterfall.h"
#include "settings.h"
#include "draw_waterfall.h"
#include "plugin.h"
#include "properties.h"
#include <libxfce4ui/libxfce4ui.h>
#include <math.h>
#include "xfce4++/util.h"



using xfce4::PluginSize;
using xfce4::Propagation;
using xfce4::TooltipTime;



/* vim: !sort -k3 */
static void          about_cb       ();
static Propagation   command_cb     (GdkEventButton *event, const Ptr<CPUWaterfall> &base);
static Ptr<CPUWaterfall> create_gui   (XfcePanelPlugin *plugin);
static Propagation   draw_area_cb   (cairo_t *cr, const Ptr<CPUWaterfall> &base);
static void          mode_cb        (XfcePanelPlugin *plugin, const Ptr<CPUWaterfall> &base);
static void          shutdown       (const Ptr<CPUWaterfall> &base);
static PluginSize    size_cb        (XfcePanelPlugin *plugin, guint size, const Ptr<CPUWaterfall> &base);
static TooltipTime   tooltip_cb     (GtkTooltip *tooltip, const Ptr<CPUWaterfall> &base);
static void          update_tooltip (const Ptr<CPUWaterfall> &base);



void
cpuwaterfall_construct (XfcePanelPlugin *plugin)
{
    xfce_textdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR, "UTF-8");

    Ptr<CPUWaterfall> base = create_gui (plugin);

    read_settings (plugin, base);

    xfce_panel_plugin_menu_show_about (plugin);
    xfce_panel_plugin_menu_show_configure (plugin);

    xfce4::connect_about           (plugin, [base](XfcePanelPlugin *p) { about_cb(); });
    xfce4::connect_free_data       (plugin, [base](XfcePanelPlugin *p) { shutdown(base); });
    xfce4::connect_save            (plugin, [base](XfcePanelPlugin *p) { write_settings(p, base); });
    xfce4::connect_configure_plugin(plugin, [base](XfcePanelPlugin *p) { create_options(p, base); });
    xfce4::connect_mode_changed    (plugin, [base](XfcePanelPlugin *p, XfcePanelPluginMode mode) { mode_cb(p, base); });
    xfce4::connect_size_changed    (plugin, [base](XfcePanelPlugin *p, guint size) { return size_cb(p, size, base); });
}



static guint
init_cpu_data (std::vector<CpuData> &data)
{
    guint cpuNr = detect_cpu_number ();
    if (cpuNr != 0)
        data.resize(cpuNr+1);
    return cpuNr;
}



static Ptr<CPUWaterfall>
create_gui (XfcePanelPlugin *plugin)
{
    GtkWidget *frame, *ebox;
    GtkOrientation orientation;
    auto base = xfce4::make<CPUWaterfall>();

    orientation = xfce_panel_plugin_get_orientation (plugin);
    if ((base->nr_cores = init_cpu_data (base->cpu_data)) == 0)
        fprintf (stderr,"Cannot init cpu data !\n");

    /* Read CPU data twice in order to initialize
     * cpu_data[].previous_used and cpu_data[].previous_total
     * with the current HWMs. HWM = High Water Mark. */
    read_cpu_data (base->cpu_data);
    read_cpu_data (base->cpu_data);

    base->topology = read_topology ();

    base->plugin = plugin;

    base->ebox = ebox = gtk_event_box_new ();
    gtk_event_box_set_visible_window (GTK_EVENT_BOX (ebox), FALSE);
    gtk_event_box_set_above_child (GTK_EVENT_BOX (ebox), TRUE);
    gtk_container_add (GTK_CONTAINER (plugin), ebox);
    xfce_panel_plugin_add_action_widget (plugin, ebox);
    xfce4::connect_button_press (ebox, [base](GtkWidget*, GdkEventButton *event) -> Propagation {
        return command_cb (event, base);
    });

    base->box = gtk_box_new (orientation, 0);
    gtk_container_add (GTK_CONTAINER (ebox), base->box);
    gtk_widget_set_has_tooltip (base->box, TRUE);
    xfce4::connect_query_tooltip (base->box, [base](GtkWidget *widget, gint x, gint y, bool keyboard, GtkTooltip *tooltip) {
        return tooltip_cb (tooltip, base);
    });

    base->frame_widget = frame = gtk_frame_new (NULL);
    gtk_box_pack_end (GTK_BOX (base->box), frame, TRUE, TRUE, 2);

    base->draw_area = gtk_drawing_area_new ();
    gtk_container_add (GTK_CONTAINER (frame), GTK_WIDGET (base->draw_area));
    xfce4::connect_after_draw (base->draw_area, [base](cairo_t *cr) { return draw_area_cb (cr, base); });

    mode_cb (plugin, base);
    gtk_widget_show_all (ebox);

    base->tooltip_text = gtk_label_new (NULL);
    g_object_ref (base->tooltip_text);

    return base;
}



static void
about_cb ()
{
    const gchar *auth[] = {
        // awk -v QQ='"' '{ print QQ $0 QQ "," }' ../AUTHORS
        "Agustin Ferrin Pozuelo <gatoguan-os@yahoo.com>",
        "Alexander Nordfelth <alex.nordfelth@telia.com>",
        "Andrea Villa <dolomighty74@gmail.com>",
        "Angelo Miguel Arrifano <miknix@gmail.com>",
        "Florian Rivoal <frivoal@gmail.com>",
        "Jan Ziak <0xe2.0x9a.0x9b@xfce.org>",
        "Ludovic Mercier <lidiriel@coriolys.org>",
        "Peter Tribble <peter.tribble@gmail.com>",
        NULL
    };

    gtk_show_about_dialog (NULL,
        "logo-icon-name", "org.xfce.panel.cpuwaterfall",
        "license", xfce_get_license_text (XFCE_LICENSE_TEXT_GPL),
        "version", PACKAGE_VERSION,
        "program-name", PACKAGE_NAME,
        "comments", _("Graphical representation of the CPU load"),
        "website", "TBD",
        "copyright", _("Copyright (c) 2003-2022\n"),
        "authors", auth, NULL);
}



static void
ebox_revalidate (const Ptr<CPUWaterfall> &base)
{
    gtk_event_box_set_above_child (GTK_EVENT_BOX (base->ebox), FALSE);
    gtk_event_box_set_above_child (GTK_EVENT_BOX (base->ebox), TRUE);
}



CPUWaterfall::~CPUWaterfall()
{
    g_info ("%s", __PRETTY_FUNCTION__);
    for (auto hist_data : history.data)
        g_free (hist_data);
}



static void
shutdown (const Ptr<CPUWaterfall> &base)
{
    gtk_widget_destroy (base->ebox);
    base->ebox = NULL;
    g_object_unref (base->tooltip_text);
    base->tooltip_text = NULL;
    if (base->timeout_id)
    {
        g_source_remove (base->timeout_id);
        base->timeout_id = 0;
    }
}



static void
queue_draw (const Ptr<CPUWaterfall> &base)
{
    if (base->mode != MODE_DISABLED)
        gtk_widget_queue_draw (base->draw_area);
}



static void
resize_history (const Ptr<CPUWaterfall> &base, gssize history_size)
{
    const guint fastest = get_update_interval_ms (RATE_100MS);
    const guint slowest = get_update_interval_ms (RATE_2S);
    const gssize old_cap_pow2 = base->history.cap_pow2;

    gssize cap_pow2 = 1;
    while (cap_pow2 < MAX_SIZE * slowest / fastest)
        cap_pow2 <<= 1;
    while (cap_pow2 < history_size * slowest / fastest)
        cap_pow2 <<= 1;

    if (cap_pow2 != old_cap_pow2)
    {
        const std::vector<CpuLoad*> old_data = std::move(base->history.data);
        const gssize old_mask = base->history.mask();
        const gssize old_offset = base->history.offset;

        base->history.cap_pow2 = cap_pow2;
        base->history.data.resize(base->nr_cores + 1);
        base->history.offset = 0;
        for (guint core = 0; core < base->nr_cores + 1; core++)
        {
            base->history.data[core] = (CpuLoad*) g_malloc0 (cap_pow2 * sizeof (CpuLoad));
            if (!old_data.empty())
            {
                for (gssize i = 0; i < old_cap_pow2 && i < cap_pow2; i++)
                    base->history.data[core][i] = old_data[core][(old_offset + i) & old_mask];
                g_free (old_data[core]);
            }
        }

        xfce4::trim_memory ();
    }

    base->history.size = history_size;
}



static PluginSize
size_cb (XfcePanelPlugin *plugin, guint plugin_size, const Ptr<CPUWaterfall> &base)
{
    gint frame_h, frame_v, size;
    gssize history;
    GtkOrientation orientation;
    guint border_width;
    const gint shadow_width = base->has_frame ? 2*1 : 0;

    size = base->size;

    orientation = xfce_panel_plugin_get_orientation (plugin);

    if (orientation == GTK_ORIENTATION_HORIZONTAL)
    {
        frame_h = size + shadow_width;
        frame_v = plugin_size;
        history = base->size;
    }
    else
    {
        frame_h = plugin_size;
        frame_v = size + shadow_width;
        history = plugin_size;
    }

    if (G_UNLIKELY (history < 0 || history > MAX_HISTORY_SIZE))
        history = MAX_HISTORY_SIZE;

    if (history > base->history.cap_pow2)
        resize_history (base, history);
    else
        base->history.size = history;

    gtk_widget_set_size_request (GTK_WIDGET (base->frame_widget), frame_h, frame_v);

    if (base->has_border)
        border_width = (xfce_panel_plugin_get_size (base->plugin) > 26 ? 2 : 1);
    else
        border_width = 0;
    gtk_container_set_border_width (GTK_CONTAINER (base->box), border_width);

    base->set_border (base, base->has_border);

    return xfce4::RECTANGLE;
}




static void
mode_cb (XfcePanelPlugin *plugin, const Ptr<CPUWaterfall> &base)
{
    gtk_orientable_set_orientation (GTK_ORIENTABLE (base->box), xfce_panel_plugin_get_orientation (plugin));
    size_cb (plugin, xfce_panel_plugin_get_size (base->plugin), base);
}




static xfce4::TimeoutResponse
update_cb (const Ptr<CPUWaterfall> &base)
{
    if (!read_cpu_data (base->cpu_data))
        return xfce4::TIMEOUT_AGAIN;

    if (!base->history.data.empty())
    {
        const gint64 timestamp = g_get_real_time ();

        /* Prepend the current CPU load to the history */
        base->history.offset = (base->history.offset - 1) & base->history.mask();
        for (guint core = 0; core < base->nr_cores + 1; core++)
        {
            CpuLoad load;
            load.timestamp = timestamp;
            load.value = base->cpu_data[core].load;
            base->history.data[core][base->history.offset] = load;
        }
    }

    queue_draw (base);
    update_tooltip (base);

    return xfce4::TIMEOUT_AGAIN;
}



static void
update_tooltip (const Ptr<CPUWaterfall> &base)
{
    auto tooltip = xfce4::sprintf (_("Usage: %u%%"), (guint) roundf (base->cpu_data[0].load * 100));
    if (gtk_label_get_text (GTK_LABEL (base->tooltip_text)) != tooltip)
        gtk_label_set_text (GTK_LABEL (base->tooltip_text), tooltip.c_str());
}



static TooltipTime
tooltip_cb (GtkTooltip *tooltip, const Ptr<CPUWaterfall> &base)
{
    gtk_tooltip_set_custom (tooltip, base->tooltip_text);
    return xfce4::NOW;
}



static Propagation
draw_area_cb (cairo_t *cr, const Ptr<CPUWaterfall> &base)
{
    GtkAllocation alloc;
    gint w, h;
    void (*draw) (const Ptr<CPUWaterfall> &base, cairo_t *cr, gint w, gint h) = NULL;

    gtk_widget_get_allocation (base->draw_area, &alloc);
    w = alloc.width;
    h = alloc.height;

    switch (base->mode)
    {
        case MODE_DISABLED:
            break;
        case MODE_WATERFALL:
            draw = draw_waterfall;
            break;
    }

    if (draw)
    {
        if (!base->colors[BG_COLOR].isTransparent())
        {
            xfce4::cairo_set_source (cr, base->colors[BG_COLOR]);
            cairo_rectangle (cr, 0, 0, w, h);
            cairo_fill (cr);
        }
        draw (base, cr, w, h);
    }
    return xfce4::PROPAGATE;
}



static const gchar*
default_command (bool *in_terminal, bool *startup_notification)
{
    gchar *s = g_find_program_in_path ("xfce4-taskmanager");
    if (s)
    {
        g_free (s);
        *in_terminal = false;
        *startup_notification = true;
        return "xfce4-taskmanager";
    }

    *in_terminal = true;
    *startup_notification = false;

    s = g_find_program_in_path ("htop");
    if (s)
    {
        g_free (s);
        return "htop";
    }

    return "top";
}



static Propagation
command_cb (GdkEventButton *event, const Ptr<CPUWaterfall> &base)
{
    if (event->button == 1)
    {
        std::string command;
        bool in_terminal, startup_notification;

        if (!base->command.empty())
        {
            command = base->command;
            in_terminal = base->command_in_terminal;
            startup_notification = base->command_startup_notification;
        }
        else
        {
            command = default_command (&in_terminal, &startup_notification);
        }

        xfce_spawn_command_line_on_screen (gdk_screen_get_default (),
                                           command.c_str(), in_terminal,
                                           startup_notification, NULL);
    }
    return xfce4::STOP;
}



/**
 * get_update_interval_ms:
 *
 * Returns: update interval in milliseconds.
 */
guint
get_update_interval_ms (CPUWaterfallUpdateRate rate)
{
    switch (rate)
    {
        case RATE_100MS: return 100;
        case RATE_200MS: return 200;
        case RATE_500MS: return 500;
        case RATE_1S:    return 1000;
        case RATE_2S:    return 2000;
        default:         return 200;
    }
}



void
CPUWaterfall::set_startup_notification (const Ptr<CPUWaterfall> &base, bool startup_notification)
{
    base->command_startup_notification = startup_notification;
}



void
CPUWaterfall::set_in_terminal (const Ptr<CPUWaterfall> &base, bool in_terminal)
{
    base->command_in_terminal = in_terminal;
}



void
CPUWaterfall::set_command (const Ptr<CPUWaterfall> &base, const std::string &command)
{
    base->command = xfce4::trim (command);
}




void
CPUWaterfall::set_border (const Ptr<CPUWaterfall> &base, bool has_border)
{
    if (base->has_border != has_border)
    {
        base->has_border = has_border;
        size_cb (base->plugin, xfce_panel_plugin_get_size (base->plugin), base);
    }
}



void
CPUWaterfall::set_average (const Ptr<CPUWaterfall> &base, bool has_average )
{
    if (base->has_average != has_average)
    {
        base->has_average = has_average;
        size_cb (base->plugin, xfce_panel_plugin_get_size (base->plugin), base);
    }
}


void
CPUWaterfall::set_frame (const Ptr<CPUWaterfall> &base, bool has_frame)
{
    base->has_frame = has_frame;
    gtk_frame_set_shadow_type (GTK_FRAME (base->frame_widget), has_frame ? GTK_SHADOW_IN : GTK_SHADOW_NONE);
    if (base->bars.frame)
        gtk_frame_set_shadow_type (GTK_FRAME (base->bars.frame), has_frame ? GTK_SHADOW_IN : GTK_SHADOW_NONE);
    size_cb (base->plugin, xfce_panel_plugin_get_size (base->plugin), base);
}





void
CPUWaterfall::set_update_rate (const Ptr<CPUWaterfall> &base, CPUWaterfallUpdateRate rate)
{
    bool change = (base->update_interval != rate);
    bool init = (base->timeout_id == 0);

    if (change || init)
    {
        guint interval = get_update_interval_ms (rate);

        base->update_interval = rate;
        if (base->timeout_id)
            g_source_remove (base->timeout_id);
        base->timeout_id = xfce4::timeout_add (interval, [base]() { return update_cb(base); });

        if (change && !init)
            queue_draw (base);
    }
}



void
CPUWaterfall::set_size (const Ptr<CPUWaterfall> &base, guint size)
{
    if (G_UNLIKELY (size < MIN_SIZE))
        size = MIN_SIZE;
    if (G_UNLIKELY (size > MAX_SIZE))
        size = MAX_SIZE;

    base->size = size;
    size_cb (base->plugin, xfce_panel_plugin_get_size (base->plugin), base);
}




void
CPUWaterfall::set_mode (const Ptr<CPUWaterfall> &base, CPUWaterfallMode mode)
{
    base->mode = mode;
    if (mode == MODE_DISABLED)
    {
        gtk_widget_hide (base->frame_widget);
    }
    else
    {
        gtk_widget_show (base->frame_widget);
        ebox_revalidate (base);
    }
}



void
CPUWaterfall::set_color (const Ptr<CPUWaterfall> &base, CPUWaterfallColorNumber number, const xfce4::RGBA &color)
{
    if (!base->colors[number].equals(color))
    {
        base->colors[number] = color;
        queue_draw (base);
    }
}




