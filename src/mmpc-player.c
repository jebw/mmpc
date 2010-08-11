/*
 * Copyright (C) 2007,2008 Holger Macht <holger@homac.de>
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

#define VOLUMEBAR_TIME_SHOW	3
#define VOLUMEBAR_STEP		10

#include <libmpd/libmpd.h>
#include <gtk/gtk.h>
/* needed for compilation for OS2006 */
#include <gdk/gdkkeysyms.h>
#include <string.h>
#include <stdlib.h>

#include <hildon/hildon-program.h>
#include <hildon/hildon-vvolumebar.h>
#include <hildon/hildon-seekbar.h>
#include <hildon/hildon-banner.h>

#include "main.h"

/* from playlist3.c */
void pl3_current_playlist_browser_row_activated(GtkTreeView *tree, GtkTreePath *path,
						GtkTreeViewColumn *col);

/* structures to hold GUI elements */
struct ui_toolbar {
	HildonSeekbar	*seekbar;
	GtkWidget	*label_time_left;
	GtkWidget	*label_time_total;
	GtkToolItem	*button_random;
	GtkToolItem	*button_repeat;
};

struct ui_pixbufs {
	GdkPixbuf *play_button;
	GdkPixbuf *pause_button;
};

struct ui_interface {
	struct ui_toolbar	tb;
	struct ui_pixbufs	pixbufs;
	HildonWindow		*main_window;
	HildonVolumebar		*volumebar;
	GtkWidget		*vbox_left;
	GtkWidget		*label_cur_song;
	GtkWidget		*play_button;
	GtkWidget		*stop_button;
	GtkWidget		*forward_button;
	GtkWidget		*backward_button;
	GtkWidget		*playlist;
};

static struct ui_interface	ui;	
static int			volumebar_timeout	= VOLUMEBAR_TIME_SHOW;
extern GtkListStore		*pl2_store;

/* dummies */
void player_show() {}
void player_hide() {}
int  player_get_hidden() { return 0; }
int  msg_pop_popup() { return 0; }
void player_connection_changed(MpdObj *mi, int connect) {}
void player_destroy() {}

/****************************** helper ****************************/

static void popup(char *msg)
{
	hildon_banner_show_information(GTK_WIDGET(ui.main_window), NULL, msg);
}

/* give time in seconds and given label will get formatted time as text */
static void label_set_text_as_time(int time, GtkWidget *label)
{
	char *sec_str, *min_str, *text;
	int minutes = 0;
	int seconds = 0;

	seconds = time % 60;
	minutes = (time - seconds) / 60;

	if (seconds < 10)
		sec_str = g_strdup_printf("0%d", seconds);
	else
		sec_str = g_strdup_printf("%d", seconds);

	if (minutes < 10)
		min_str = g_strdup_printf("0%d", minutes);
	else
		min_str = g_strdup_printf("%d", minutes);

	text = g_strdup_printf("%s:%s", min_str, sec_str);

	gtk_label_set_text(GTK_LABEL(label), text);
	g_free(min_str);
	g_free(sec_str);
	g_free(text);
}

static GdkPixbuf *pixbuf_get(char *name)
{
	char		*path	= NULL;
	GdkPixbuf	*pb	= NULL;

	path = gmpc_get_full_image_path(name);
	pb = gdk_pixbuf_new_from_file(path, NULL);
	g_free(path);

	return pb;
}
/****************************** helper end ****************************/

/* user triggered seek */
static void seekbar_clicked(GtkRange *range, GtkScrollType scroll,
			    gdouble value, gpointer data)
{
	mpd_player_seek(connection, (int)value);
}

static void player_song_changed()
{
	int	total_time;
	gchar	buffer[1024];
	char	*string;

	string = cfg_get_single_value_as_string_with_default(config, "player", "display_markup",
							     DEFAULT_PLAYER_MARKUP);
	mpd_song_markup(buffer, 1024, string, mpd_playlist_get_current_song(connection));
	cfg_free_string(string);
	if (strlen(buffer) == 0)
		return;

	msg_set_base(buffer);
	popup(buffer);

	total_time = mpd_status_get_total_song_time(connection);
	hildon_seekbar_set_total_time(ui.tb.seekbar, total_time);
	hildon_seekbar_set_fraction(ui.tb.seekbar, total_time);

	label_set_text_as_time(total_time, ui.tb.label_time_total);
}

