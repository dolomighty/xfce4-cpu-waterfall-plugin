/*  properties.cc
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

#include "heatmap.h"
#include "properties.h"
#include "settings.h"

#include <libxfce4ui/libxfce4ui.h>
#include <math.h>
#include <vector>
#include "xfce4++/util.h"

struct CPUHeatmapOptions
{
    const Ptr<CPUHeatmap> base;

    GtkColorButton  *color_buttons[NUM_COLORS] = {};
    GtkBox          *hbox_in_terminal = NULL;
    GtkBox          *hbox_startup_notification = NULL;
    guint           timeout_id = 0;

    CPUHeatmapOptions(const Ptr<CPUHeatmap> &_base) : base(_base) {}

    ~CPUHeatmapOptions() {
        g_info ("%s", __PRETTY_FUNCTION__);
        removeTimer();
    }

    void
    removeTimer() {
        if (timeout_id) {
            g_source_remove (timeout_id);
            timeout_id = 0;
        }
    }
};


static GtkBox*    create_tab ();
static GtkLabel*  create_label_line (GtkBox *tab, const gchar *text);
static GtkBox*    create_option_line (GtkBox *tab, GtkSizeGroup *sg, const gchar *name, const gchar *tooltip);
static GtkBox*    create_check_box (GtkBox *tab, GtkSizeGroup *sg, const gchar *name, bool init,
                                    GtkToggleButton **out_checkbox, const std::function<void (GtkToggleButton*)> &callback);
static GtkWidget* create_drop_down (GtkBox *tab, GtkSizeGroup *sg, const gchar *name,
                                    const std::vector<std::string> &items, size_t init,
                                    const std::function<void(GtkComboBox*)> &callback);
static void       setup_update_interval_option (GtkBox *vbox, GtkSizeGroup *sg, const Ptr<CPUHeatmapOptions> &data);
static void       setup_size_option (GtkBox *vbox, GtkSizeGroup *sg, XfcePanelPlugin *plugin, const Ptr<CPUHeatmap> &base);
static void       setup_command_option (GtkBox *vbox, GtkSizeGroup *sg, const Ptr<CPUHeatmapOptions> &data);
static void       setup_color_option (GtkBox *vbox, GtkSizeGroup *sg, const Ptr<CPUHeatmapOptions> &data,
                                      CPUHeatmapColorNumber number, const gchar *name, const gchar *tooltip,
                                      const std::function<void(GtkColorButton*)> &callback);
static void       setup_mode_option (GtkBox *vbox, GtkSizeGroup *sg, const Ptr<CPUHeatmapOptions> &data);
static void       change_color (GtkColorButton  *button, const Ptr<CPUHeatmap> &base, CPUHeatmapColorNumber number);
static void       update_sensitivity (const Ptr<CPUHeatmapOptions> &data, bool initial = false);



void
create_options (XfcePanelPlugin *plugin, const Ptr<CPUHeatmap> &base)
{
    xfce_panel_plugin_block_menu (plugin);

    GtkWidget *dlg = xfce_titled_dialog_new_with_mixed_buttons (
        _("CPU Heatmap Properties"),
        GTK_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET (plugin))),
        GTK_DIALOG_DESTROY_WITH_PARENT,
        "window-close-symbolic",
        _("_Close"),
        GTK_RESPONSE_OK,
        NULL
    );

    auto dlg_data = xfce4::make<CPUHeatmapOptions>(base);

    xfce4::connect_destroy (dlg, [dlg_data](GtkWidget*) {
        dlg_data->removeTimer();
    });
    xfce4::connect_response (GTK_DIALOG (dlg), [base, dlg](GtkDialog*, gint response) {
        gtk_widget_destroy (dlg);
        xfce_panel_plugin_unblock_menu (base->plugin);
        write_settings (base->plugin, base);
    });

    gtk_window_set_icon_name (GTK_WINDOW (dlg), "org.xfce.panel.cpuheatmap");

    GtkSizeGroup *sg = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);

    GtkBox *vbox = create_tab ();
    setup_update_interval_option (vbox, sg, dlg_data);
    setup_size_option (vbox, sg, plugin, base);

    gtk_box_pack_start (vbox, gtk_separator_new (GTK_ORIENTATION_HORIZONTAL), FALSE, FALSE, BORDER/2);
    setup_command_option (vbox, sg, dlg_data);
    dlg_data->hbox_in_terminal = create_check_box (vbox, sg, _("Run in terminal"),
        base->command_in_terminal, NULL,
        [dlg_data](GtkToggleButton *button) {
            CPUHeatmap::set_in_terminal (dlg_data->base, gtk_toggle_button_get_active (button));
            update_sensitivity (dlg_data);
        });
    dlg_data->hbox_startup_notification = create_check_box (vbox, sg, _("Use startup notification"),
        base->command_startup_notification, NULL,
        [dlg_data](GtkToggleButton *button) {
            CPUHeatmap::set_startup_notification (dlg_data->base, gtk_toggle_button_get_active (button));
            update_sensitivity (dlg_data);
        });



    GtkBox *vbox2 = create_tab ();
    setup_color_option (vbox2, sg, dlg_data, BG_COLOR, _("Background:"), NULL, [base](GtkColorButton *button) {
        change_color (button, base, BG_COLOR);
    });
    setup_color_option (vbox2, sg, dlg_data, FG_COLOR1, _("Color 1:"), NULL, [base](GtkColorButton *button) {
        change_color (button, base, FG_COLOR1);
    });
    setup_color_option (vbox2, sg, dlg_data, FG_COLOR2, _("Color 2:"), NULL, [base](GtkColorButton *button) {
        change_color (button, base, FG_COLOR2);
    });
    setup_mode_option (vbox2, sg, dlg_data);



    gtk_box_pack_start (vbox2, gtk_separator_new (GTK_ORIENTATION_HORIZONTAL), FALSE, FALSE, BORDER/2);

    create_check_box (vbox2, sg, _("Show frame"), base->has_frame, NULL,
        [dlg_data](GtkToggleButton *button) {
            CPUHeatmap::set_frame (dlg_data->base, gtk_toggle_button_get_active (button));
//            update_sensitivity (dlg_data);
        });
    create_check_box (vbox2, sg, _("Show border"), base->has_border, NULL,
        [dlg_data](GtkToggleButton *button) {
            CPUHeatmap::set_border (dlg_data->base, gtk_toggle_button_get_active (button));
//            update_sensitivity (dlg_data);
        });

    create_check_box (vbox2, sg, _("Show average"), base->has_average, NULL,
        [dlg_data](GtkToggleButton *button) {
            CPUHeatmap::set_average (dlg_data->base, gtk_toggle_button_get_active (button));
//            update_sensitivity (dlg_data);
        });

    GtkWidget *notebook = gtk_notebook_new ();
    gtk_container_set_border_width (GTK_CONTAINER (notebook), BORDER - 2);
    gtk_notebook_append_page (GTK_NOTEBOOK (notebook), GTK_WIDGET (vbox2), gtk_label_new (_("Appearance")));
    gtk_notebook_append_page (GTK_NOTEBOOK (notebook), GTK_WIDGET (vbox), gtk_label_new (_("Advanced")));

    GtkWidget *content = gtk_dialog_get_content_area (GTK_DIALOG (dlg));
    gtk_container_add (GTK_CONTAINER (content), notebook);

    gtk_widget_show_all (notebook);
    update_sensitivity (dlg_data, true);
    gtk_widget_show (dlg);
}


static GtkBox *
create_tab ()
{
    GtkBox *tab = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, BORDER));
    gtk_container_set_border_width (GTK_CONTAINER (tab), BORDER);
    return tab;
}


static GtkLabel *
create_label_line (GtkBox *tab, const gchar *text)
{
    GtkBox *line = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, BORDER));
    gtk_box_pack_start (tab, GTK_WIDGET (line), FALSE, FALSE, 0);

    GtkLabel *label = GTK_LABEL (gtk_label_new (text));
    gtk_box_pack_start (line, GTK_WIDGET (label), FALSE, FALSE, 0);
    gtk_label_set_xalign (label, 0.0);
    gtk_label_set_yalign (label, 0.5);

    return label;
}


static GtkBox *
create_option_line (GtkBox *tab, GtkSizeGroup *sg, const gchar *name, const gchar *tooltip)
{
    GtkBox *line = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, BORDER));
    gtk_box_pack_start (tab, GTK_WIDGET (line), FALSE, FALSE, 0);

    if (name)
    {
        GtkBox *line2 = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
        GtkWidget *label = gtk_label_new (name);
        gtk_box_pack_start (line2, label, FALSE, FALSE, 0);
        gtk_label_set_xalign (GTK_LABEL (label), 0.0);
        gtk_label_set_yalign (GTK_LABEL (label), 0.5);
        if (tooltip)
        {
            GtkWidget *icon = gtk_image_new_from_icon_name ("gtk-help", GTK_ICON_SIZE_MENU);
            gtk_widget_set_tooltip_text (icon, tooltip);
            gtk_box_pack_start (line2, icon, FALSE, FALSE, BORDER);
        }
        gtk_size_group_add_widget (sg, GTK_WIDGET (line2));
        gtk_box_pack_start (line, GTK_WIDGET (line2), FALSE, FALSE, 0);
    }

    return line;
}


static GtkBox*
create_check_box (GtkBox *tab, GtkSizeGroup *sg, const gchar *name, bool init,
                  GtkToggleButton **out_checkbox,
                  const std::function<void (GtkToggleButton*)> &callback)
{
    GtkBox *hbox = create_option_line (tab, sg, NULL, NULL);

    GtkToggleButton *checkbox = GTK_TOGGLE_BUTTON (gtk_check_button_new_with_mnemonic (name));
    gtk_toggle_button_set_active (checkbox, init);
    gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET (checkbox), FALSE, FALSE, 0);
    xfce4::connect (GTK_TOGGLE_BUTTON (checkbox), "toggled", callback);

    if (out_checkbox)
        *out_checkbox = checkbox;

    return hbox;
}


static GtkWidget*
create_drop_down (GtkBox *tab, GtkSizeGroup *sg, const gchar *name,
                  const std::vector<std::string> &items, size_t init,
                  const std::function<void(GtkComboBox*)> &callback)
{
    GtkBox *hbox = create_option_line (tab, sg, name, NULL);

    GtkWidget *combo = gtk_combo_box_text_new ();
    for (const std::string &item : items)
        gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT (combo), NULL, item.c_str());
    gtk_combo_box_set_active (GTK_COMBO_BOX (combo), init);
    gtk_box_pack_start (GTK_BOX (hbox), combo, FALSE, FALSE, 0);

    xfce4::connect (GTK_COMBO_BOX (combo), "changed", callback);

    return combo;
}


static void
setup_update_interval_option (GtkBox *vbox, GtkSizeGroup *sg, const Ptr<CPUHeatmapOptions> &data)
{
    const std::vector<std::string> items = {
        _("~100ms"),
        _("~200ms"),
        _("~500ms"),
        _("~1s"),
        _("~2s")
    };

    create_drop_down (vbox, sg, _("Update Interval:"), items, data->base->update_interval,
        [data](GtkComboBox *combo) {
            CPUHeatmap::set_update_rate (data->base, (CPUHeatmapUpdateRate) gtk_combo_box_get_active (combo));
        });
}


static void
setup_size_option (GtkBox *vbox, GtkSizeGroup *sg, XfcePanelPlugin *plugin, const Ptr<CPUHeatmap> &base)
{
    GtkBox *hbox;
    if (xfce_panel_plugin_get_orientation (plugin) == GTK_ORIENTATION_HORIZONTAL)
        hbox = create_option_line (vbox, sg, _("Width:"), NULL);
    else
        hbox = create_option_line (vbox, sg, _("Height:"), NULL);

    GtkWidget *size = gtk_spin_button_new_with_range (MIN_SIZE, MAX_SIZE, 1);
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (size), base->size);
    gtk_box_pack_start (GTK_BOX (hbox), size, FALSE, FALSE, 0);
    xfce4::connect (GTK_SPIN_BUTTON (size), "value-changed", [base](GtkSpinButton *button) {
        CPUHeatmap::set_size (base, gtk_spin_button_get_value_as_int (button));
    });
}





static void
setup_command_option (GtkBox *vbox, GtkSizeGroup *sg, const Ptr<CPUHeatmapOptions> &data)
{
    GtkBox *hbox = create_option_line (vbox, sg, _("Associated command:"), NULL);

    GtkWidget *associatecommand = gtk_entry_new ();
    gtk_entry_set_text (GTK_ENTRY (associatecommand), data->base->command.c_str());
    gtk_entry_set_icon_from_icon_name (GTK_ENTRY (associatecommand),
                                       GTK_ENTRY_ICON_SECONDARY,
                                       "help-contents");
    auto tooltip = std::string() +
        _("The command to run when the plugin is left-clicked.") + "\n" +
        _("If not specified, it defaults to xfce4-taskmanager, htop or top.");
    gtk_entry_set_icon_tooltip_text (GTK_ENTRY (associatecommand),
                                     GTK_ENTRY_ICON_SECONDARY,
                                     tooltip.c_str());
    gtk_box_pack_start (GTK_BOX (hbox), associatecommand, FALSE, FALSE, 0);
    xfce4::connect (GTK_ENTRY (associatecommand), "changed", [data](GtkEntry *entry) {
        CPUHeatmap::set_command (data->base, gtk_entry_get_text (entry));
        update_sensitivity (data);
    });
}




static void
setup_color_option (GtkBox *vbox, GtkSizeGroup *sg, const Ptr<CPUHeatmapOptions> &data,
                    CPUHeatmapColorNumber number, const gchar *name, const gchar *tooltip,
                    const std::function<void(GtkColorButton*)> &callback)
{
    GtkBox *hbox = create_option_line (vbox, sg, name, tooltip);

    data->color_buttons[number] = xfce4::gtk_color_button_new (data->base->colors[number], true);
    gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET (data->color_buttons[number]), FALSE, FALSE, 0);

    xfce4::connect (data->color_buttons[number], "color-set", callback);
}


static void
setup_mode_option (GtkBox *vbox, GtkSizeGroup *sg, const Ptr<CPUHeatmapOptions> &data)
{

    const std::vector<std::string> items = {
        _("Disabled"),
        _("Heatmap"),
    };

    gint selected = 0;
    switch (data->base->mode)
    {
        case MODE_DISABLED: selected = 0; break;
        case MODE_HEATMAP:  selected = 1; break;
    }

    create_drop_down (vbox, sg, _("Mode:"), items, selected,
        [data](GtkComboBox *combo) {
            /* 'Disabled' mode was introduced in 1.1.0 as '-1'
             * for this reason we need to decrement the selected value */
            gint active = gtk_combo_box_get_active (combo);
            CPUHeatmapMode mode;

            switch (active)
            {
                case MODE_DISABLED:
                case MODE_HEATMAP:
                    mode = (CPUHeatmapMode) active;
                    break;
                default:
                    mode = MODE_HEATMAP;
            }

            CPUHeatmap::set_mode (data->base, mode);

            update_sensitivity (data);
        });
}



static void
change_color (GtkColorButton *button, const Ptr<CPUHeatmap> &base, CPUHeatmapColorNumber number)
{
    CPUHeatmap::set_color (base, number, xfce4::gtk_get_rgba (button));
}


static void
update_sensitivity (const Ptr<CPUHeatmapOptions> &data, bool initial)
{
    const Ptr<CPUHeatmap> base = data->base;
    const bool default_command = base->command.empty();

    gtk_widget_set_sensitive (GTK_WIDGET (data->hbox_in_terminal), !default_command);
    gtk_widget_set_sensitive (GTK_WIDGET (data->hbox_startup_notification), !default_command);

    if (initial)
    {
        gtk_widget_set_visible (GTK_WIDGET (data->hbox_in_terminal), !default_command);
        gtk_widget_set_visible (GTK_WIDGET (data->hbox_startup_notification), !default_command);
        return;
    }
    
    if (!default_command)
    {
        gtk_widget_set_visible (GTK_WIDGET (data->hbox_in_terminal), true);
        gtk_widget_set_visible (GTK_WIDGET (data->hbox_startup_notification), true);
        return;
    }
}


