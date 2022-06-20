/*  settings.cc
 *  Part of xfce4-cpuheatmap-plugin
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

#include "settings.h"
#include "xfce4++/util.h"
#include <math.h>


static const xfce4::RGBA default_colors[NUM_COLORS] =
{
    [BG_COLOR]         = {1.0, 1.0, 1.0, 1.0},
    [FG_COLOR1]        = {0.0, 0.0, 0.0, 1.0},
    [FG_COLOR2]        = {1.0, 0.0, 0.0, 1.0},
};


static const gchar *const color_keys[NUM_COLORS] =
{
    [BG_COLOR]         = "Background",
    [FG_COLOR1]        = "Foreground1",
    [FG_COLOR2]        = "Foreground2",
};


void
read_settings (XfcePanelPlugin *plugin, const Ptr<CPUHeatmap> &base)
{
    // defaults
    CPUHeatmapUpdateRate rate = RATE_200MS;
    CPUHeatmapMode mode = MODE_HEATMAP;
    bool border = true;
    bool frame = false;

    xfce4::RGBA colors[NUM_COLORS];
    std::string command;
    bool in_terminal = true;
    bool startup_notification = false;

    for (guint i = 0; i < NUM_COLORS; i++)
        colors[i] = default_colors[i];

    gint size = xfce_panel_plugin_get_size (plugin);

    char *file;
    if ((file = xfce_panel_plugin_lookup_rc_file (plugin)) != NULL)
    {
        const auto rc = xfce4::Rc::simple_open (file, true);
        g_free (file);

        if (rc)
        {
            Ptr0<std::string> value;

            rate = (CPUHeatmapUpdateRate) rc->read_int_entry ("UpdateInterval", rate);
            size = rc->read_int_entry ("Size", size);
            frame = rc->read_int_entry ("Frame", frame);
            in_terminal = rc->read_int_entry ("InTerminal", in_terminal);
            startup_notification = rc->read_int_entry ("StartupNotification", startup_notification);
            border = rc->read_int_entry ("Border", border);

            if ((value = rc->read_entry ("Command", NULL))) {
                command = *value;
            }

            for (guint i = 0; i < NUM_COLORS; i++)
            {
                if ((value = rc->read_entry (color_keys[i], NULL)))
                {
                    xfce4::RGBA::parse (colors[i], *value);
                }
            }

            rc->close ();
        }
    }


    // Validate settings
    {
        switch (mode)
        {
            case MODE_DISABLED:
            case MODE_HEATMAP:
                break;
            default:
                mode = MODE_HEATMAP;
        }

        switch (rate)
        {
            case RATE_100MS:
            case RATE_200MS:
            case RATE_500MS:
            case RATE_1S:
            case RATE_2S:
                break;
            default:
                rate = RATE_200MS;
        }

        if (G_UNLIKELY (size <= 0))
            size = 10;
    }

    CPUHeatmap::set_border (base, border);
    for (guint i = 0; i < NUM_COLORS; i++){
        CPUHeatmap::set_color (base, (CPUHeatmapColorNumber) i, colors[i]);
    }
    CPUHeatmap::set_command (base, command);
    CPUHeatmap::set_in_terminal (base, in_terminal);
    CPUHeatmap::set_frame (base, frame);
    CPUHeatmap::set_mode (base, mode);
    CPUHeatmap::set_size (base, size);
    CPUHeatmap::set_startup_notification (base, startup_notification);
    CPUHeatmap::set_update_rate(base, rate);
}


void
write_settings (XfcePanelPlugin *plugin, const Ptr<const CPUHeatmap> &base)
{
    char *file;

    if (!(file = xfce_panel_plugin_save_location (plugin, TRUE)))
        return;

    const auto rc = xfce4::Rc::simple_open (file, false);
    g_free (file);
    file = NULL;

    if (!rc)
        return;

    rc->write_int_entry ("UpdateInterval", base->update_interval);
    rc->write_int_entry ("Mode", base->mode);
    rc->write_int_entry ("Size", base->size);
    rc->write_int_entry ("Frame", base->has_frame ? 1 : 0);
    rc->write_int_entry ("Border", base->has_border ? 1 : 0);
    rc->write_default_entry ("Command", base->command, "");
    rc->write_int_entry ("InTerminal", base->command_in_terminal ? 1 : 0);
    rc->write_int_entry ("StartupNotification", base->command_startup_notification ? 1 : 0);

    for (guint i=0; i<NUM_COLORS; i++)
    {
        const gchar *key = color_keys[i];

        if (key)
        {
            auto rgba = (std::string) base->colors[i];
            auto rgba_default = (std::string) default_colors[i];
            rc->write_default_entry (key, rgba, rgba_default);
        }
    }

    rc->close ();
}