static void player_update(int state)
{
	if (state == MPD_PLAYER_UNKNOWN) {
		gtk_widget_set_sensitive(GTK_WIDGET(ui.play_button), FALSE);
		gtk_widget_set_sensitive(GTK_WIDGET(ui.stop_button), FALSE);
		gtk_widget_set_sensitive(GTK_WIDGET(ui.tb.seekbar), FALSE);

	} else if (state == MPD_PLAYER_STOP) {
		GtkWidget *play_image = gtk_image_new_from_pixbuf(ui.pixbufs.play_button);
		gtk_button_set_image(GTK_BUTTON(ui.play_button), play_image);

		gtk_widget_set_sensitive(GTK_WIDGET(ui.play_button), TRUE);
		gtk_widget_set_sensitive(GTK_WIDGET(ui.stop_button), FALSE);
		gtk_widget_set_sensitive(GTK_WIDGET(ui.forward_button), FALSE);
		gtk_widget_set_sensitive(GTK_WIDGET(ui.backward_button), FALSE);
		gtk_widget_set_sensitive(GTK_WIDGET(ui.tb.seekbar), FALSE);

	} else if (state == MPD_PLAYER_PLAY) {
		GtkWidget *pause_image = gtk_image_new_from_pixbuf(ui.pixbufs.pause_button);
		gtk_button_set_image(GTK_BUTTON(ui.play_button), pause_image);

		gtk_widget_set_sensitive(GTK_WIDGET(ui.play_button), TRUE);
		gtk_widget_set_sensitive(GTK_WIDGET(ui.stop_button), TRUE);
		gtk_widget_set_sensitive(GTK_WIDGET(ui.forward_button), TRUE);
		gtk_widget_set_sensitive(GTK_WIDGET(ui.backward_button), TRUE);
		gtk_widget_set_sensitive(GTK_WIDGET(ui.tb.seekbar), TRUE);

	} else if (state == MPD_PLAYER_PAUSE) {
		GtkWidget *play_image = gtk_image_new_from_pixbuf(ui.pixbufs.play_button);
		gtk_button_set_image(GTK_BUTTON(ui.play_button), play_image);

		gtk_widget_set_sensitive(GTK_WIDGET(ui.play_button), TRUE);
		gtk_widget_set_sensitive(GTK_WIDGET(ui.forward_button), TRUE);
		gtk_widget_set_sensitive(GTK_WIDGET(ui.backward_button), TRUE);
	}

	if (state != MPD_PLAYER_PLAY && state != MPD_PLAYER_PAUSE)
		msg_set_base(_("Maemo Music Player Client"));
	else
		player_song_changed();
}

/* here, time and progress bar should be updated*/
/* static */ int update_player()
{
	int elapsed = mpd_status_get_elapsed_song_time(connection);

	hildon_seekbar_set_position(ui.tb.seekbar, elapsed);
	label_set_text_as_time(elapsed, ui.tb.label_time_left);
	return 0;
}

/****************** volumebar ************************/
static void volumebar_clicked()
{
	gdouble value;

	if (!mpd_check_connected(connection))
		return;

	value = hildon_volumebar_get_level(HILDON_VOLUMEBAR(ui.volumebar));
	mpd_status_set_volume(connection, (int)value);
	volumebar_timeout = VOLUMEBAR_TIME_SHOW;
}

static void volumebar_mute_toggled()
{
	gboolean muted		= FALSE;
	static gdouble value	= -1;

	if (!mpd_check_connected(connection))
		return;

	muted = hildon_volumebar_get_mute(HILDON_VOLUMEBAR(ui.volumebar));

	if (muted) {
		value = hildon_volumebar_get_level(HILDON_VOLUMEBAR(ui.volumebar));
		mpd_status_set_volume(connection, 0);
	} else
		mpd_status_set_volume(connection,(int)value);

	volumebar_timeout = VOLUMEBAR_TIME_SHOW;
}

static gboolean volumebar_hide()
{
	if (volumebar_timeout > 0) {
		volumebar_timeout--;
		return TRUE;
	}

	gtk_widget_show(GTK_WIDGET(ui.vbox_left));
	gtk_widget_hide(GTK_WIDGET(ui.volumebar));

	volumebar_timeout = VOLUMEBAR_TIME_SHOW;
	return FALSE;
}

static void volumebar_show()
{
	if (GTK_WIDGET_VISIBLE(ui.volumebar)) {
		volumebar_timeout = VOLUMEBAR_TIME_SHOW;
		return;
	}

	gtk_widget_hide(GTK_WIDGET(ui.vbox_left));
	gtk_widget_show(GTK_WIDGET(ui.volumebar));

	g_timeout_add(1000, volumebar_hide, NULL);
}
/****************** volumebar end ************************/

