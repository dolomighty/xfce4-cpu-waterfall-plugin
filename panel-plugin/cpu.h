/*  cpu.h
 *  Part of xfce4-cpuheatmap-plugin
 *
 *  Copyright (c) Alexander Nordfelth <alex.nordfelth@telia.com>
 *  Copyright (c) gatopeich <gatoguan-os@yahoo.com>
 *  Copyright (c) 2007-2008 Angelo Arrifano <miknix@gmail.com>
 *  Copyright (c) 2007-2008 Lidiriel <lidiriel@coriolys.org>
 *  Copyright (c) 2010 Florian Rivoal <frivoal@gmail.com>
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
#ifndef _XFCE_CPUHEATMAP_CPU_H_
#define _XFCE_CPUHEATMAP_CPU_H_

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <libxfce4panel/libxfce4panel.h>
#include <string>
#include <vector>
#include "xfce4++/util.h"

#include "os.h"

using xfce4::Ptr;
using xfce4::Ptr0;

#define BORDER 8
#define MAX_HISTORY_SIZE (100*1000)
#define MAX_SIZE 128
#define MIN_SIZE 10


enum CPUHeatmapMode
{
    MODE_DISABLED = 0,
    MODE_HEATMAP  = 1,
};


/* Number of milliseconds between updates */
enum CPUHeatmapUpdateRate
{
    RATE_100MS = 0,
    RATE_200MS = 1,
    RATE_500MS = 2,
    RATE_1S    = 3,
    RATE_2S    = 4,
};

enum CPUHeatmapColorNumber
{
    BG_COLOR   = 0,
    FG_COLOR1  = 1,
    FG_COLOR2  = 2,
    NUM_COLORS = 3,
};


struct CpuLoad
{
    gint64 timestamp; /* Microseconds since 1970-01-01 UTC, or zero */
    gfloat value;     /* Range: from 0.0 to 1.0 */
} __attribute__((packed));


struct CPUHeatmap
{
    /* GUI components */
    XfcePanelPlugin *plugin;
    GtkWidget *frame_widget;
    GtkWidget *draw_area;
    GtkWidget *box;
    GtkWidget *ebox;
    struct {
        /* Widget pointers are NULL if bars are disabled */
        GtkWidget *frame;
        GtkWidget *draw_area;
        GtkOrientation orientation;
    } bars;
    GtkWidget *tooltip_text;

    /* Settings */
    CPUHeatmapUpdateRate update_interval;
    guint                size;
    CPUHeatmapMode       mode;
    std::string          command;
    xfce4::RGBA          colors[NUM_COLORS];

    /* Boolean settings */
    bool command_in_terminal:1;
    bool command_startup_notification:1;
    bool has_border:1;
    bool has_frame:1;

    /* Runtime data */
    guint nr_cores;
    guint timeout_id;
    struct {
        gssize cap_pow2;            /* Capacity. A power of 2. */
        gssize size;                /* size <= cap_pow2 */
        gssize offset;              /* Circular buffer position. Range: from 0 to (cap_pow2 - 1) */
        std::vector<CpuLoad*> data; /* Circular buffers */
        gssize mask() const         { return cap_pow2 - 1; }
    } history;
    std::vector<CpuData> cpu_data;  /* size == nr_cores+1 */
    Ptr0<Topology> topology;
    CpuStats stats;

    ~CPUHeatmap();

    static void set_border               (const Ptr<CPUHeatmap> &base, bool border);
    static void set_color                (const Ptr<CPUHeatmap> &base, CPUHeatmapColorNumber number, const xfce4::RGBA &color);
    static void set_command              (const Ptr<CPUHeatmap> &base, const std::string &command);
    static void set_frame                (const Ptr<CPUHeatmap> &base, bool frame);
    static void set_in_terminal          (const Ptr<CPUHeatmap> &base, bool in_terminal);
    static void set_mode                 (const Ptr<CPUHeatmap> &base, CPUHeatmapMode mode);
    static void set_size                 (const Ptr<CPUHeatmap> &base, guint width);
    static void set_startup_notification (const Ptr<CPUHeatmap> &base, bool startup_notification);
    static void set_update_rate          (const Ptr<CPUHeatmap> &base, CPUHeatmapUpdateRate rate);
};

guint get_update_interval_ms (CPUHeatmapUpdateRate rate);

#endif /* _XFCE_CPUHEATMAP_CPU_H_ */
