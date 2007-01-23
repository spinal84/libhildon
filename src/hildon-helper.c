/*
 * This file is a part of hildon
 *
 * Copyright (C) 2005, 2006 Nokia Corporation, all rights reserved.
 *
 * Contact: Michael Dominic Kostrzewa <michael.kostrzewa@nokia.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; version 2.1 of
 * the License.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 */

/**
 * SECTION:hildon-helpers
 * @short_description: A collection of usefull utilities and functions.
 *
 */

#ifdef                                          HAVE_CONFIG_H
#include                                        <config.h>
#endif

#include                                        <gtk/gtk.h>
#include                                        "hildon-helper.h"

#define                                         HILDON_FINGER_PRESSURE_THRESHOLD 0.4

#define                                         HILDON_FINGER_BUTTON 8

#define                                         HILDON_FINGER_ALT_BUTTON 1

#define                                         HILDON_FINGER_ALT_MASK GDK_MOD4_MASK

#define                                         HILDON_FINGER_SIMULATE_BUTTON 2

struct                                          _HildonLogicalData
{
    GtkRcFlags rcflags;
    GtkStateType state;
    gchar *logicalcolorstring;
    gchar *logicalfontstring;
} typedef                                       HildonLogicalData;

static void 
hildon_change_style_recursive_from_ld           (GtkWidget *widget, 
                                                 GtkStyle *prev_style, 
                                                 HildonLogicalData *ld)
{
    g_assert (GTK_IS_WIDGET (widget));

    /* Change the style for child widgets */
    if (GTK_IS_CONTAINER (widget)) {
        GList *iterator = gtk_container_get_children (GTK_CONTAINER (widget));
        for (iterator = iterator; iterator != NULL; iterator = g_list_next (iterator))
            hildon_change_style_recursive_from_ld (GTK_WIDGET (iterator->data), prev_style, ld);
    }

    /* gtk_widget_modify_*() emit "style_set" signals, so if we got here from
       "style_set" signal, we need to block this function from being called
       again or we get into inifinite loop.

    FIXME: Compiling with gcc > 3.3 and -pedantic won't allow
    conversion between function and object pointers. GLib API however
    requires an object pointer for a function, so we have to work
    around this.
    See http://bugzilla.gnome.org/show_bug.cgi?id=310175
    */

    G_GNUC_EXTENSION
        g_signal_handlers_block_matched (G_OBJECT (widget), G_SIGNAL_MATCH_ID | G_SIGNAL_MATCH_FUNC,
                g_signal_lookup ("style_set", G_TYPE_FROM_INSTANCE (widget)),
                0, NULL,
                (gpointer) hildon_change_style_recursive_from_ld,
                NULL);

    if (ld->logicalcolorstring != NULL)
    {
        /* Changing logical color */
        GdkColor color;
        gtk_widget_ensure_style (widget);
        if (gtk_style_lookup_color (widget->style, ld->logicalcolorstring, &color) == TRUE)
            switch (ld->rcflags)
            {
                case GTK_RC_FG:
                    gtk_widget_modify_fg (widget, ld->state, &color);
                    break;

                case GTK_RC_BG:
                    gtk_widget_modify_bg (widget, ld->state, &color);
                    break;

                case GTK_RC_TEXT:
                    gtk_widget_modify_text (widget, ld->state, &color);
                    break;

                case GTK_RC_BASE:
                    gtk_widget_modify_base (widget, ld->state, &color);
                    break;

            } else {
                g_warning ("Failed to lookup '%s' color!", ld->logicalcolorstring);
            }
    }

    if (ld->logicalfontstring != NULL)
    {
        /* Changing logical font */
        GtkStyle *fontstyle = gtk_rc_get_style_by_paths (gtk_settings_get_default (), ld->logicalfontstring, NULL, G_TYPE_NONE);
        if (fontstyle != NULL)
        {
            PangoFontDescription *fontdesc = fontstyle->font_desc;

            if (fontdesc != NULL)
                gtk_widget_modify_font (widget, fontdesc);
        }
    }

    /* FIXME: Compilation workaround for gcc > 3.3 + -pedantic again */

    G_GNUC_EXTENSION
        g_signal_handlers_unblock_matched (G_OBJECT (widget), G_SIGNAL_MATCH_ID | G_SIGNAL_MATCH_FUNC,
                g_signal_lookup ("style_set", G_TYPE_FROM_INSTANCE (widget)),
                0, NULL,
                (gpointer) hildon_change_style_recursive_from_ld,
                NULL);
}

static void 
hildon_logical_data_free                        (HildonLogicalData *ld)
{
    g_return_if_fail (ld != NULL);

    if (ld->logicalcolorstring)
        g_free (ld->logicalcolorstring);

    if (ld->logicalfontstring)
        g_free (ld->logicalfontstring);

    g_free (ld);
}

/**
 * hildon_helper_event_button_is_finger:
 * @event: A @gtkeventbutton to check
 *
 * Checks if the given button event is a finger event.
 * 
 * return value : TRUE if the event is a button event.
 **/
