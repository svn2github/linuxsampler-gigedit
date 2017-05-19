/*
 * Copyright (C) 2006, 2007 Andreas Persson
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with program; see the file COPYING. If not, write to the Free
 * Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#if HAVE_CONFIG_H
# include <config.h>
#endif

#include <gtk/gtk.h>
#include "gigedit.h"
#include <gtkmm.h>

#if GTKMM_MAJOR_VERSION >= 3

/**
 * This is required since GTK 3, because those GTK super heros came up with
 * the clever idea to simply disable things like icons and keyboard shortcuts
 * for menus and for buttons by default for all users, all devices and all
 * apps. Yey! Seriously, I have no idea what came on their mind to find that
 * was a good idea!
 */
static void enforceGtk3Settings(int argc, char* argv[]) {
    gtk_init(&argc, &argv);

    // got not behavior change on those 2 settings, so ignoring them for now,
    // actually I though I could use them to show the mnemonics in the GTK 3
    // menus again, but it seems that was entirely removed from around GTK 3.10.
    //g_object_set(gtk_settings_get_default(), "gtk-auto-mnemonics", false, NULL);
    //g_object_set(gtk_settings_get_default(), "gtk-can-change-accels", true, NULL);

    // bring back keyboard accelerators with GTK 3
    g_object_set(gtk_settings_get_default(), "gtk-enable-accels", true, NULL);
    g_object_set(gtk_settings_get_default(), "gtk-enable-mnemonics", true, NULL);

    // bring back icons with GTK 3
    g_object_set(gtk_settings_get_default(), "gtk-menu-images", true, NULL);
    g_object_set(gtk_settings_get_default(), "gtk-button-images", true, NULL);

    // who knows ... one day those GTK "masterminds" decide to disable tooltips by default as well
    g_object_set(gtk_settings_get_default(), "gtk-enable-tooltips", true, NULL);
}

#endif // GTKM 3

#if defined(WIN32)
#include <windows.h>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    LPSTR lpCmdLine, int nCmdShow)
{
    #if GTKMM_MAJOR_VERSION >= 3
    enforceGtk3Settings(__argc, __argv);
    #endif

    GigEdit app;
    return app.run(__argc, __argv);
}

#else

int main(int argc, char* argv[])
{
    #if GTKMM_MAJOR_VERSION >= 3
    enforceGtk3Settings(argc, argv);
    #endif

#ifdef __APPLE__
    // remove the argument added by the OS
    if (argc > 1 && strncmp(argv[1], "-psn", 4) == 0) {
        argc--;
        for (int i = 1 ; i < argc ; i++) {
            argv[i] = argv[i + 1];
        }
    }
#endif
    GigEdit app;
    return app.run(argc, argv);
}

#endif