/************************** playlist *******************************/
/* returns the layout container for embedding it in the main view */
static GtkWidget *playlist_create(void)
{
	GtkCellRenderer	*renderer	= NULL;
	GtkWidget	*scroll		= NULL;
	GtkWidget	*vbox		= NULL;

	ui.playlist = gtk_tree_view_new_with_model(GTK_TREE_MODEL(pl2_store));

	renderer = gtk_cell_renderer_pixbuf_new();
	gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(ui.playlist),
						    -1,
						    "Icon",
						    renderer,
						    "stock-id", SONG_STOCK_ID, NULL);

	renderer = gtk_cell_renderer_text_new();

	gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(ui.playlist),
						    -1, "Song", renderer,
						    "text", SONG_TITLE,
						    "weight", WEIGHT_INT,
						    NULL);

	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(ui.playlist), FALSE);
	gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(ui.playlist), TRUE);
	gtk_tree_selection_set_mode(gtk_tree_view_get_selection(GTK_TREE_VIEW(ui.playlist)),
				    GTK_SELECTION_MULTIPLE);
	g_signal_connect(G_OBJECT(ui.playlist), "row-activated",
			 G_CALLBACK(pl3_current_playlist_browser_row_activated), NULL);

	scroll = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
				       GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scroll),
					    GTK_SHADOW_ETCHED_IN);

	vbox = gtk_vbox_new(FALSE, 5);
	gtk_container_add(GTK_CONTAINER(scroll), ui.playlist);
	gtk_box_pack_start(GTK_BOX(vbox), scroll, TRUE, TRUE,0);
	gtk_widget_show_all(vbox);

	return vbox;
}

static void playlist_selection_remove()
{
	GtkTreeSelection	*selection		= NULL;
	GList			*list			= NULL;
	GtkTreeModel		*model			= GTK_TREE_MODEL(pl2_store);

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(ui.playlist));

	if (gtk_tree_selection_count_selected_rows(selection) <= 0)
		return;

	list = gtk_tree_selection_get_selected_rows(selection, &model);

	for ( ; list != NULL; list = g_list_next(list)) {
		GtkTreeIter	iter;
		int		id;
		gtk_tree_model_get_iter(model, &iter, (GtkTreePath *)list->data);
		gtk_tree_model_get(model, &iter, SONG_ID, &id, -1);
		mpd_playlist_queue_delete_id(connection, id);			
	}
	
	mpd_playlist_queue_commit(connection);
	g_list_foreach(list, (GFunc) gtk_tree_path_free, NULL);
	g_list_free(list);
}

static void playlist_show_info()
{
	GtkTreeModel		*model		= NULL;
	GtkTreeSelection	*selection	= NULL;
	GList			*list		= NULL;
	gint			song_id		= -1;

	g_assert(ui.playlist);

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(ui.playlist));
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(ui.playlist));

	if (gtk_tree_selection_count_selected_rows(selection) <= 0)
		return;

	list = gtk_tree_selection_get_selected_rows(selection, &model);
	
	for ( ; list != NULL; list = g_list_next(list)){
		GtkTreeIter iter;
		gtk_tree_model_get_iter(model, &iter, (GtkTreePath *)list->data);
		gtk_tree_model_get(model, &iter, SONG_ID, &song_id, -1);
		call_id3_window(song_id);
	}
	
	while ((list = g_list_previous(list)));
	g_list_foreach(list, (GFunc)gtk_tree_path_free, NULL);
	g_list_free(list);
}

static GtkWidget *playlist_menu_create(GtkWidget *widget)
{
	GtkWidget *item = NULL;
	GtkWidget *menu = NULL;

	g_assert(widget);

	menu = gtk_menu_new();	

	/* delete */
	item = gtk_image_menu_item_new_from_stock(GTK_STOCK_REMOVE, NULL);
	gtk_menu_append(GTK_MENU(menu), item);
	g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(playlist_selection_remove), NULL);

	/* shuffle */
	item = gtk_image_menu_item_new_with_label("Shuffle");
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(item),
			gtk_image_new_from_stock(GTK_STOCK_REFRESH, GTK_ICON_SIZE_MENU));
	gtk_menu_append(GTK_MENU(menu), item);
	g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(mpd_playlist_shuffle), connection);

	item = gtk_image_menu_item_new_from_stock(GTK_STOCK_DIALOG_INFO,NULL);
	gtk_menu_append(GTK_MENU(menu), item);
	g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(playlist_show_info), NULL);

	gtk_widget_show_all(menu);
	gtk_widget_tap_and_hold_setup(GTK_WIDGET(widget), menu, NULL, 0);
}
/****************** playlist end ************************/