gboolean 
hildon_helper_event_button_is_finger (GdkEventButton *event)
{
    gdouble pressure;

    if (gdk_event_get_axis ((GdkEvent*) event, GDK_AXIS_PRESSURE, &pressure) &&
        pressure > HILDON_FINGER_PRESSURE_THRESHOLD)
        return TRUE;

    if (event->button == HILDON_FINGER_BUTTON)
        return TRUE;

    if (event->button == HILDON_FINGER_ALT_BUTTON &&
        event->state & HILDON_FINGER_ALT_MASK)
        return TRUE;

    if (event->button == HILDON_FINGER_SIMULATE_BUTTON)
        return TRUE;

    return FALSE;
}

/**
 * hildon_helper_set_logical_font:
 * @widget : a @gtkwidget to assign this logical font for.
 * @logicalfontname : a gchar* with the logical font name to assign to the widget.
 * 
 * this function assigns a defined logical font to the @widget and all its child widgets.
 * it also connects to the "style_set" signal which will retrieve & assign the new font 
 * for the given logical name each time the theme is changed.
 * the returned signal id can be used to disconnect the signal. 
 * the previous signal (obtained by calling this function) is disconnected 
 * automatically and should not be used. 
 * 
 * return value : the signal id that is triggered every time theme is changed. 0 if font set failed.
 **/
gulong
hildon_helper_set_logical_font                  (GtkWidget *widget, 
                                                 const gchar *logicalfontname)
{
    HildonLogicalData *ld;
    gulong signum = 0;

    g_return_val_if_fail (GTK_IS_WIDGET (widget), 0);
    g_return_val_if_fail (logicalfontname != NULL, 0);

    ld = g_malloc (sizeof (HildonLogicalData));

    ld->rcflags = 0;
    ld->state = 0;
    ld->logicalcolorstring = NULL;
    ld->logicalfontstring = g_strdup(logicalfontname);

    /* Disconnects the previously connected signals. That calls the closure notify
     * and effectively disposes the allocated data (hildon_logical_data_free) */
    g_signal_handlers_disconnect_matched (G_OBJECT (widget), G_SIGNAL_MATCH_FUNC, 
            0, 0, NULL, 
            G_CALLBACK (hildon_change_style_recursive_from_ld), NULL);

    /* Change the font now */
    hildon_change_style_recursive_from_ld (widget, NULL, ld);

    /* Connect to "style_set" so that the font gets changed whenever theme changes. */
    signum = g_signal_connect_data (G_OBJECT (widget), "style_set",
            G_CALLBACK (hildon_change_style_recursive_from_ld),
            ld, (GClosureNotify) hildon_logical_data_free, 0);

    return signum;
}

/**
 * hildon_helper_set_logical_color:
 * @widget : A @GtkWidget to assign this logical font for.
 * @rcflags : @GtkRcFlags enumeration defining whether to assign to FG, BG, TEXT or BASE style.
 * @state : @GtkStateType indicating to which state to assign the logical color
 * @logicalcolorname : A gchar* with the logical font name to assign to the widget.
 * 
 * This function assigns a defined logical color to the @widget and all it's child widgets.
 * It also connects to the "style_set" signal which will retrieve & assign the new color 
 * for the given logical name each time the theme is changed.
 * The returned signal id can be used to disconnect the signal.
 * The previous signal (obtained by calling this function) is disconnected 
 * automatically and should not be used. 
 * 
 * Example : If the style you want to modify is bg[NORMAL] then set rcflags to GTK_RC_BG and state to GTK_STATE_NORMAL.
 * 
 * Return value : The signal id that is triggered every time theme is changed. 0 if color set failed.
 **/
gulong 
hildon_helper_set_logical_color                 (GtkWidget *widget, 
                                                 GtkRcFlags rcflags,
                                                 GtkStateType state, 
                                                 const gchar *logicalcolorname)
{
    HildonLogicalData *ld;
    gulong signum = 0;

    g_return_val_if_fail (GTK_IS_WIDGET (widget), 0);
    g_return_val_if_fail (logicalcolorname != NULL, 0);

    ld = g_malloc (sizeof (HildonLogicalData));

    ld->rcflags = rcflags;
    ld->state = state;
    ld->logicalcolorstring = g_strdup (logicalcolorname);
    ld->logicalfontstring = NULL;

    /* Disconnects the previously connected signals. That calls the closure notify
     * and effectively disposes the allocated data (hildon_logical_data_free) */
    g_signal_handlers_disconnect_matched (G_OBJECT (widget), G_SIGNAL_MATCH_FUNC, 
            0, 0, NULL, 
            G_CALLBACK (hildon_change_style_recursive_from_ld), NULL);

    /* Change the colors now */
    hildon_change_style_recursive_from_ld (widget, NULL, ld);

    /* Connect to "style_set" so that the colors gets changed whenever theme */
    signum = g_signal_connect_data (G_OBJECT (widget), "style_set",
            G_CALLBACK (hildon_change_style_recursive_from_ld),
            ld, (GClosureNotify) hildon_logical_data_free, 0);

    return signum;
}

