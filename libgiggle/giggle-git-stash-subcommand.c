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

#include "giggle-git-stash-subcommand.h"

typedef struct GiggleGitStashSubcommandPriv GiggleGitStashSubcommandPriv;

struct GiggleGitStashSubcommandPriv {
	gchar *subcommand;
	gchar *stash;
};

static void     git_stash_subcommand_finalize            (GObject           *object);
static void     git_stash_subcommand_get_property        (GObject           *object,
							guint              param_id,
							GValue            *value,
							GParamSpec        *pspec);
static void     git_stash_subcommand_set_property        (GObject           *object,
							guint              param_id,
							const GValue      *value,
							GParamSpec        *pspec);

static gboolean git_stash_subcommand_get_command_line    (GiggleJob         *job,
							gchar            **command_line);


G_DEFINE_TYPE (GiggleGitStashSubcommand, giggle_git_stash_subcommand, GIGGLE_TYPE_JOB)

#define GET_PRIV(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), GIGGLE_TYPE_GIT_STASH_SUBCOMMAND, GiggleGitStashSubcommandPriv))

enum {
	PROP_0,
	PROP_SUBCOMMAND,
	PROP_ID,
};

static void
giggle_git_stash_subcommand_class_init (GiggleGitStashSubcommandClass *class)
{
	GObjectClass   *object_class = G_OBJECT_CLASS (class);
	GiggleJobClass *job_class    = GIGGLE_JOB_CLASS (class);

	object_class->finalize     = git_stash_subcommand_finalize;
	object_class->get_property = git_stash_subcommand_get_property;
	object_class->set_property = git_stash_subcommand_set_property;

	job_class->get_command_line = git_stash_subcommand_get_command_line;

	g_object_class_install_property (object_class,
					 PROP_SUBCOMMAND,
					 g_param_spec_string ("subcommand",
							      "SubCommand",
							      "Subcommand to execute",
							      NULL,
							      G_PARAM_READWRITE));

	g_object_class_install_property (object_class,
					 PROP_ID,
					 g_param_spec_string ("stash",
							      "Stash",
							      "Id of the stash",
							      NULL,
							      G_PARAM_READWRITE));

	g_type_class_add_private (object_class, sizeof (GiggleGitStashSubcommandPriv));
}

static void
giggle_git_stash_subcommand_init (GiggleGitStashSubcommand *dummy)
{
}

static void
git_stash_subcommand_finalize (GObject *object)
{
	GiggleGitStashSubcommandPriv *priv;

	priv = GET_PRIV (object);

	g_free (priv->subcommand);
	g_free (priv->stash);

	G_OBJECT_CLASS (giggle_git_stash_subcommand_parent_class)->finalize (object);
}

static void
git_stash_subcommand_get_property (GObject    *object,
			 guint       param_id,
			 GValue     *value,
			 GParamSpec *pspec)
{
	GiggleGitStashSubcommandPriv *priv;

	priv = GET_PRIV (object);

	switch (param_id) {
	case PROP_SUBCOMMAND:
		g_value_set_string (value, priv->subcommand);
		break;
	case PROP_ID:
		g_value_set_string (value, priv->stash);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
		break;
	}
}

static void
git_stash_subcommand_set_property (GObject      *object,
			 guint         param_id,
			 const GValue *value,
			 GParamSpec   *pspec)
{
	GiggleGitStashSubcommandPriv *priv;

	priv = GET_PRIV (object);

	switch (param_id) {
	case PROP_SUBCOMMAND:
		priv->subcommand = g_value_dup_string (value);
		break;
	case PROP_ID:
		priv->stash = g_value_dup_string (value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
		break;
	}
}

static gboolean
git_stash_subcommand_get_command_line (GiggleJob *job, gchar **command_line)
{
	GiggleGitStashSubcommandPriv *priv;

	priv = GET_PRIV (job);
	g_debug("%s: %p->%p", __FUNCTION__, priv, priv->subcommand);
	g_debug("%s: %s", __FUNCTION__, priv->subcommand);
//	g_return_val_if_fail (priv->subcommand != NULL, FALSE);
	
	*command_line = g_strdup_printf ("/bin/echo " GIT_COMMAND " stash %s", priv->stash);
	g_debug("%s: %s", __FUNCTION__, *command_line);
	
	return TRUE;
}

GiggleJob *
giggle_git_stash_subcommand_new (const gchar *subcommand, const gchar *stash)
{
	return g_object_new (GIGGLE_TYPE_GIT_STASH_SUBCOMMAND,
			     "subcommand", subcommand,
			     "stash", stash,
			     NULL);
}
