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

#include "giggle-git-stash-show.h"

typedef struct GiggleGitStashShowPriv GiggleGitStashShowPriv;

struct GiggleGitStashShowPriv {
	gchar *stash;
	
	gchar          *result;
};

static void     git_stash_show_finalize            (GObject           *object);
static void     git_stash_show_get_property        (GObject           *object,
							guint              param_id,
							GValue            *value,
							GParamSpec        *pspec);
static void     git_stash_show_set_property        (GObject           *object,
							guint              param_id,
							const GValue      *value,
							GParamSpec        *pspec);

static gboolean git_stash_show_get_command_line    (GiggleJob         *job,
							gchar            **command_line);

static void
git_stash_show_handle_output (GiggleJob   *job,
			const gchar *output_str,
			gsize        output_len);


G_DEFINE_TYPE (GiggleGitStashShow, giggle_git_stash_show, GIGGLE_TYPE_JOB)

#define GET_PRIV(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), GIGGLE_TYPE_GIT_STASH_SHOW, GiggleGitStashShowPriv))

enum {
	PROP_0,
	PROP_STASH,
};

static void
giggle_git_stash_show_class_init (GiggleGitStashShowClass *class)
{
	GObjectClass   *object_class = G_OBJECT_CLASS (class);
	GiggleJobClass *job_class    = GIGGLE_JOB_CLASS (class);

	object_class->finalize     = git_stash_show_finalize;
	object_class->get_property = git_stash_show_get_property;
	object_class->set_property = git_stash_show_set_property;

	job_class->get_command_line = git_stash_show_get_command_line;
	job_class->handle_output = git_stash_show_handle_output;

	g_object_class_install_property (object_class,
					 PROP_STASH,
					 g_param_spec_string ("stash",
							      "Stash",
							      "Id of the stash",
							      NULL,
							      G_PARAM_READWRITE));

	g_type_class_add_private (object_class, sizeof (GiggleGitStashShowPriv));
}

static void
giggle_git_stash_show_init (GiggleGitStashShow *dummy)
{
}

static void
git_stash_show_finalize (GObject *object)
{
	GiggleGitStashShowPriv *priv;

	priv = GET_PRIV (object);

	g_free (priv->stash);
	g_free (priv->result);

	G_OBJECT_CLASS (giggle_git_stash_show_parent_class)->finalize (object);
}

static void
git_stash_show_get_property (GObject    *object,
			 guint       param_id,
			 GValue     *value,
			 GParamSpec *pspec)
{
	GiggleGitStashShowPriv *priv;

	priv = GET_PRIV (object);

	switch (param_id) {
	case PROP_STASH:
		g_value_set_string (value, priv->stash);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
		break;
	}
}

static void
git_stash_show_set_property (GObject      *object,
			 guint         param_id,
			 const GValue *value,
			 GParamSpec   *pspec)
{
	GiggleGitStashShowPriv *priv;

	priv = GET_PRIV (object);

	switch (param_id) {
	case PROP_STASH:
		priv->stash = g_value_dup_string (value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
		break;
	}
}

static gboolean
git_stash_show_get_command_line (GiggleJob *job, gchar **command_line)
{
	GiggleGitStashShowPriv *priv;

	priv = GET_PRIV (job);
	
	*command_line = g_strdup_printf (GIT_COMMAND " stash show --full-diff %s", priv->stash);
	g_debug("%s: %s", __FUNCTION__, *command_line);
	
	return TRUE;
}

static void
git_stash_show_handle_output (GiggleJob   *job,
			const gchar *output_str,
			gsize        output_len)
{
	GiggleGitStashShowPriv *priv;

	priv = GET_PRIV (job);

	priv->result = g_strdup (output_str);
}

GiggleJob *
giggle_git_stash_show_new (const gchar *stash)
{
	return g_object_new (GIGGLE_TYPE_GIT_STASH_SHOW,
			     "stash", stash,
			     NULL);
}

const gchar *
giggle_git_stash_show_get_result (GiggleGitStashShow *stash)
{
	GiggleGitStashShowPriv *priv;

	g_return_val_if_fail (GIGGLE_IS_GIT_STASH_SHOW (stash), NULL);

	priv = GET_PRIV (stash);

	return priv->result;
}
