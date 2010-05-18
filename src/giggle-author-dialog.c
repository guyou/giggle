/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * giggle
 * Copyright (C) Guilhem Bonnefille 2009 <guilhem.bonnefille@gmail.com>
 * 
 * giggle is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * giggle is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"
#include "giggle-author-dialog.h"

#include <libgiggle/giggle-author.h>

#include <libgiggle-git/giggle-git.h>
#include <libgiggle-git/giggle-git-authors.h>

#include <gtk/gtk.h>
#include <glib/gi18n.h>

static void giggle_author_dialog_finalize (GObject *object);
static void author_dialog_update (GiggleAuthorDialog *view);
static void author_dialog_job_callback (GiggleGit *git,
					GiggleJob *job,
					GError    *error,
					gpointer   user_data);


typedef struct GiggleAuthorDialogPriv GiggleAuthorDialogPriv;

struct GiggleAuthorDialogPriv {
	GiggleGit    *git;
	GiggleJob    *job;
	GtkTreeView  *tree;
};

#define GET_PRIV(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), GIGGLE_TYPE_AUTHOR_DIALOG, GiggleAuthorDialogPriv))

G_DEFINE_TYPE (GiggleAuthorDialog, giggle_author_dialog, GTK_TYPE_DIALOG);

enum
{
  COL_NAME = 0,
  NUM_COLS
} ;

static void
giggle_author_dialog_class_init (GiggleAuthorDialogClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
	/* GtkDialogClass* parent_class = GTK_DIALOG_CLASS (klass); */

	g_type_class_add_private (object_class, sizeof (GiggleAuthorDialogPriv));

	object_class->finalize = giggle_author_dialog_finalize;
}

static void
giggle_author_dialog_init (GiggleAuthorDialog *author_window)
{
	GiggleAuthorDialogPriv *priv;
	GtkCellRenderer     *renderer;
	GtkWidget            *view;

	priv = GET_PRIV (author_window);

	priv->git = giggle_git_get ();
	g_signal_connect_swapped (priv->git, "notify::git-dir",
				  G_CALLBACK (author_dialog_update), author_window);

	gtk_window_set_default_size (GTK_WINDOW (author_window), 500, 380);
	gtk_window_set_title (GTK_WINDOW (author_window), _("Commit author"));

	view = gtk_tree_view_new ();
	priv->tree = GTK_TREE_VIEW (view);

	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (view),
                                               -1,      
                                               "Author",  
                                               renderer,
                                               "text", COL_NAME,
                                               NULL);



	gtk_widget_show_all (view);
	gtk_container_add (GTK_CONTAINER (GTK_DIALOG (author_window)->vbox), view);

	g_object_set (author_window,
		      "has-separator", FALSE,
		      NULL);

	gtk_dialog_add_button (GTK_DIALOG (author_window),
			       GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL);
	gtk_dialog_add_button (GTK_DIALOG (author_window),
			       GTK_STOCK_OK, GTK_RESPONSE_OK);

	/* initialize for first time */
	author_dialog_update (author_window);
}

static void
giggle_author_dialog_finalize (GObject *object)
{
	GiggleAuthorDialogPriv *priv;

	priv = GET_PRIV (object);

	if (priv->job) {
		giggle_git_cancel_job (priv->git, priv->job);
		g_object_unref (priv->job);
		priv->job = NULL;
	}

	g_object_unref (priv->git);

	G_OBJECT_CLASS (giggle_author_dialog_parent_class)->finalize (object);
}

GtkWidget *
giggle_author_dialog_new (void)
{
	return g_object_new (GIGGLE_TYPE_AUTHOR_DIALOG, NULL);
}

static void
author_dialog_job_callback (GiggleGit *git,
			    GiggleJob *job,
			    GError    *error,
			    gpointer   user_data)
{
	GiggleAuthorDialog     *author_dialog;
	GiggleAuthorDialogPriv *priv;

	author_dialog = GIGGLE_AUTHOR_DIALOG (user_data);
	priv = GET_PRIV (author_dialog);

	if (error) {
		GtkWidget *dialog;

		dialog = gtk_message_dialog_new (GTK_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET (author_dialog))),
						 GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
						 GTK_MESSAGE_ERROR,
						 GTK_BUTTONS_OK,
						 _("An error occurred when retrieving authors list:\n%s"),
						 error->message);

		gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (dialog);
	} else {
		GtkListStore *store;
		GtkTreeView *view;
		GtkTreeIter   iter;
		GList                 *authors;

		view = priv->tree;

		store = gtk_list_store_new (NUM_COLS, G_TYPE_STRING);
		authors = giggle_git_authors_get_list (GIGGLE_GIT_AUTHORS (job));

		for(; authors; authors = g_list_next (authors)) {
			gchar *name = g_strdup (giggle_author_get_string (GIGGLE_AUTHOR (authors->data)));
			gtk_list_store_append (store, &iter);
			gtk_list_store_set (store, &iter,
					    COL_NAME, name,
					    -1);
		}

		gtk_tree_view_set_model (GTK_TREE_VIEW (view), GTK_TREE_MODEL (store));

		g_object_unref (store);
	}

	g_object_unref (priv->job);
	priv->job = NULL;
}

static void
author_dialog_update (GiggleAuthorDialog *view)
{
	GiggleAuthorDialogPriv *priv;

	priv = GET_PRIV (view);

	if (priv->job) {
		giggle_git_cancel_job (priv->git, priv->job);
		g_object_unref (priv->job);
		priv->job = NULL;
	}

	priv->job = giggle_git_authors_new ();

	giggle_git_run_job (priv->git,
			    priv->job,
			    author_dialog_job_callback,
			    view);
}
