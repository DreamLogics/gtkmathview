/*
 * Copyright (C) 2000, Luca Padovani <luca.padovani@cs.unibo.it>.
 * 
 * This file is part of GtkMathView, a Gtk widget for MathML.
 * 
 * GtkMathView is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * GtkMathView is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with GtkMathView; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 * 
 * For details, see the GtkMathView World-Wide-Web page,
 * http://cs.unibo.it/~lpadovan/mml-widget, or send a mail to
 * <luca.padovani@cs.unibo.it>
 */

#include <config.h>
#include <stdio.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include "gtkmathview.h"
#include "guiGTK.h"

#define XLINK_NS_URI "http://www.w3.org/1999/xlink"

static GtkWidget* window;
static GtkWidget* main_area;
static GtkWidget* scrolled_area;
static GtkWidget* status_bar;
static GtkMenuItem* anti_aliasing_item;
static GtkMenuItem* transparency_item;
static GdkCursor* normal_cursor;
static GdkCursor* link_cursor;  

static gchar* doc_name = NULL;
static GdomeElement* root_selected = NULL;

static guint statusbar_context;

static void create_widget_set(void);
static GtkWidget* get_main_menu(void);
static void file_open(GtkWidget*, gpointer);
static void file_re_open(GtkWidget*, gpointer);
static void file_close(GtkWidget*, gpointer);
static void options_font_manager(GtkWidget*, FontManagerId);
static void options_set_font_size(GtkWidget*, gpointer);
static void options_change_font_size(GtkWidget*, gboolean);
static void options_verbosity(GtkWidget*, guint);
static void options_anti_aliasing(GtkWidget*, gpointer);
static void options_transparency(GtkWidget*, gpointer);
static void selection_delete(GtkWidget*, gpointer);
static void selection_parent(GtkWidget*, gpointer);
static void selection_reset(GtkWidget*, gpointer);
static void help_about(GtkWidget*, gpointer);

static GtkItemFactoryEntry menu_items[] = {
  { "/_File",                          NULL,         NULL,          0, "<Branch>" },
  { "/File/_Open...",                  "<control>O", file_open,     0, NULL },
  { "/File/_Reopen",                   NULL,         file_re_open,  0, NULL },
  { "/File/_Close",                    "<control>W", file_close,    0, NULL },
  { "/File/sep1",                      NULL,         NULL,          0, "<Separator>" },
  { "/File/_Quit",                     "<control>Q", gtk_main_quit, 0, NULL },

  { "/_Selection",                     NULL, NULL,                  0,  "<Branch>" },
  { "/Selection/Reset",                NULL, selection_reset,       0, NULL },
  { "/Selection/Delete",               NULL, selection_delete,      0, NULL },
  { "/Selection/Select Parent",        NULL, selection_parent,      0, NULL },

  { "/_Options",                       NULL, NULL,                  0,  "<Branch>" },
  { "/Options/Default _Font Size",     NULL, NULL,                  0,  "<Branch>" },
  { "/Options/Default Font Size/Set...", NULL, options_set_font_size, 0,  NULL },
  { "/Options/Default Font Size/sep1", NULL, NULL,                  0,  "<Separator>" },
  { "/Options/Default Font Size/Larger", NULL, options_change_font_size, TRUE, NULL },
  { "/Options/Default Font Size/Smaller", NULL, options_change_font_size, FALSE, NULL },
  { "/Options/Font Manager",           NULL, NULL,                  0,  "<Branch>" },
  { "/Options/Font Manager/_GTK",      NULL, options_font_manager,  FONT_MANAGER_GTK, "<RadioItem>" },
  { "/Options/Font Manager/_Type 1",   NULL, options_font_manager,  FONT_MANAGER_T1, "/Options/Font Manager/GTK" },
  { "/Options/Verbosity",              NULL, NULL,                  0,  "<Branch>" },
  { "/Options/Verbosity/_Errors",      NULL, options_verbosity,     0,  "<RadioItem>" },
  { "/Options/Verbosity/_Warnings",    NULL, options_verbosity,     1,  "/Options/Verbosity/Errors" },
  { "/Options/Verbosity/_Info",        NULL, options_verbosity,     2,  "/Options/Verbosity/Errors" },
  { "/Options/Verbosity/_Debug",       NULL, options_verbosity,     3,  "/Options/Verbosity/Errors" },
  { "/Options/sep1",                   NULL, NULL,                  0,  "<Separator>" },
  { "/Options/_Anti Aliasing",         NULL, options_anti_aliasing, 0,  "<ToggleItem>" },
  { "/Options/_Transparency",          NULL, options_transparency,  0,  "<ToggleItem>" },

  { "/_Help" ,        NULL,         NULL,          0, "<LastBranch>" },
  { "/Help/About...", NULL,         help_about,    0, NULL }
};

