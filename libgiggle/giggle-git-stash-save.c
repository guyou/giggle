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

#include "giggle-git-stash-save.h"

typedef struct GiggleGitStashSavePriv GiggleGitStashSavePriv;

struct GiggleGitStashSavePriv {
	gchar *log;
};

static void     git_stash_save_finalize            (GObject           *object);
static void     git_stash_save_get_property        (GObject           *object,
						guint              param_id,
						GValue            *value,
						GParamSpec        *pspec);
static void     git_stash_save_set_property        (GObject           *object,
						guint              param_id,
						const GValue      *value,
						GParamSpec        *pspec);

static gboolean git_stash_save_get_command_line    (GiggleJob         *job,
						    gchar            **command_line);


G_DEFINE_TYPE (GiggleGitStashSave, giggle_git_stash_save, GIGGLE_TYPE_JOB)

#define GET_PRIV(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), GIGGLE_TYPE_GIT_STASH_SAVE, GiggleGitStashSavePriv))

enum {
	PROP_0,
	PROP_LOG,
};

static void
giggle_git_stash_save_class_init (GiggleGitStashSaveClass *class)
{
	GObjectClass   *object_class = G_OBJECT_CLASS (class);
	GiggleJobClass *job_class    = GIGGLE_JOB_CLASS (class);

	object_class->finalize     = git_stash_save_finalize;
	object_class->get_property = git_stash_save_get_property;
	object_class->set_property = git_stash_save_set_property;

	job_class->get_command_line = git_stash_save_get_command_line;

	g_object_class_install_property (object_class,
					 PROP_LOG,
					 g_param_spec_string ("log",
							      "Log",
							      "Log for the changeset",
							      NULL,
							      G_PARAM_READWRITE));

	g_type_class_add_private (object_class, sizeof (GiggleGitStashSavePriv));
}

static void
giggle_git_stash_save_init (GiggleGitStashSave *dummy)
{
}

static void
git_stash_save_finalize (GObject *object)
{
	GiggleGitStashSavePriv *priv;

	priv = GET_PRIV (object);

	g_free (priv->log);

	G_OBJECT_CLASS (giggle_git_stash_save_parent_class)->finalize (object);
}

static void
git_stash_save_get_property (GObject    *object,
			 guint       param_id,
			 GValue     *value,
			 GParamSpec *pspec)
{
	GiggleGitStashSavePriv *priv;

	priv = GET_PRIV (object);

	switch (param_id) {
	case PROP_LOG:
		g_value_set_string (value, priv->log);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
		break;
	}
}

static void
git_stash_save_set_property (GObject      *object,
			 guint         param_id,
			 const GValue *value,
			 GParamSpec   *pspec)
{
	GiggleGitStashSavePriv *priv;

	priv = GET_PRIV (object);

	switch (param_id) {
	case PROP_LOG:
		priv->log = g_value_dup_string (value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
		break;
	}
}

static gboolean
git_stash_save_get_command_line (GiggleJob *job, gchar **command_line)
{
	GiggleGitStashSavePriv *priv;
	GString             *str;
	gchar               *escaped;

	priv = GET_PRIV (job);
	str = g_string_new (GIT_COMMAND " stash save");

	if (priv->log) {
		escaped = g_strescape (priv->log, "\b\f\n\r\t\\");

		g_string_append_printf (str, " \"%s\"", escaped);
	}

	*command_line = g_string_free (str, FALSE);
	return TRUE;
}

GiggleJob *
giggle_git_stash_save_new (const gchar *log)
{
	return g_object_new (GIGGLE_TYPE_GIT_STASH_SAVE,
			     "log", log,
			     NULL);
}
