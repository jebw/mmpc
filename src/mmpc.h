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

#ifndef __MAEMO_GMPC__
#define __MAEMO_GMPC__

#include <hildon/hildon-window.h>
#include <hildon/hildon-program.h>

#define MMPC_FS_PW 0

gboolean	mmpc_fs_get_fullscreen();
void		mmpc_attach(HildonWindow *window, GCallback close_func, gpointer data);

#endif