static void
quick_message(const char* msg)
{
  GtkWidget* dialog;
  GtkWidget* label;
  GtkWidget* okay_button;
     
  /* Create the widgets */
     
  dialog = gtk_dialog_new();
  label = gtk_label_new (msg);
  okay_button = gtk_button_new_with_label("OK");

  gtk_widget_set_usize(dialog, 300, 100);

  /* Ensure that the dialog box is destroyed when the user clicks ok. */
     
  gtk_signal_connect_object (GTK_OBJECT (okay_button), "clicked",
			     GTK_SIGNAL_FUNC (gtk_widget_destroy), dialog);
  gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->action_area),
		     okay_button);
  
  /* Add the label, and show everything we've added to the dialog. */
  
  gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->vbox), label);
  gtk_widget_show_all (dialog);
}

static void
load_error_msg(const char* name)
{
  char* msg = g_strdup_printf("Could not load\n`%s'", name);
  quick_message(msg);
  g_free(msg);
}

void
GUI_init(int* argc, char*** argv, char* title, guint width, guint height)
{
  gtk_init(argc, argv);

  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(window), title);
  gtk_window_set_default_size(GTK_WINDOW(window), width, height);
  gtk_signal_connect(GTK_OBJECT(window), "delete_event", (GtkSignalFunc) gtk_main_quit, NULL);
  create_widget_set();

  gtk_widget_show(window);

  normal_cursor = gdk_cursor_new(GDK_TOP_LEFT_ARROW);
  link_cursor = gdk_cursor_new(GDK_HAND2);
}

void
GUI_uninit()
{
}

int
GUI_load_document(const char* name)
{
  GtkMathView* math_view;

  g_return_val_if_fail(name != NULL, -1);
  g_return_val_if_fail(main_area != NULL, -1);
  g_return_val_if_fail(GTK_IS_MATH_VIEW(main_area), -1);

  math_view = GTK_MATH_VIEW(main_area);

  if (!gtk_math_view_load_uri(math_view, name)) {
    load_error_msg(name);
    return -1;
  }

  if (name != doc_name) {
    if (doc_name != NULL) g_free(doc_name);
    doc_name = g_strdup(name);
  }

  gtk_statusbar_pop(GTK_STATUSBAR(status_bar), statusbar_context);
  if (strlen(name) > 40) name += strlen(name) - 40;
  gtk_statusbar_push(GTK_STATUSBAR(status_bar), statusbar_context, name);
    
  return 0;
}

void
GUI_unload_document()
{
  GtkMathView* math_view;

  g_return_if_fail(main_area != NULL);
  g_return_if_fail(GTK_IS_MATH_VIEW(main_area));

  math_view = GTK_MATH_VIEW(main_area);

  gtk_math_view_unload(math_view);

  if (doc_name != NULL) g_free(doc_name);
  doc_name = NULL;

  gtk_statusbar_pop(GTK_STATUSBAR(status_bar), statusbar_context);
  gtk_statusbar_push(GTK_STATUSBAR(status_bar), statusbar_context, "");
}

void
GUI_run()
{
  gtk_main();
}

void
GUI_set_font_manager(FontManagerId id)
{
  gboolean t1;
  GtkMathView* math_view;

  g_return_if_fail(id != FONT_MANAGER_UNKNOWN);
  g_return_if_fail(main_area != NULL);
  g_return_if_fail(GTK_IS_MATH_VIEW(main_area));

  t1 = id == FONT_MANAGER_T1;

  math_view = GTK_MATH_VIEW(main_area);

  gtk_math_view_freeze(math_view);

  if (id != gtk_math_view_get_font_manager_type(math_view))
    gtk_math_view_set_font_manager_type(math_view, id);

  gtk_widget_set_sensitive(anti_aliasing_item, t1);
  gtk_widget_set_sensitive(transparency_item, t1);

  if (t1)
    {
      gtk_math_view_set_anti_aliasing(math_view, GTK_CHECK_MENU_ITEM(anti_aliasing_item)->active);
      gtk_math_view_set_transparency(math_view, GTK_CHECK_MENU_ITEM(transparency_item)->active);
    }

  gtk_math_view_thaw(math_view);
}

