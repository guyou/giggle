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

#include "giggle-author-dialog.h"



G_DEFINE_TYPE (GiggleAuthorDialog, giggle_author_dialog, GTK_TYPE_DIALOG);

static void
giggle_author_dialog_init (GiggleAuthorDialog *object)
{
	/* TODO: Add initialization code here */
}

static void
giggle_author_dialog_finalize (GObject *object)
{
	/* TODO: Add deinitalization code here */

	G_OBJECT_CLASS (giggle_author_dialog_parent_class)->finalize (object);
}

static void
giggle_author_dialog_class_init (GiggleAuthorDialogClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
	GtkDialogClass* parent_class = GTK_DIALOG_CLASS (klass);

	object_class->finalize = giggle_author_dialog_finalize;
}

