/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Copyright (C) 2007 Imendio AB
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

#include "giggle-git-stash-list.h"

typedef struct GiggleGitStashListPriv GiggleGitStashListPriv;

struct GiggleGitStashListPriv {
	GSList *states;
};

static void     git_stash_list_finalize            (GObject           *object);

static gboolean git_stash_list_get_command_line    (GiggleJob         *job,
						    gchar            **command_line);
static void     git_stash_list_handle_output       (GiggleJob         *job,
						    const gchar       *output_str,
						    gsize              output_len);

G_DEFINE_TYPE (GiggleGitStashList, giggle_git_stash_list, GIGGLE_TYPE_JOB)

#define GET_PRIV(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), GIGGLE_TYPE_GIT_STASH_LIST, GiggleGitStashListPriv))

static void
giggle_git_stash_list_class_init (GiggleGitStashListClass *class)
{
	GObjectClass   *object_class = G_OBJECT_CLASS (class);
	GiggleJobClass *job_class    = GIGGLE_JOB_CLASS (class);

	object_class->finalize     = git_stash_list_finalize;

	job_class->get_command_line = git_stash_list_get_command_line;
	job_class->handle_output    = git_stash_list_handle_output;

	g_type_class_add_private (object_class, sizeof (GiggleGitStashListPriv));
}

static void
giggle_git_stash_list_init (GiggleGitStashList *list)
{
	GiggleGitStashListPriv *priv;

	priv = GET_PRIV (list);

	priv->states = NULL;
}

static void
git_stash_list_finalize (GObject *object)
{
	GiggleGitStashListPriv *priv;

	priv = GET_PRIV (object);

	g_slist_free (priv->states);

	G_OBJECT_CLASS (giggle_git_stash_list_parent_class)->finalize (object);
}

static gboolean
git_stash_list_get_command_line (GiggleJob *job, gchar **command_line)
{
	*command_line = g_strdup_printf (GIT_COMMAND " stash list");

	return TRUE;
}

static void
git_stash_list_handle_output (GiggleJob   *job,
			const gchar *output_str,
			gsize        output_len)
{
	GiggleGitStashListPriv    *priv;
	gchar                    **lines;
	gint                       i;

	priv = GET_PRIV (job);
	lines = g_strsplit (output_str, "\n", -1);

	for (i = 0; lines[i] && *lines[i]; i++) {
		g_debug("%s: %s", __FUNCTION__, lines[i]);
		priv->states = g_slist_append (priv->states, g_strdup(lines[i]));
	}

	g_strfreev (lines);
}

GSList *
giggle_git_stash_list_get_states (GiggleGitStashList *stash_list)
{
	GiggleGitStashListPriv *priv;

	g_return_val_if_fail (GIGGLE_IS_GIT_STASH_LIST (stash_list), NULL);

	priv = GET_PRIV (stash_list);

	return priv->states;
}

GiggleJob *
giggle_git_stash_list_new ()
{
	return g_object_new (GIGGLE_TYPE_GIT_STASH_LIST, NULL);
}