static void
store_filename(GtkFileSelection* selector, GtkWidget* user_data)
{
  gchar* selected_filename = gtk_file_selection_get_filename (GTK_FILE_SELECTION(user_data));
  if (selected_filename != NULL)
    GUI_load_document(selected_filename);
}

static void
file_close(GtkWidget* widget, gpointer data)
{
  GUI_unload_document();
}

static void
file_re_open(GtkWidget* widget, gpointer data)
{
  if (doc_name != NULL) {
    GUI_load_document(doc_name);
  }
}

static void
file_open(GtkWidget* widget, gpointer data)
{
  GtkWidget* fs = gtk_file_selection_new("Open File");

  gtk_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION(fs)->ok_button),
		      "clicked", GTK_SIGNAL_FUNC (store_filename), (gpointer) fs);
                             
  /* Ensure that the dialog box is destroyed when the user clicks a button. */
     
  gtk_signal_connect_object (GTK_OBJECT (GTK_FILE_SELECTION(fs)->ok_button),
			     "clicked", GTK_SIGNAL_FUNC (gtk_widget_destroy),
			     (gpointer) fs);

  gtk_signal_connect_object (GTK_OBJECT (GTK_FILE_SELECTION(fs)->cancel_button),
			     "clicked", GTK_SIGNAL_FUNC (gtk_widget_destroy),
			     (gpointer) fs);
     
  /* Display that dialog */
     
  gtk_widget_show (fs);
}

static void
options_font_manager(GtkWidget* widget, FontManagerId id)
{
  g_return_if_fail(id != FONT_MANAGER_UNKNOWN);
  GUI_set_font_manager(id);
}

static void
options_anti_aliasing(GtkWidget* widget, gpointer data)
{
  gboolean aa = gtk_math_view_get_anti_aliasing(GTK_MATH_VIEW(main_area));
  gtk_math_view_set_anti_aliasing(GTK_MATH_VIEW(main_area), !aa);
}

static void
options_transparency(GtkWidget* widget, gpointer data)
{
  gboolean t = gtk_math_view_get_transparency(GTK_MATH_VIEW(main_area));
  gtk_math_view_set_transparency(GTK_MATH_VIEW(main_area), !t);
}

static void
options_verbosity(GtkWidget* widget, guint level)
{
  gtk_math_view_set_log_verbosity(GTK_MATH_VIEW(main_area), level);
}

static void
selection_delete(GtkWidget* widget, gpointer data)
{
  if (root_selected != NULL)
    {
      GdomeException exc;
      gtk_math_view_freeze(GTK_MATH_VIEW(main_area));
      printf("about to remove element %p\n", root_selected);
      delete_element(root_selected);
      gdome_el_unref(root_selected, &exc);
      g_assert(exc == 0);
      root_selected = NULL;
      gtk_math_view_thaw(GTK_MATH_VIEW(main_area));
    }
}

static void
selection_parent(GtkWidget* widget, gpointer data)
{
  if (root_selected != NULL)
    {
      GdomeException exc = 0;
      GdomeElement* parent = gdome_n_parentNode(root_selected, &exc);
      g_assert(exc == 0);
      gdome_el_unref(root_selected, &exc);
      g_assert(exc == 0);
      root_selected = parent;
      gtk_math_view_select(GTK_MATH_VIEW(main_area), root_selected);
    }
}

static void
selection_reset(GtkWidget* widget, gpointer data)
{
  if (root_selected != NULL)
    {
      GdomeException exc = 0;
      gtk_math_view_unselect(GTK_MATH_VIEW(main_area), root_selected);
      gdome_el_unref(root_selected, &exc);
      g_assert(exc == 0);
      root_selected = NULL;
    }
}

