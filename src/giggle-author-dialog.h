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

#ifndef _GIGGLE_AUTHOR_DIALOG_H_
#define _GIGGLE_AUTHOR_DIALOG_H_

#include <glib-object.h>
#include <gtk/gtk.h>
#include <gtk/gtkdialog.h>

G_BEGIN_DECLS

#define GIGGLE_TYPE_AUTHOR_DIALOG             (giggle_author_dialog_get_type ())
#define GIGGLE_AUTHOR_DIALOG(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GIGGLE_TYPE_AUTHOR_DIALOG, GiggleAuthorDialog))
#define GIGGLE_AUTHOR_DIALOG_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), GIGGLE_TYPE_AUTHOR_DIALOG, GiggleAuthorDialogClass))
#define GIGGLE_IS_AUTHOR_DIALOG(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GIGGLE_TYPE_AUTHOR_DIALOG))
#define GIGGLE_IS_AUTHOR_DIALOG_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), GIGGLE_TYPE_AUTHOR_DIALOG))
#define GIGGLE_AUTHOR_DIALOG_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), GIGGLE_TYPE_AUTHOR_DIALOG, GiggleAuthorDialogClass))

typedef struct _GiggleAuthorDialogClass GiggleAuthorDialogClass;
typedef struct _GiggleAuthorDialog GiggleAuthorDialog;

struct _GiggleAuthorDialogClass
{
	GtkDialogClass parent_class;
};

struct _GiggleAuthorDialog
{
	GtkDialog parent_instance;
};

GType giggle_author_dialog_get_type (void) G_GNUC_CONST;
GtkWidget *giggle_author_dialog_new (void);

G_END_DECLS

#endif /* _GIGGLE_AUTHOR_DIALOG_H_ */
