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
static void author_dialog_list_selection_changed (GtkWidget *widget, gpointer data);
static void author_dialog_entry_changed (GtkWidget *widget, gpointer data);


typedef struct GiggleAuthorDialogPriv GiggleAuthorDialogPriv;

struct GiggleAuthorDialogPriv {
	GiggleGit    *git;
	GiggleJob    *job;
	GtkTreeView  *tree;
	GtkWidget    *ok_button;
	GtkWidget    *author_entry;
};

#define GET_PRIV(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), GIGGLE_TYPE_AUTHOR_DIALOG, GiggleAuthorDialogPriv))

G_DEFINE_TYPE (GiggleAuthorDialog, giggle_author_dialog, GTK_TYPE_DIALOG);

enum
{
  COL_AUTHOR = 0,
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
	GtkCellRenderer        *renderer;
	GtkWidget              *view;
	GtkWidget              *scrolled_window;
	GtkTreeSelection       *selection;
	GtkWidget              *label;
	gchar                  *str;

	priv = GET_PRIV (author_window);

	priv->git = giggle_git_get ();
	g_signal_connect_swapped (priv->git, "notify::git-dir",
				  G_CALLBACK (author_dialog_update), author_window);

	gtk_window_set_default_size (GTK_WINDOW (author_window), 500, 380);
	gtk_window_set_title (GTK_WINDOW (author_window), _("Commit author"));

	gtk_container_set_border_width (GTK_CONTAINER (GTK_DIALOG (author_window)->vbox), 7);
	gtk_box_set_homogeneous (GTK_BOX (GTK_DIALOG (author_window)->vbox), FALSE);

	/* Author */
	label = gtk_label_new (NULL);
	str = g_strdup_printf ("<b>%s</b>", _("Author:"));
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.0);
	gtk_label_set_markup (GTK_LABEL (label), str);
	g_free (str);
	gtk_widget_show_all (label);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (author_window)->vbox), label, FALSE, TRUE, 0);

	priv->author_entry = gtk_entry_new ();

	g_signal_connect(priv->author_entry, "changed", G_CALLBACK(author_dialog_entry_changed), author_window);

	gtk_widget_show_all (priv->author_entry);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (author_window)->vbox), priv->author_entry, FALSE, TRUE, 0);

	/* Known authors */
	label = gtk_label_new (NULL);
	str = g_strdup_printf ("<b>%s</b>", _("Known authors:"));
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.0);
	gtk_label_set_markup (GTK_LABEL (label), str);
	g_free (str);
	gtk_widget_show_all (label);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (author_window)->vbox), label, FALSE, TRUE, 0);

	view = gtk_tree_view_new ();
	priv->tree = GTK_TREE_VIEW (view);

	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (view),
                                               -1,      
                                               "Author",  
                                               renderer,
                                               "text", COL_AUTHOR,
                                               NULL);

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(view));
	gtk_tree_selection_set_mode (selection, GTK_SELECTION_SINGLE);
	g_signal_connect(selection, "changed", G_CALLBACK(author_dialog_list_selection_changed), author_window);

	scrolled_window = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
					GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_container_add (GTK_CONTAINER (scrolled_window), view);

	gtk_widget_show_all (scrolled_window);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (author_window)->vbox), scrolled_window, TRUE, TRUE, 0);

	g_object_set (author_window,
		      "has-separator", FALSE,
		      NULL);

	gtk_dialog_add_button (GTK_DIALOG (author_window),
			       GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL);
	priv->ok_button = gtk_dialog_add_button (GTK_DIALOG (author_window),
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
					    COL_AUTHOR, name,
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

static void
author_dialog_list_selection_changed (GtkWidget *widget, gpointer data)
{
	GtkTreeIter   iter;
	GtkTreeModel *model;
	GiggleAuthorDialogPriv *priv;

	g_return_if_fail (GIGGLE_IS_AUTHOR_DIALOG (data));
	priv = GET_PRIV (data);

	if (gtk_tree_selection_get_selected(GTK_TREE_SELECTION(widget), &model, &iter)) {
		gchar *author = NULL;
		
		gtk_tree_model_get (model, &iter, COL_AUTHOR, &author, -1); 

		/* Save the selected author */
		gtk_entry_set_text (GTK_ENTRY (priv->author_entry), author);
		g_free (author);
		author = NULL;
	}
}

static void
author_dialog_entry_changed (GtkWidget *widget, gpointer data)
{
	gboolean                sensitivity;
	GiggleAuthorDialogPriv *priv;
	const gchar            *author;

	sensitivity = FALSE;

	g_return_if_fail (GIGGLE_IS_AUTHOR_DIALOG (data));
	priv = GET_PRIV (data);

	author = gtk_entry_get_text (GTK_ENTRY (priv->author_entry));
	if (strlen (author) > 0) {
		/* TODO check email */
		sensitivity = TRUE;
	}

	/* Update buttons */
	gtk_widget_set_sensitive(priv->ok_button, sensitivity);
}

const gchar *
giggle_author_dialog_get_author (GiggleAuthorDialog *self)
{
	GiggleAuthorDialogPriv *priv;
	const gchar            *author = NULL;

	priv = GET_PRIV (self);

	author = gtk_entry_get_text (GTK_ENTRY (priv->author_entry));

	return author;
}