static void
help_about(GtkWidget* widget, gpointer data)
{
  GtkWidget* dialog;
  GtkWidget* label;
  GtkWidget* ok;

  dialog = gtk_dialog_new();
  label = gtk_label_new("\n    MathML Viewer    \n    Copyright (C) 2000-2003 Luca Padovani    \n");
  ok = gtk_button_new_with_label("Close");

  gtk_signal_connect_object (GTK_OBJECT (ok), "clicked",
			     GTK_SIGNAL_FUNC (gtk_widget_destroy), (gpointer) dialog);
  gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->action_area),
		     ok);

  gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->vbox), label);

  gtk_widget_show_all (dialog);
}

static void
change_default_font_size(GtkSpinButton* widget, GtkSpinButton* spin)
{
  g_return_if_fail(spin != NULL);
  gtk_math_view_set_font_size( GTK_MATH_VIEW(main_area), gtk_spin_button_get_value_as_int(spin));
}

static void
options_change_font_size(GtkWidget* widget, gboolean larger)
{
  gfloat size = gtk_math_view_get_font_size (GTK_MATH_VIEW(main_area));
  if (larger) size = size / 0.71;
  else size = size * 0.71;
  if (size < 1) size = 1;
  gtk_math_view_set_font_size (GTK_MATH_VIEW(main_area), (gint) size + 0.5);
}

static void
options_set_font_size(GtkWidget* widget, gpointer data)
{
  GtkWidget* dialog;
  GtkWidget* label;
  GtkWidget* ok;
  GtkWidget* cancel;
  GtkWidget* spin;
  GtkObject* adj;

  dialog = gtk_dialog_new();
  label = gtk_label_new("Default font size:");
  ok = gtk_button_new_with_label("OK");
  cancel = gtk_button_new_with_label("Cancel");

  adj = gtk_adjustment_new (gtk_math_view_get_font_size (GTK_MATH_VIEW(main_area)), 1, 200, 1, 1, 1);
  spin = gtk_spin_button_new (GTK_ADJUSTMENT(adj), 1, 0);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (spin), TRUE);

  gtk_signal_connect (GTK_OBJECT (ok), "clicked",
		      GTK_SIGNAL_FUNC (change_default_font_size), (gpointer) spin);

  gtk_signal_connect_object (GTK_OBJECT (ok), "clicked",
			     GTK_SIGNAL_FUNC (gtk_widget_destroy), (gpointer) dialog);

  gtk_signal_connect_object (GTK_OBJECT (ok), "clicked",
			     GTK_SIGNAL_FUNC (gtk_widget_destroy), (gpointer) dialog);

  gtk_signal_connect_object (GTK_OBJECT (cancel), "clicked",
			     GTK_SIGNAL_FUNC (gtk_widget_destroy), (gpointer) dialog);

  gtk_container_set_border_width (GTK_CONTAINER (GTK_DIALOG(dialog)->vbox), 5);

  gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->action_area), ok);
  gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->action_area), cancel);
  gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->vbox), label);
  gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->vbox), spin);

  gtk_widget_show_all (dialog);
}

#if defined(HAVE_GMETADOM)
static void
element_changed(GtkMathView* math_view, GdomeElement* elem)
{
  GdomeDOMString* link = NULL;

  g_return_if_fail(math_view != NULL);
  g_return_if_fail(GTK_IS_MATH_VIEW(math_view));

/*   printf("pointer is on %p\n", elem); */

  link = find_hyperlink(elem, XLINK_NS_URI, "href");
  if (link != NULL)
    gdk_window_set_cursor(GTK_WIDGET(math_view)->window, link_cursor);
  else
    gdk_window_set_cursor(GTK_WIDGET(math_view)->window, normal_cursor);

  if (link != NULL)
    gdome_str_unref(link);
}
#endif

static void
press_move(GtkMathView* math_view, GdomeElement* first, GdomeElement* last)
{
  g_return_if_fail(math_view != NULL);
  g_return_if_fail(GTK_IS_MATH_VIEW(math_view));
  g_return_if_fail(first != NULL);

/*   printf("selection changed %p %p\n", first, last); */

  if (last != NULL)
    {
      GdomeException exc = 0;

      if (root_selected != NULL)
	{
	  gdome_el_unref(root_selected, &exc);
	  g_assert(exc == 0);
	}

      root_selected = find_common_ancestor(first, last);
/*       printf("selecting root %p\n", first, last, root_selected); */
      gtk_math_view_select(math_view, root_selected);
      g_assert(exc == 0);
    }
}

