/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Copyright (C) 2008 Guilhem Bonnefille <guilhem.bonnefille@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <config.h>
#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include <glade/glade.h>

#include "libgiggle/giggle-git.h"
#include "libgiggle/giggle-git-stash-list.h"
#include "libgiggle/giggle-git-stash-save.h"
#include "libgiggle/giggle-git-stash-show.h"
#include "libgiggle/giggle-git-stash-subcommand.h"
#include "libgiggle/giggle-git-stash-clear.h"
#include "giggle-diff-view.h"
#include "giggle-view-stash.h"

typedef struct GiggleViewStashPriv GiggleViewStashPriv;

struct GiggleViewStashPriv {
	GiggleGit *git;
	
	/* git stash list related */
	GiggleJob *list_job;
	GtkWidget *states_list;
	
	/* git stash save related */
	GiggleJob *save_job;
	GtkWidget *save_button;
	GtkWidget *save_entry;
	
	/* git stash clear related */
	GiggleJob *clear_job;
	GtkWidget *clear_button;

	/* git stash apply related */
	GtkWidget *apply_button;

	/* git stash pop related */
	GtkWidget *pop_button;

	/* git stash drop related */
	GtkWidget *drop_button;
	
	/* git stash subcommands */
	GiggleJob *subcommand_job;
	
	/* Displaying stash */
	GtkWidget *diff_view;
};

enum {
	COL_ID,
	COL_STASH
};

static void    view_stash_finalize              (GObject           *object);

static void    view_stash_project_changed_cb    (GObject           *object,
						 GParamSpec        *arg,
						 GiggleViewStash *view);
static void
view_stash_save (GtkButton *button, void *data);

static void
view_stash_save_job_callback (GiggleGit *git,
			      GiggleJob *job,
			      GError    *error,
			      gpointer   user_data);

static void
view_stash_clear (GtkButton *button, void *data);

static void
view_stash_clear_job_callback (GiggleGit *git,
			      GiggleJob *job,
			      GError    *error,
			      gpointer   user_data);

static void
view_stash_list (GiggleViewStash     *view);

static void
view_stash_list_job_callback (GiggleGit *git,
			      GiggleJob *job,
			      GError    *error,
			      gpointer   user_data);

static void
view_stash_list_selection_changed (GtkWidget *widget, gpointer label);

static void
view_stash_subcommand (GtkButton *button, void *data);

static void
view_stash_subcommand_job_callback (GiggleGit *git,
			            GiggleJob *job,
			            GError    *error,
			            gpointer   user_data);

static void
view_stash_display (GiggleViewStash *view,
		    gchar           *id);

static void
view_stash_display_job_callback (GiggleGit *git,
			            GiggleJob *job,
			            GError    *error,
			            gpointer   user_data);

G_DEFINE_TYPE (GiggleViewStash, giggle_view_stash, GIGGLE_TYPE_VIEW)

#define GET_PRIV(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), GIGGLE_TYPE_VIEW_STASH, GiggleViewStashPriv))

static void
giggle_view_stash_class_init (GiggleViewStashClass *class)
{
	GObjectClass *object_class = G_OBJECT_CLASS (class);

	object_class->finalize = view_stash_finalize;

	g_type_class_add_private (object_class, sizeof (GiggleViewStashPriv));
}

static void
view_stash_update_data (GiggleViewStash *view)
{
	GiggleViewStashPriv *priv;

	priv = GET_PRIV (view);

	g_debug(__FUNCTION__);
	
	view_stash_list(view);
}

