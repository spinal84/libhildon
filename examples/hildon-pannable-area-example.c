/*
 * This file is a part of hildon examples
 *
 * Copyright (C) 2008 Nokia Corporation, all rights reserved.
 *
 * Author: Alberto Garcia <agarcia@igalia.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; version 2.1 of
 * the License, or (at your option) any later version.
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

#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include <gtk/gtk.h>
#include "hildon.h"

enum { TEXT_COLUMN, N_COLUMNS };

static void
on_button_clicked (GtkWidget *widget, gpointer data)
{
    g_debug ("Button %d clicked", GPOINTER_TO_INT (data));
}

int
main (int argc, char **args)
{
    int i;
    HildonProgram *program;
    GtkWidget *window, *tv, *panarea;
    GtkTreeViewColumn *col;
    GtkCellRenderer *renderer;
    GtkListStore *store;
    GtkVBox *vbox;

    gtk_init (&argc, &args);

    program = hildon_program_get_instance ();

    /* Create the main window */
    window = hildon_window_new ();
    hildon_program_add_window (program, HILDON_WINDOW (window));

    gtk_container_set_border_width (GTK_CONTAINER (window), 5);

    /* Create a VBox and pack some buttons */
    vbox = GTK_VBOX (gtk_vbox_new (FALSE, 1));
    for (i = 0; i < 30; i++) {
            gchar *label = g_strdup_printf ("Button number %d", i);
            GtkWidget *but = gtk_button_new_with_label (label);
            gtk_box_pack_start (GTK_BOX (vbox), but, TRUE, TRUE, 0);
            g_signal_connect (G_OBJECT (but), "clicked", G_CALLBACK (on_button_clicked), GINT_TO_POINTER (i));
            g_free (label);
    }

    /* Create a treeview */
    tv = gtk_tree_view_new ();
    renderer = gtk_cell_renderer_text_new ();
    col = gtk_tree_view_column_new_with_attributes ("Title", renderer, "text", TEXT_COLUMN, NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW(tv), col);

    /* Add some rows to the treeview */
    store = gtk_list_store_new (N_COLUMNS, G_TYPE_STRING);
    for (i = 0; i < 100; i++) {
            GtkTreeIter iter;
            gchar *label = g_strdup_printf ("Row number %d", i);
            gtk_list_store_append (store, &iter);
            gtk_list_store_set (store, &iter, TEXT_COLUMN, label, -1);
            g_free (label);
    }
    gtk_tree_view_set_model (GTK_TREE_VIEW (tv), GTK_TREE_MODEL (store));
    g_object_unref (store);

    /* Pack the treeview in the VBox */
    gtk_box_pack_start (GTK_BOX (vbox), tv, TRUE, TRUE, 0);

    /* Put everything in a pannable area */
    panarea = hildon_pannable_area_new ();
    hildon_pannable_area_add_with_viewport (HILDON_PANNABLE_AREA (panarea), GTK_WIDGET (vbox));
    gtk_container_add (GTK_CONTAINER (window), panarea);

    g_signal_connect (G_OBJECT (window), "delete_event", G_CALLBACK (gtk_main_quit), NULL);

    gtk_widget_show_all (GTK_WIDGET (window));

    gtk_main ();

    return 0;
}