/* Callback for hardware keys */
static gboolean key_press_cb(GtkWidget * widget, GdkEventKey * event, HildonWindow * window)
{
	switch (event->keyval) {
	case GDK_Left:
		prev_song();
		return TRUE;
	case GDK_Right:
		next_song();
		return TRUE;
	case GDK_F7: {
		int cur_value;
		if (!GTK_WIDGET_VISIBLE(ui.volumebar))
			volumebar_show(window);
		cur_value = hildon_volumebar_get_level(HILDON_VOLUMEBAR(ui.volumebar));
		hildon_volumebar_set_level(HILDON_VOLUMEBAR(ui.volumebar),
					   cur_value + VOLUMEBAR_STEP);
		volumebar_clicked();
		return TRUE;
	}
	case GDK_F8: {
		int cur_value;
		if (!GTK_WIDGET_VISIBLE(ui.volumebar))
			volumebar_show(window);
		cur_value = hildon_volumebar_get_level(HILDON_VOLUMEBAR(ui.volumebar));
		hildon_volumebar_set_level(HILDON_VOLUMEBAR(ui.volumebar),
					   cur_value - VOLUMEBAR_STEP);
		volumebar_clicked();
		return TRUE;
		
		hildon_banner_show_information(GTK_WIDGET(window), NULL, "Decrease (zoom out)");
		return TRUE;
	}
	case GDK_F5:
		/* catch it to not quit fullscreen mode */
		return TRUE;
	case GDK_Escape:
		stop_song();
		return TRUE;
	}
	
	return FALSE;
}

/********************** public player methods ************************/
void player_mpd_state_changed(MpdObj *mi, ChangedStatusType what, void *userdata)
{
	gchar *msg = NULL;
	
	if (what & MPD_CST_RANDOM) {
		int random = mpd_player_get_random(connection);
		if (random !=
		    gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(ui.tb.button_random))) {
			
			gtk_toggle_tool_button_set_active(
				GTK_TOGGLE_TOOL_BUTTON(ui.tb.button_random), random);
		}
		msg = g_strdup_printf("%s: %s", _("Random"), random ? _("On"):_("Off"));
		popup(msg);
		g_free(msg);
	}

	if (what & MPD_CST_REPEAT) {
		int repeat = mpd_player_get_repeat(connection);
		if(repeat !=
		   gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(ui.tb.button_repeat))) {

			gtk_toggle_tool_button_set_active(
				GTK_TOGGLE_TOOL_BUTTON(ui.tb.button_repeat), repeat);
		}
		msg = g_strdup_printf("%s: %s", _("Repeat"), repeat ?_("On"):_("Off"));
		popup(msg);
		g_free(msg);
	}

	if (what & MPD_CST_VOLUME) {
		int volume = mpd_status_get_volume(connection);

		if ((int)hildon_volumebar_get_level(ui.volumebar) != volume)
			hildon_volumebar_set_level(ui.volumebar, (double)volume);
		msg = g_strdup_printf("%s: %d", _("Volume"), volume);
		popup(msg);
		g_free(msg);
	}

	if (what & MPD_CST_CROSSFADE)
	{
		int crossfade = mpd_status_get_crossfade(connection);
		if (crossfade > 0)
			msg = g_strdup_printf("%s: %i sec", _("Crossfade"), crossfade);
		else
			msg = g_strdup_printf("%s: %s", _("Crossfade"), _("Off"));
		popup(msg);
		g_free(msg);
	}

	if (what & MPD_CST_SONGID)
		player_song_changed();

	if (what & MPD_CST_STATE)
		player_update(mpd_player_get_state(connection));

	if (what & MPD_CST_TOTAL_TIME || what & MPD_CST_ELAPSED_TIME)
		update_player();
}

void msg_set_base(char *msg)
{
	gtk_label_set_text(GTK_LABEL(ui.label_cur_song), msg);
}
/********************** public player methods end ************************/

