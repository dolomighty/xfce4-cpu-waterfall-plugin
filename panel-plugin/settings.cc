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
    [BG_COLOR]         = {1.0, 1.0, 1.0, 0.0},
    [FG_COLOR1]        = {0.0, 1.0, 0.0, 1.0},
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
    CPUHeatmapUpdateRate rate = RATE_NORMAL;
    CPUHeatmapMode mode = MODE_HEATMAP;
    guint color_mode = 0;
//    bool bars = true;
    bool border = true;
    bool frame = false;
//    bool highlight_smt = HIGHLIGHT_SMT_BY_DEFAULT;
//    bool nonlinear = false;
//    bool per_core = false;
//    guint per_core_spacing = PER_CORE_SPACING_DEFAULT;
//    guint tracked_core = 0;

    xfce4::RGBA colors[NUM_COLORS];
    std::string command;
    bool in_terminal = true;
    bool startup_notification = false;
    guint load_threshold = 0;

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
//            nonlinear = rc->read_int_entry ("TimeScale", nonlinear);
            size = rc->read_int_entry ("Size", size);
//            mode = (CPUHeatmapMode) rc->read_int_entry ("Mode", mode);
//            color_mode = rc->read_int_entry ("ColorMode", color_mode);
            frame = rc->read_int_entry ("Frame", frame);
            in_terminal = rc->read_int_entry ("InTerminal", in_terminal);
            startup_notification = rc->read_int_entry ("StartupNotification", startup_notification);
            border = rc->read_int_entry ("Border", border);
//            bars = rc->read_int_entry ("Bars", bars);
//            highlight_smt = rc->read_int_entry ("SmtIssues", highlight_smt);
//            per_core = rc->read_int_entry ("PerCore", per_core);
//            per_core_spacing = rc->read_int_entry ("PerCoreSpacing", per_core_spacing);
//            tracked_core = rc->read_int_entry ("TrackedCore", tracked_core);
            load_threshold = rc->read_int_entry ("LoadThreshold", load_threshold);

            if ((value = rc->read_entry ("Command", NULL))) {
                command = *value;
            }

            for (guint i = 0; i < NUM_COLORS; i++)
            {
                if ((value = rc->read_entry (color_keys[i], NULL)))
                {
                    xfce4::RGBA::parse (colors[i], *value);
//                    if (i == BARS_COLOR)
//                        base->has_barcolor = true;
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

//        if (mode == MODE_DISABLED && !bars)
//            mode = MODE_HEATMAP;

        switch (rate)
        {
            case RATE_FASTEST:
            case RATE_FASTER:
            case RATE_FAST:
            case RATE_NORMAL:
            case RATE_SLOW:
            case RATE_SLOWEST:
                break;
            default:
                rate = RATE_NORMAL;
        }

        if (G_UNLIKELY (size <= 0))
            size = 10;
    }

//    CPUHeatmap::set_bars (base, bars);
    CPUHeatmap::set_border (base, border);
    for (guint i = 0; i < NUM_COLORS; i++)
        CPUHeatmap::set_color (base, (CPUHeatmapColorNumber) i, colors[i]);
//    CPUHeatmap::set_color_mode (base, color_mode);
    CPUHeatmap::set_command (base, command);
    CPUHeatmap::set_in_terminal (base, in_terminal);
    CPUHeatmap::set_frame (base, frame);
    CPUHeatmap::set_load_threshold (base, load_threshold * 0.01f);
    CPUHeatmap::set_mode (base, mode);
//    CPUHeatmap::set_nonlinear_time (base, nonlinear);
//    CPUHeatmap::set_per_core (base, per_core);
//    CPUHeatmap::set_per_core_spacing (base, per_core_spacing);
    CPUHeatmap::set_size (base, size);
//    CPUHeatmap::set_smt (base, highlight_smt);
    CPUHeatmap::set_startup_notification (base, startup_notification);
//    CPUHeatmap::set_tracked_core (base, tracked_core);
//    CPUHeatmap::set_update_rate (base, rate);
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

    rc->write_default_int_entry ("UpdateInterval", base->update_interval, RATE_NORMAL);
//    rc->write_int_entry ("TimeScale", base->non_linear ? 1 : 0);
    rc->write_int_entry ("Size", base->size);
    rc->write_default_int_entry ("Mode", base->mode, MODE_HEATMAP);
    rc->write_int_entry ("Frame", base->has_frame ? 1 : 0);
    rc->write_int_entry ("Border", base->has_border ? 1 : 0);
//    rc->write_int_entry ("Bars", base->has_bars ? 1 : 0);
//    rc->write_int_entry ("PerCore", base->per_core ? 1 : 0);
//    rc->write_int_entry ("TrackedCore", base->tracked_core);
    rc->write_default_entry ("Command", base->command, "");
    rc->write_int_entry ("InTerminal", base->command_in_terminal ? 1 : 0);
    rc->write_int_entry ("StartupNotification", base->command_startup_notification ? 1 : 0);
//    rc->write_int_entry ("ColorMode", base->color_mode);
    rc->write_default_int_entry ("LoadThreshold", gint (roundf (100 * base->load_threshold)), 0);

    for (guint i = 0; i < NUM_COLORS; i++)
    {
        const gchar *key = color_keys[i];

//        if(i == BARS_COLOR && !base->has_barcolor)
//            key = NULL;

        if (key)
        {
            auto rgba = (std::string) base->colors[i];
            auto rgba_default = (std::string) default_colors[i];
            rc->write_default_entry (key, rgba, rgba_default);
        }
    }

//    rc->write_default_int_entry ("SmtIssues", base->highlight_smt ? 1 : 0, HIGHLIGHT_SMT_BY_DEFAULT);
//    rc->write_default_int_entry ("PerCoreSpacing", base->per_core_spacing, PER_CORE_SPACING_DEFAULT);

    rc->close ();
}