static void
giggle_view_stash_init (GiggleViewStash *view)
{
	GiggleViewStashPriv *priv;
	GladeXML            *xml;

        GtkTreeViewColumn   *column;
        GtkCellRenderer     *cell_renderer;
	GtkTreeSelection    *selection;
	
	GtkWidget           *scrolled_window;
		
	priv = GET_PRIV (view);

	xml = glade_xml_new (GLADEDIR "/main-window.glade", "stash_vbox", NULL);

	gtk_box_pack_start_defaults (GTK_BOX (view),
				     glade_xml_get_widget (xml, "stash_vbox"));

	/* git stash list */
	priv->states_list = glade_xml_get_widget (xml, "states_list");
	/* Creation de la premiere colonne */
        cell_renderer = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes(_("Id"), cell_renderer,
							  "text", COL_ID,
							  NULL);
        gtk_tree_view_append_column(GTK_TREE_VIEW(priv->states_list), column);
	
        cell_renderer = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes(_("States"), cell_renderer,
							  "text", COL_STASH,
							  NULL);
        gtk_tree_view_append_column(GTK_TREE_VIEW(priv->states_list), column);

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(priv->states_list));
	gtk_tree_selection_set_mode (selection, GTK_SELECTION_SINGLE);
	g_signal_connect(selection, "changed", G_CALLBACK(view_stash_list_selection_changed), view);
	
	/* git stash save */
	priv->save_button = glade_xml_get_widget (xml, "save_button");
	priv->save_entry = glade_xml_get_widget (xml, "save_log");
	
	g_signal_connect(G_OBJECT(priv->save_button), "clicked", G_CALLBACK(view_stash_save), view);
	
	/* git stash clear */
	priv->clear_button = glade_xml_get_widget (xml, "clear_button");
	g_signal_connect(G_OBJECT(priv->clear_button), "clicked", G_CALLBACK(view_stash_clear), view);
	
	/* git stash apply */
	priv->apply_button = glade_xml_get_widget (xml, "apply_button");
	g_signal_connect(G_OBJECT(priv->apply_button), "clicked", G_CALLBACK(view_stash_subcommand), view);
	
	/* git stash pop */
	priv->pop_button = glade_xml_get_widget (xml, "pop_button");
	g_signal_connect(G_OBJECT(priv->pop_button), "clicked", G_CALLBACK(view_stash_subcommand), view);
	
	/* git stash drop */
	priv->drop_button = glade_xml_get_widget (xml, "drop_button");
	g_signal_connect(G_OBJECT(priv->drop_button), "clicked", G_CALLBACK(view_stash_subcommand), view);

	/* diff view */
	priv->diff_view = giggle_diff_view_new ();
	scrolled_window = glade_xml_get_widget (xml, "stash_diff_sw");
	
	gtk_container_add (GTK_CONTAINER (scrolled_window), priv->diff_view);
	gtk_widget_show_all (scrolled_window);

	g_object_unref (xml);

	priv->git = giggle_git_get ();
	g_signal_connect (priv->git, "notify::directory",
			  G_CALLBACK (view_stash_project_changed_cb), view);

	/* initialize for the first time */
	view_stash_update_data (view);
}

static void
view_stash_finalize (GObject *object)
{
	GiggleViewStashPriv *priv;

	priv = GET_PRIV (object);
}

static void
view_stash_project_changed_cb (GObject           *object,
				 GParamSpec        *arg,
				 GiggleViewStash *view)
{
	view_stash_update_data (view);
}

static void
view_stash_save (GtkButton *button,
                 void      *data)
{
	GiggleViewStash     *view = GIGGLE_VIEW_STASH(data);
	GiggleViewStashPriv *priv;
	const gchar         *log;

	priv = GET_PRIV (view);

	if (priv->save_job) {
		giggle_git_cancel_job (priv->git, priv->save_job);
		g_object_unref (priv->save_job);
		priv->save_job = NULL;
	}

	log = gtk_entry_get_text (GTK_ENTRY(priv->save_entry));

	priv->save_job = giggle_git_stash_save_new (log);

	giggle_git_run_job (priv->git,
			    priv->save_job,
			    view_stash_save_job_callback,
			    view);
}

static void
view_stash_save_job_callback (GiggleGit *git,
			      GiggleJob *job,
			      GError    *error,
			      gpointer   user_data)
{
	GiggleViewStashPriv *priv;

	priv = GET_PRIV (user_data);

	if (error) {
		GtkWidget *dialog;

		dialog = gtk_message_dialog_new (NULL,
						 GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
						 GTK_MESSAGE_ERROR,
						 GTK_BUTTONS_CLOSE,
						 _("An error ocurred when stash saving:\n%s"),
						 error->message);

		gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (dialog);
	} else {
		/* Tell GiggleGit listeners to update */
		giggle_git_changed (priv->git);
	}

	g_object_unref (priv->save_job);
	priv->save_job = NULL;
}