/************************ main player GUI ***********************/
static GtkWidget *toolbar_create()
{
	GtkWidget	*image		= NULL;
	GtkWidget	*toolbar	= NULL;
	GtkToolItem	*item		= NULL;

	toolbar = gtk_toolbar_new();

	image = gtk_image_new();
	gtk_image_set_from_stock(GTK_IMAGE(image), "media-unmuted", GTK_ICON_SIZE_BUTTON);
	item = gtk_tool_button_new(GTK_WIDGET(image), "Volume");
	g_signal_connect(G_OBJECT(item), "clicked", G_CALLBACK(volumebar_show), NULL);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), item, -1);

	item = gtk_tool_button_new_from_stock(GTK_STOCK_PREFERENCES);
	g_signal_connect(G_OBJECT(item), "clicked", G_CALLBACK(create_preferences_window), NULL);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), item, -1);

	image = gtk_image_new();
	gtk_image_set_from_stock(GTK_IMAGE(image), "media-playlist", GTK_ICON_SIZE_BUTTON);
	item = gtk_tool_button_new(GTK_WIDGET(image), "Playlist");
	g_signal_connect(G_OBJECT(item), "clicked", G_CALLBACK(create_playlist3), NULL);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), item, -1);

	image = gtk_image_new();
	gtk_image_set_from_stock(GTK_IMAGE(image), "media-random", GTK_ICON_SIZE_BUTTON);
	ui.tb.button_random = gtk_toggle_tool_button_new();
	gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(ui.tb.button_random), GTK_WIDGET(image));
	gtk_tool_button_set_label(GTK_TOOL_BUTTON(ui.tb.button_random), "Random"); 
	g_signal_connect(G_OBJECT(ui.tb.button_random), "clicked", G_CALLBACK(random_pl), NULL);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), ui.tb.button_random, -1);

	image = gtk_image_new();
	gtk_image_set_from_stock(GTK_IMAGE(image), "media-repeat", GTK_ICON_SIZE_BUTTON);
	ui.tb.button_repeat = gtk_toggle_tool_button_new();
	gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(ui.tb.button_repeat), GTK_WIDGET(image));
	gtk_tool_button_set_label(GTK_TOOL_BUTTON(ui.tb.button_repeat), "Repeat"); 
	g_signal_connect(G_OBJECT(ui.tb.button_repeat), "clicked", G_CALLBACK(repeat_pl), NULL);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), ui.tb.button_repeat, -1);

	/* time elapsed */
	item = gtk_tool_item_new();
	ui.tb.label_time_left = gtk_label_new("00:00");
	gtk_container_add(GTK_CONTAINER(item), GTK_WIDGET(ui.tb.label_time_left));
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), item, -1);

	/* seekbar */
	item = gtk_tool_item_new();
	gtk_tool_item_set_expand(item, TRUE);
	ui.tb.seekbar = HILDON_SEEKBAR(hildon_seekbar_new());
	g_signal_connect_after(G_OBJECT(ui.tb.seekbar), "change-value",
			 G_CALLBACK(seekbar_clicked), NULL);
	gtk_container_add(GTK_CONTAINER(item), GTK_WIDGET(ui.tb.seekbar));
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), item, -1);

	/* total time */
	item = gtk_tool_item_new();
	ui.tb.label_time_total = gtk_label_new("00:00");
	gtk_container_add(GTK_CONTAINER(item), GTK_WIDGET(ui.tb.label_time_total));
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), item, -1);
	
	return toolbar;
}

