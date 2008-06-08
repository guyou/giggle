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
#include "libgiggle/giggle-git-stash-save.h"
#include "giggle-view-stash.h"

typedef struct GiggleViewStashPriv GiggleViewStashPriv;

struct GiggleViewStashPriv {
	GiggleGit *git;
	
	/* git stash save related */
	GiggleJob *save_job;
	GtkWidget *save_button;
	GtkWidget *save_entry;
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

}

static void
giggle_view_stash_init (GiggleViewStash *view)
{
	GiggleViewStashPriv *priv;
	GladeXML            *xml;

	priv = GET_PRIV (view);

	xml = glade_xml_new (GLADEDIR "/main-window.glade", "stash_vbox", NULL);

	gtk_box_pack_start_defaults (GTK_BOX (view),
				     glade_xml_get_widget (xml, "stash_vbox"));

	/* git stash save */
	priv->save_button = glade_xml_get_widget (xml, "save_button");
	priv->save_entry = glade_xml_get_widget (xml, "save_log");
	
	g_signal_connect(G_OBJECT(priv->save_button), "clicked", G_CALLBACK(view_stash_save), view);
	
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
						 _("An error ocurred when stashing:\n%s"),
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

GtkWidget *
giggle_view_stash_new (void)
{
	return g_object_new (GIGGLE_TYPE_VIEW_STASH, NULL);
}