static void
view_stash_clear (GtkButton *button,
                  void      *data)
{
	GiggleViewStash     *view = GIGGLE_VIEW_STASH(data);
	GiggleViewStashPriv *priv;
	GtkWidget           *dialog;
	gint                 confirm;

	dialog = gtk_message_dialog_new(GTK_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET (view))),
			                GTK_DIALOG_DESTROY_WITH_PARENT,
                                        GTK_MESSAGE_WARNING,
                                        GTK_BUTTONS_OK_CANCEL,
                                        _("You are about to remove all the stashed states.\n"
					  "Note that those states will then be subject to pruning, and may be difficult or impossible to recover.\n"
					  "Do you want to continue?"));
	gtk_window_set_title(GTK_WINDOW(dialog), _("Clear stashed states"));
        confirm = gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);
	
	if (confirm != GTK_RESPONSE_OK)
		return;
	
	priv = GET_PRIV (view);

	if (priv->clear_job) {
		giggle_git_cancel_job (priv->git, priv->clear_job);
		g_object_unref (priv->clear_job);
		priv->clear_job = NULL;
	}

	priv->clear_job = giggle_git_stash_clear_new ();

	giggle_git_run_job (priv->git,
			    priv->clear_job,
			    view_stash_clear_job_callback,
			    view);
}

static void
view_stash_clear_job_callback (GiggleGit *git,
			       GiggleJob *job,
			       GError    *error,
			       gpointer   user_data)
{
	GiggleViewStashPriv *priv;

	priv = GET_PRIV (user_data);

	if (error) {
		GtkWidget *dialog;

		dialog = gtk_message_dialog_new (NULL,
						 GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
						 GTK_MESSAGE_ERROR,
						 GTK_BUTTONS_CLOSE,
						 _("An error ocurred when stash clearing:\n%s"),
						 error->message);

		gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (dialog);
	} else {
		/* Tell GiggleGit listeners to update */
		giggle_git_changed (priv->git);
	}

	g_object_unref (priv->clear_job);
	priv->clear_job = NULL;
}

static void
view_stash_list (GiggleViewStash     *view)
{
	GiggleViewStashPriv *priv;

	priv = GET_PRIV (view);

	if (priv->list_job) {
		giggle_git_cancel_job (priv->git, priv->list_job);
		g_object_unref (priv->list_job);
		priv->save_job = NULL;
	}

	priv->list_job = giggle_git_stash_list_new ();

	giggle_git_run_job (priv->git,
			    priv->list_job,
			    view_stash_list_job_callback,
			    view);
}

static void
view_stash_list_job_callback (GiggleGit *git,
			      GiggleJob *job,
			      GError    *error,
			      gpointer   user_data)
{
	GiggleViewStashPriv *priv;
	GiggleViewStash     *view;
	
	view = GIGGLE_VIEW_STASH (user_data);
	priv = GET_PRIV (view);

	if (error) {
		GtkWidget *dialog;

		dialog = gtk_message_dialog_new (NULL,
						 GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
						 GTK_MESSAGE_ERROR,
						 GTK_BUTTONS_CLOSE,
						 _("An error ocurred when stash clearing:\n%s"),
						 error->message);

		gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (dialog);
	} else {
		/* Insert elem */
		GtkListStore *list_store;
		GtkTreeIter   iter;
                GSList *entry;

		list_store = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_STRING);
		gtk_tree_view_set_model (GTK_TREE_VIEW(priv->states_list), GTK_TREE_MODEL(list_store));

		entry = giggle_git_stash_list_get_states (GIGGLE_GIT_STASH_LIST(priv->list_job));
                for (; entry != NULL; entry = g_slist_next(entry)) {
			gchar **tokens = g_strsplit(entry->data, ": ", 2);
			g_debug("%s: %s, %s", __FUNCTION__, tokens[0], tokens[1]);
			gtk_list_store_append (list_store, &iter);
			gtk_list_store_set (list_store, &iter,
					    COL_ID, tokens[0],
					    COL_STASH, tokens[1],
					    -1);
			g_strfreev(tokens);
		}
	}

	g_object_unref (priv->list_job);
	priv->list_job = NULL;
}

void
view_stash_list_selection_changed (GtkWidget *widget, gpointer data)
{
	GtkTreeIter   iter;
	GtkTreeModel *model;
	gboolean      sensitivity;
	GiggleViewStash     *view = GIGGLE_VIEW_STASH(data);
	GiggleViewStashPriv *priv;

	g_debug(__FUNCTION__);
	
	priv = GET_PRIV (data);

	if (gtk_tree_selection_get_selected(GTK_TREE_SELECTION(widget), &model, &iter)) {
		gchar *id = NULL;

		sensitivity = TRUE;
		
		gtk_tree_model_get (model, &iter, COL_ID, &id, -1); 
		view_stash_display (view, id);
	} else {
		sensitivity = FALSE;
		
		/* clear the diff view */
		giggle_diff_view_set_diff (GIGGLE_DIFF_VIEW(priv->diff_view), "");
	}

	/* Update buttons */
	gtk_widget_set_sensitive(priv->apply_button, sensitivity);
	gtk_widget_set_sensitive(priv->pop_button, sensitivity);
	gtk_widget_set_sensitive(priv->drop_button, sensitivity);
}