/* create the player and connect signals */
void player_create()
{
	HildonProgram	*program		= NULL;
	GdkPixbuf	*pixbuf_stop_button	= NULL;
	GdkPixbuf	*pixbuf_forward_button	= NULL;
	GdkPixbuf	*pixbuf_backward_button	= NULL;
	GtkWidget	*hbox			= NULL;
	GtkWidget	*vbox_right		= NULL;
	GtkWidget	*playlist_box		= NULL;
	GtkRequisition	requisition;

	/* Creating Hildonized main view.. */
	program = HILDON_PROGRAM(hildon_program_get_instance());
	g_set_application_name("mmpc");

	ui.main_window = HILDON_WINDOW(hildon_window_new());
	hildon_program_add_window(program, HILDON_WINDOW(ui.main_window));

	hildon_window_add_toolbar(ui.main_window, GTK_TOOLBAR(toolbar_create()));

	mmpc_attach(HILDON_WINDOW(ui.main_window), G_CALLBACK(gtk_main_quit), NULL);

	g_signal_connect(G_OBJECT(ui.main_window), "delete_event",
			 G_CALLBACK(gtk_main_quit), NULL);

	/* GUI layout:

	  ----------------------hbox---------------
                                 |
           vbox_left | volumebar |    vbox_right
           (buttons)             |    (playlist)
                                 |
          -----------------------------------------
	                   toolbar
	*/

	/* hbox */
	hbox = gtk_hbox_new(FALSE, 3);
	gtk_container_set_border_width(GTK_CONTAINER(hbox), 3);

	/* vbox_left */
	ui.vbox_left = gtk_vbox_new(FALSE, 5);
	/* if we want bigger buttons, change this
	gtk_widget_set_size_request(GTK_WIDGET(ui.vbox_left), 150, 100);*/

	ui.pixbufs.play_button = pixbuf_get("media-playback-start.png");
	pixbuf_stop_button = pixbuf_get("media-playback-stop.png");
	ui.pixbufs.pause_button = pixbuf_get("media-playback-pause.png");
	pixbuf_forward_button = pixbuf_get("media-skip-forward.png");
	pixbuf_backward_button = pixbuf_get("media-skip-backward.png");

	ui.play_button = gtk_button_new();
	gtk_button_set_image(GTK_BUTTON(ui.play_button),
			     gtk_image_new_from_pixbuf(ui.pixbufs.play_button));
	g_signal_connect_swapped(G_OBJECT(ui.play_button), "clicked", G_CALLBACK(play_song), NULL);

	ui.stop_button = gtk_button_new();
	gtk_button_set_image(GTK_BUTTON(ui.stop_button),
			     gtk_image_new_from_pixbuf(pixbuf_stop_button));
	g_signal_connect(G_OBJECT(ui.stop_button), "clicked", G_CALLBACK(stop_song), NULL);

	ui.forward_button = gtk_button_new();
	gtk_button_set_image(GTK_BUTTON(ui.forward_button),
			     gtk_image_new_from_pixbuf(pixbuf_forward_button));
	g_signal_connect(G_OBJECT(ui.forward_button), "clicked", G_CALLBACK(next_song), NULL);

	ui.backward_button = gtk_button_new();
	gtk_button_set_image(GTK_BUTTON(ui.backward_button),
			     gtk_image_new_from_pixbuf(pixbuf_backward_button));
	g_signal_connect(G_OBJECT(ui.backward_button), "clicked", G_CALLBACK(prev_song), NULL);

	gtk_container_add(GTK_CONTAINER(ui.vbox_left), ui.forward_button);
	gtk_container_add(GTK_CONTAINER(ui.vbox_left), ui.play_button);
	gtk_container_add(GTK_CONTAINER(ui.vbox_left), ui.stop_button);
	gtk_container_add(GTK_CONTAINER(ui.vbox_left), ui.backward_button);
	gtk_box_pack_start(GTK_BOX(hbox), ui.vbox_left, FALSE, FALSE, 10);

	/* volumebar */
	ui.volumebar = HILDON_VOLUMEBAR(hildon_vvolumebar_new());
	gtk_widget_get_child_requisition(ui.vbox_left, &requisition);
	/* do we want the buttons and the volumebar on the left having the same size?
	gtk_widget_set_size_request(GTK_WIDGET(ui.volumebar), requisition.width, requisition.height);*/
	g_signal_connect(G_OBJECT(ui.volumebar), "level_changed",
			 G_CALLBACK(volumebar_clicked), NULL);
	g_signal_connect(G_OBJECT(ui.volumebar), "mute_toggled",
			 G_CALLBACK(volumebar_mute_toggled), NULL);
	gtk_box_pack_start(GTK_BOX(hbox), GTK_WIDGET(ui.volumebar), FALSE, FALSE, 10);

	/* vbox_right */
	vbox_right = gtk_vbox_new(FALSE, 5);
	/* song label */
	ui.label_cur_song = gtk_label_new(_("Maemo Music Player Client"));
	/* playlist */
	playlist_box = playlist_create();
	playlist_menu_create(ui.playlist);

	gtk_box_pack_start(GTK_BOX(vbox_right), playlist_box, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox_right), ui.label_cur_song, FALSE, FALSE, 0);
	gtk_container_add(GTK_CONTAINER(hbox), vbox_right);

	/* hardware button listener */
	g_signal_connect(G_OBJECT(ui.main_window),  "key_press_event",
			 G_CALLBACK(key_press_cb), ui.main_window);

	gtk_container_add(GTK_CONTAINER(ui.main_window), hbox);
	gtk_widget_show_all(GTK_WIDGET(ui.main_window));

	/* hide the volumebar */
	gtk_widget_hide(GTK_WIDGET(ui.volumebar));
}
/************************ main player GUI ***********************/
