/*
 * Copyright (C) 2007 Holger Macht <holger@homac.de>
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

/* needed for compilation for OS2006 */
#include <gdk/gdkkeysyms.h>
#include <string.h>

#include <hildon/hildon-set-password-dialog.h>
#include <hildon/hildon-get-password-dialog.h>
#include <hildon/hildon-banner.h>

#include "config.h"
#include "main.h"

static gboolean	is_fullscreen;
static gchar	*fullscreen_password;

/****************** fullscreen password ************************/
/*
  returns: 1 if password was correct
	    0 if password was wrong
           -1 if cancel was pressed
*/
static int mmpc_fs_password_query(GtkWindow *parent)
{
	HildonGetPasswordDialog	*dialog		= NULL;
	const char		*pass		= NULL;
	gint			response	= -1;
	int			ret		= 0;

	dialog = HILDON_GET_PASSWORD_DIALOG(hildon_get_password_dialog_new(parent, FALSE));
	gtk_widget_show(GTK_WIDGET(dialog));
	response = gtk_dialog_run(GTK_DIALOG(dialog));

	pass = hildon_get_password_dialog_get_password(dialog);

	/* wrong password */
	if (response == GTK_RESPONSE_OK && fullscreen_password != NULL
	    && (strcmp(pass, fullscreen_password) != 0)) {

		hildon_banner_show_information(GTK_WIDGET(dialog), NULL,
					       _("Wrong password"));
		ret = 0;
	/* cancel pressed */
	} else if (response == GTK_RESPONSE_CANCEL) {
		ret = -1;
	/* password ok */
	} else if (response == GTK_RESPONSE_OK) {
		ret = 1;
	} else {
		hildon_banner_show_information(GTK_WIDGET(dialog), NULL, _("Error"));
		ret = 0;
	}

	gtk_widget_destroy(GTK_WIDGET(dialog));
	return ret;
}

static void mmpc_fs_password_store(GtkWindow *parent)
{
	GtkWidget	*dialog		= NULL;
	const gchar	*pass		= NULL;
	gint		response	= -1;

	dialog = hildon_set_password_dialog_new_with_default(parent, "", FALSE);
	gtk_widget_show_all(dialog);
	response = gtk_dialog_run(GTK_DIALOG(dialog));

	pass = hildon_set_password_dialog_get_password(HILDON_SET_PASSWORD_DIALOG(dialog));

	if (response == GTK_RESPONSE_OK)
		fullscreen_password = g_strdup(pass);

	gtk_widget_destroy(dialog);
}

static void mmpc_fs_unlock(HildonWindow *window)
{
	if (mmpc_fs_get_fullscreen() && fullscreen_password == NULL) {
		gtk_window_unfullscreen(GTK_WINDOW(window));
		is_fullscreen = FALSE;
	} else if (mmpc_fs_get_fullscreen() && fullscreen_password != NULL) {
		int ret;
		while ((ret = mmpc_fs_password_query(GTK_WINDOW(NULL))) == 0) ;
		
		if (ret >= 0) {
			gtk_window_unfullscreen(GTK_WINDOW(window));
			is_fullscreen = FALSE;
		}
	} else {
		gtk_window_fullscreen(GTK_WINDOW(window));
		is_fullscreen = TRUE;
	}

}

/* Callback for hardware keys */
static gboolean key_press_cb(GtkWidget *widget, GdkEventKey *event, HildonWindow *window)
{
	switch (event->keyval) {
	/* fullscreen button */
	case GDK_F6:
		mmpc_fs_unlock(window);
		return TRUE;
	/* power button */
	case GDK_Execute:
		mmpc_fs_unlock(window);
		return TRUE;
	}

	return FALSE;
}

/* check if we are currently in fullscreen mode or not */
gboolean mmpc_fs_get_fullscreen()
{
	return is_fullscreen;
}
/****************** fullscreen password end ************************/

gchar const* authors[] = {
	"Holger Macht",
	"Qball Cow",
	NULL
};

static void about_show()
{
	GtkAboutDialog *about = GTK_ABOUT_DIALOG(gtk_about_dialog_new());
	
	gtk_about_dialog_set_name(about, "Maemo Music Player Client");
	gtk_about_dialog_set_version(about, PACKAGE_VERSION);
	gtk_about_dialog_set_copyright(about,
				       "Copyright (C) 2007,2008 Holger Macht <holger@homac.de>"
				       "(Maemo player interface and port)\n"
				       "Copyright (C) 2004-2006 Qball Cow <Qball@qballcow.nl>");
	gtk_about_dialog_set_website(about,
				     "http://mmpc.garage.maemo.org\n"
				     "http://sarine.nl/gmpc");

	gtk_about_dialog_set_comments(about, "Hildonized and hildon player window by Holger Macht, "
				      "based on gmpc by Qball Cow");
	gtk_about_dialog_set_authors(about, authors);

	gtk_widget_show_all(GTK_WIDGET(about));
	gtk_dialog_run(GTK_DIALOG(about));
	gtk_widget_destroy(GTK_WIDGET(about));
}

static GtkWidget *menu_create(HildonWindow *window, GCallback func, gpointer data)
{
	GtkWidget *main_menu;
	GtkWidget *item_close;
#if MMPC_FS_PW
	GtkWidget *item_lock;
#endif
	GtkWidget *item_about;

	main_menu = gtk_menu_new();

#if MMPC_FS_PW
	item_lock = gtk_menu_item_new_with_label(_("Set fullscreen password"));
	gtk_menu_append(main_menu, item_lock);
	g_signal_connect(G_OBJECT(item_lock), "activate",
			 G_CALLBACK(mmpc_fs_password_store), NULL);
#endif

	item_about = gtk_menu_item_new_with_label(_("About..."));
	gtk_menu_append(main_menu, item_about);
	g_signal_connect(G_OBJECT(item_about), "activate", G_CALLBACK(about_show), NULL);

	item_close = gtk_menu_item_new_with_label(_("Close"));
	gtk_menu_append(main_menu, item_close);
	g_signal_connect_swapped(G_OBJECT(item_close), "activate", func, data);

	gtk_widget_show_all(GTK_WIDGET(main_menu));
	hildon_window_set_menu(window, GTK_MENU(main_menu));

	return main_menu;
}

/* attaches a window as to common maemo functions such as hardware buttons
 * or menus
 *
 * window - the HildonWindow to attach
 * close_func - function pointer that is called when menu-close is clicked
 * data - data to pass to close_func
 */
void mmpc_attach(HildonWindow *window, GCallback close_func, gpointer data)
{
	menu_create(window, close_func, data);

	if (mmpc_fs_get_fullscreen())
		gtk_window_fullscreen(GTK_WINDOW(window));

	g_signal_connect(G_OBJECT(window),  "key_press_event",
			 G_CALLBACK(key_press_cb), window);
}
