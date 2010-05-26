/* This file is part of giggle
 *
 * AUTHORS
 *     Sven Herzberg  <herzi@gnome-de.org>
 *
 * Copyright (C) 2007  Sven Herzberg
 * Copyright (C) 2010 Guilhem Bonnefille <guilhem.bonnefille@gmail.com>
 *
 * This work is provided "as is"; redistribution and modification
 * in whole or in part, in any medium, physical or electronic is
 * permitted without restriction.
 *
 * This work is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * In no event shall the authors or contributors be liable for any
 * direct, indirect, incidental, special, exemplary, or consequential
 * damages (including, but not limited to, procurement of substitute
 * goods or services; loss of use, data, or profits; or business
 * interruption) however caused and on any theory of liability, whether
 * in contract, strict liability, or tort (including negligence or
 * otherwise) arising in any way out of the use of this software, even
 * if advised of the possibility of such damage.
 */

#include <libgiggle-git/giggle-git.h>
#include <libgiggle-git/giggle-git-authors.h>
#include <libgiggle/giggle-author.h>

#include <stdio.h>

static gboolean passed = TRUE;

static void
callback (GiggleGit* git,
	  GiggleJob* job,
	  GError   * error,
	  gpointer   loop)
{
	if (error) {
		g_error("An error occurred when retrieving authors list:\n%s", error->message);
		passed = FALSE;
	} else {
		GList *authors;
		authors = giggle_git_authors_get_list (GIGGLE_GIT_AUTHORS (job));
		for(; authors; authors = g_list_next (authors)) {
			puts(giggle_author_get_string (GIGGLE_AUTHOR (authors->data)));
		}
	}

	g_main_loop_quit (loop);
}

int
main (int   argc,
      char**argv)
{
	GMainLoop* loop;
	GiggleGit* git;
	GiggleJob* job;
	gchar* dir;

	g_type_init ();

	loop = g_main_loop_new (NULL, FALSE);

	if (argc == 1)
	{
		dir = g_build_filename (g_get_current_dir (),
					g_getenv ("srcdir"),
					"multi-root.git",
					NULL);
	}
	else
	{
		dir = g_strdup (argv[1]);
	}
					

	git = giggle_git_get ();
	g_return_val_if_fail (giggle_git_set_directory (git, dir, NULL), 1);

	g_free (dir);

	job = giggle_git_authors_new ();
	giggle_git_run_job (git, job, callback, loop);

	g_main_loop_run (loop);

	return passed == TRUE ? 0 : 1;
}

