/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Copyright (C) 2007 Imendio AB
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

#include "giggle-git-stash-clear.h"

static void     git_stash_clear_finalize            (GObject           *object);

static gboolean git_stash_clear_get_command_line    (GiggleJob         *job,
						    gchar            **command_line);


G_DEFINE_TYPE (GiggleGitStashClear, giggle_git_stash_clear, GIGGLE_TYPE_JOB)

static void
giggle_git_stash_clear_class_init (GiggleGitStashClearClass *class)
{
	GObjectClass   *object_class = G_OBJECT_CLASS (class);
	GiggleJobClass *job_class    = GIGGLE_JOB_CLASS (class);

	object_class->finalize     = git_stash_clear_finalize;

	job_class->get_command_line = git_stash_clear_get_command_line;
}

static void
giggle_git_stash_clear_init (GiggleGitStashClear *dummy)
{
}

static void
git_stash_clear_finalize (GObject *object)
{
	G_OBJECT_CLASS (giggle_git_stash_clear_parent_class)->finalize (object);
}

static gboolean
git_stash_clear_get_command_line (GiggleJob *job, gchar **command_line)
{
	GString             *str;

	str = g_string_new (GIT_COMMAND " stash clear");


	*command_line = g_string_free (str, FALSE);
	return TRUE;
}

GiggleJob *
giggle_git_stash_clear_new ()
{
	return g_object_new (GIGGLE_TYPE_GIT_STASH_CLEAR,
			     NULL);
}