static gchar *
view_stash_list_selection_id (GiggleViewStash *view)
{
	GtkTreeIter   iter;
	GtkTreeModel *model;
	GiggleViewStashPriv *priv;
	GtkTreeSelection    *selection;
	gchar               *id = NULL;

	priv = GET_PRIV (view);

	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(priv->states_list));
	if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
		gtk_tree_model_get (model, &iter, COL_ID, &id, -1); 
	}
	
	return id;
}

static void
view_stash_subcommand (GtkButton *button,
                       void      *data)
{
	GiggleViewStash     *view = GIGGLE_VIEW_STASH(data);
	GiggleViewStashPriv *priv;
	gchar               *id;

	priv = GET_PRIV (view);

	if (priv->subcommand_job) {
		giggle_git_cancel_job (priv->git, priv->subcommand_job);
		g_object_unref (priv->subcommand_job);
		priv->subcommand_job = NULL;
	}
	
	id = view_stash_list_selection_id (view);

	if (button == GTK_BUTTON(priv->apply_button))
		priv->subcommand_job = giggle_git_stash_subcommand_new ("apply", id);
	if (button == GTK_BUTTON(priv->drop_button))
		priv->subcommand_job = giggle_git_stash_subcommand_new ("drop", id);
	if (button == GTK_BUTTON(priv->pop_button))
		priv->subcommand_job = giggle_git_stash_subcommand_new ("pop", id);

	giggle_git_run_job (priv->git,
			    priv->subcommand_job,
			    view_stash_subcommand_job_callback,
			    view);
}

static void
view_stash_subcommand_job_callback (GiggleGit *git,
			            GiggleJob *job,
			            GError    *error,
			            gpointer   user_data)
{
	GiggleViewStashPriv *priv;

	g_debug(__FUNCTION__);

	priv = GET_PRIV (user_data);

	if (error) {
		GtkWidget *dialog;

		dialog = gtk_message_dialog_new (NULL,
						 GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
						 GTK_MESSAGE_ERROR,
						 GTK_BUTTONS_CLOSE,
						 _("An error ocurred when stash saving:\n%s"),
						 error->message);

		gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (dialog);
	} else {
		/* Tell GiggleGit listeners to update */
		giggle_git_changed (priv->git);
	}

	g_object_unref (priv->subcommand_job);
	priv->subcommand_job = NULL;
}

static void
view_stash_display (GiggleViewStash *view,
		    gchar           *id)
{
	GiggleViewStashPriv *priv;

	priv = GET_PRIV (view);

	if (priv->subcommand_job) {
		giggle_git_cancel_job (priv->git, priv->subcommand_job);
		g_object_unref (priv->subcommand_job);
		priv->subcommand_job = NULL;
	}

	priv->subcommand_job = giggle_git_stash_show_new (id);

	giggle_git_run_job (priv->git,
			    priv->subcommand_job,
			    view_stash_display_job_callback,
			    view);
}

static void
view_stash_display_job_callback (GiggleGit *git,
			            GiggleJob *job,
			            GError    *error,
			            gpointer   user_data)
{
	GiggleViewStash     *view = GIGGLE_VIEW_STASH(user_data);
	GiggleViewStashPriv *priv;

	g_debug(__FUNCTION__);

	priv = GET_PRIV (view);

	if (error) {
		GtkWidget *dialog;

		dialog = gtk_message_dialog_new (NULL,
						 GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
						 GTK_MESSAGE_ERROR,
						 GTK_BUTTONS_CLOSE,
						 _("An error ocurred when stash saving:\n%s"),
						 error->message);

		gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (dialog);
	} else {
		giggle_diff_view_set_diff (GIGGLE_DIFF_VIEW(priv->diff_view),
			giggle_git_stash_show_get_result (GIGGLE_GIT_STASH_SHOW(priv->subcommand_job)));
	}

	g_object_unref (priv->subcommand_job);
	priv->subcommand_job = NULL;
}

GtkWidget *
giggle_view_stash_new (void)
{
	return g_object_new (GIGGLE_TYPE_VIEW_STASH, NULL);
}