#if defined(HAVE_GMETADOM)
static void
clicked(GtkMathView* math_view, GdomeElement* elem)
{
  GdomeException exc;
  GdomeDOMString* name;
  GdomeDOMString* ns_uri;
  GdomeElement* p;

  g_return_if_fail(math_view != NULL);

  /* printf("clicked on %p\n", elem); */

  if (elem != NULL)
    {
      GdomeElement* action;
      GdomeDOMString* href = find_hyperlink(elem, XLINK_NS_URI, "href");
      if (href != NULL)
	{
/* 	  printf("hyperlink %s\n", href->str); */
	  gdome_str_unref(href);
	}

      action = find_self_or_ancestor(elem, MATHML_NS_URI, "maction");
/*       printf("action? %p\n", action); */
      if (action != NULL)
	{
	  gtk_math_view_freeze(math_view);
	  action_toggle(action);
	  gtk_math_view_thaw(math_view);
	  gdome_el_unref(action, &exc);
	  g_assert(exc == 0);
	}
    }
}
#endif

static void
create_widget_set()
{
  GtkWidget* main_vbox;
  GtkWidget* menu_bar;

  main_vbox = gtk_vbox_new(FALSE, 1);
  gtk_container_border_width(GTK_CONTAINER(main_vbox), 1);
  gtk_container_add(GTK_CONTAINER(window), main_vbox);
  gtk_widget_show(main_vbox);

  menu_bar = get_main_menu();
  gtk_box_pack_start(GTK_BOX(main_vbox), menu_bar, FALSE, TRUE, 0);
  gtk_widget_show(menu_bar);

  main_area = gtk_math_view_new(NULL, NULL);
  gtk_widget_show(main_area);

  gtk_signal_connect_object (GTK_OBJECT (main_area),
			     "press_move", GTK_SIGNAL_FUNC (press_move),
			     (gpointer) main_area);

  gtk_signal_connect_object (GTK_OBJECT (main_area),
			     "element_changed", GTK_SIGNAL_FUNC (element_changed),
			     (gpointer) main_area);

  gtk_signal_connect_object (GTK_OBJECT (main_area), 
			     "clicked", GTK_SIGNAL_FUNC(clicked),
			     (gpointer) main_area);

  gtk_widget_add_events(GTK_WIDGET(main_area),
			GDK_BUTTON_PRESS_MASK
			| GDK_BUTTON_RELEASE_MASK
			| GDK_POINTER_MOTION_MASK);

  scrolled_area = gtk_scrolled_window_new(NULL, NULL);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_area),
				 GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
  gtk_widget_show(scrolled_area);
  gtk_container_add(GTK_CONTAINER(scrolled_area), main_area);
  gtk_box_pack_start(GTK_BOX(main_vbox), scrolled_area, TRUE, TRUE, 0);

  status_bar = gtk_statusbar_new();
  gtk_widget_show(status_bar);
  gtk_box_pack_start(GTK_BOX(main_vbox), status_bar, FALSE, TRUE, 0);
  statusbar_context = gtk_statusbar_get_context_id(GTK_STATUSBAR(status_bar), "filename");

  gtk_widget_show(main_vbox);

  if (gtk_math_view_get_anti_aliasing(GTK_MATH_VIEW(main_area)))
    gtk_menu_item_activate(anti_aliasing_item);
}

GtkWidget*
get_main_menu()
{
  GtkItemFactory* item_factory;
  GtkAccelGroup* accel_group;
  GtkWidget* menu_item;

  gint nmenu_items = sizeof(menu_items) / sizeof(menu_items[0]);

  accel_group = gtk_accel_group_new();

  item_factory = gtk_item_factory_new(GTK_TYPE_MENU_BAR, "<main>", accel_group);

  gtk_item_factory_create_items(item_factory, nmenu_items, menu_items, NULL);

  gtk_window_add_accel_group(GTK_WINDOW(window), accel_group);

  menu_item = gtk_item_factory_get_widget(item_factory, "/Options/Anti Aliasing");
  anti_aliasing_item = GTK_MENU_ITEM(menu_item);

  menu_item = gtk_item_factory_get_widget(item_factory, "/Options/Transparency");
  transparency_item = GTK_MENU_ITEM(menu_item);

  return gtk_item_factory_get_widget(item_factory, "<main>");
}
