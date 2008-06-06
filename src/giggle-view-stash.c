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
#include "giggle-view-stash.h"

typedef struct GiggleViewCommitPriv GiggleViewCommitPriv;

struct GiggleViewCommitPriv {
	GiggleGit *git;
};

static void    view_stash_finalize              (GObject           *object);

static void    view_stash_project_changed_cb    (GObject           *object,
						   GParamSpec        *arg,
						   GiggleViewCommit *view);

G_DEFINE_TYPE (GiggleViewCommit, giggle_view_stash, GIGGLE_TYPE_VIEW)

#define GET_PRIV(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), GIGGLE_TYPE_VIEW_STASH, GiggleViewCommitPriv))

static void
giggle_view_stash_class_init (GiggleViewCommitClass *class)
{
	GObjectClass *object_class = G_OBJECT_CLASS (class);

	object_class->finalize = view_stash_finalize;

	g_type_class_add_private (object_class, sizeof (GiggleViewCommitPriv));
}

static void
view_stash_update_data (GiggleViewCommit *view)
{
	GiggleViewCommitPriv *priv;

	priv = GET_PRIV (view);

}

static void
giggle_view_stash_init (GiggleViewCommit *view)
{
	GiggleViewCommitPriv *priv;
	GladeXML               *xml;

	priv = GET_PRIV (view);

	xml = glade_xml_new (GLADEDIR "/main-window.glade", "stash_vbox", NULL);

	gtk_box_pack_start_defaults (GTK_BOX (view),
				     glade_xml_get_widget (xml, "stash_vbox"));

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
	GiggleViewCommitPriv *priv;

	priv = GET_PRIV (object);
}

static void
view_stash_project_changed_cb (GObject           *object,
				 GParamSpec        *arg,
				 GiggleViewCommit *view)
{
	view_stash_update_data (view);
}

GtkWidget *
giggle_view_stash_new (void)
{
	return g_object_new (GIGGLE_TYPE_VIEW_STASH, NULL);
}
