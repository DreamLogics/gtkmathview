// Copyright (C) 2000-2002, Luca Padovani <luca.padovani@cs.unibo.it>.
//
// This file is part of GtkMathView, a Gtk widget for MathML.
// 
// GtkMathView is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// GtkMathView is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with GtkMathView; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
// 
// For details, see the GtkMathView World-Wide-Web page,
// http://www.cs.unibo.it/helm/mml-widget, or send a mail to
// <luca.padovani@cs.unibo.it>

#include <config.h>
#include <assert.h>
#include <stdlib.h>

#include "defs.h"

#include <gtk/gtk.h>
#include <gtk/gtkmain.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkdrawingarea.h>

#include "Globals.hh"
#include "Rectangle.hh"
#include "gtkmathview.h"
#include "traverseAux.hh"
#include "MathMLElement.hh"
#include "PS_DrawingArea.hh"
#include "Gtk_FontManager.hh"
#include "PS_T1_FontManager.hh"
#include "T1_Gtk_DrawingArea.hh"
#include "MathMLActionElement.hh"
#include "MathMLRenderingEngine.hh"

/* structures */

struct _GtkMathView {
  GtkEventBox    parent;

  GtkWidget* 	 frame;
  GtkWidget* 	 area;
  GdkPixmap*     pixmap;

  guint 	 hsignal;
  guint 	 vsignal;

  GtkAdjustment* hadjustment;
  GtkAdjustment* vadjustment;

  gfloat     	 top_x;
  gfloat     	 top_y;

  gfloat 	 old_top_x;
  gfloat 	 old_top_y;

  gboolean       frozen;

  FontManagerId  font_manager_id;

  FontManager*     font_manager;
  Gtk_DrawingArea* drawing_area;
  MathMLRenderingEngine* interface;
};

struct _GtkMathViewClass {
  GtkEventBoxClass parent_class;

  void (*set_scroll_adjustments) (GtkMathView *math_view,
				  GtkAdjustment *hadjustment,
				  GtkAdjustment *vadjustment);
};

/* helper functions */

static void gtk_math_view_class_init(GtkMathViewClass*);
static void gtk_math_view_init(GtkMathView*);

/* GtkObject functions */

static void gtk_math_view_destroy(GtkObject*);

/* GtkWidget functions */

static gboolean gtk_math_view_configure_event(GtkWidget*, GdkEventConfigure*, GtkMathView*);
static gboolean gtk_math_view_expose_event(GtkWidget*, GdkEventExpose*, GtkMathView*);
static void     gtk_math_view_realize(GtkWidget*, GtkMathView*);
static void     gtk_math_view_size_request(GtkWidget*, GtkRequisition*);

/* auxiliary functions */

static void setup_adjustment(GtkAdjustment*, gfloat, gfloat);
static void setup_adjustments(GtkMathView*);
static void reset_adjustments(GtkMathView*);

/* Local data */

static GtkEventBoxClass* parent_class = NULL;

/* widget implementation */

static void
paint_widget_area(GtkMathView* math_view, gint x, gint y, gint width, gint height)
{
  GtkWidget* widget;

  g_return_if_fail(math_view != NULL);
  g_return_if_fail(math_view->area != NULL);
  g_return_if_fail(math_view->interface != NULL);

  if (!GTK_WIDGET_MAPPED(GTK_WIDGET(math_view)) || math_view->frozen) return;

  widget = math_view->area;

  gdk_draw_rectangle(math_view->pixmap, widget->style->white_gc, TRUE, x, y, width, height);

  Rectangle rect;
  rect.x = px2sp(x) + float2sp(math_view->top_x);
  rect.y = px2sp(y) + float2sp(math_view->top_y);
  rect.width = px2sp(width);
  rect.height = px2sp(height);

  math_view->interface->Render(&rect);
}

static void
paint_widget(GtkMathView* math_view)
{
  GtkWidget* widget;

  g_return_if_fail(math_view != NULL);
  g_return_if_fail(math_view->area != NULL);

  widget = math_view->area;

  paint_widget_area(math_view, 0, 0, widget->allocation.width, widget->allocation.height);
}

static void
hadjustment_value_changed(GtkAdjustment* adj, GtkMathView* math_view)
{
  g_return_if_fail(adj != NULL);
  g_return_if_fail(math_view != NULL);
  g_return_if_fail(math_view->drawing_area != NULL);

  if (adj->value > adj->upper - adj->page_size) adj->value = adj->upper - adj->page_size;
  if (adj->value < adj->lower) adj->value = adj->lower;

  math_view->old_top_x = math_view->top_x;
  math_view->top_x = adj->value;
  math_view->drawing_area->SetTopX(float2sp(adj->value));

  if (math_view->old_top_x != math_view->top_x) {
#if 0
    gint change = sp2ipx(float2sp(fabs(math_view->old_top_x - math_view->top_x)));
    GtkWidget* widget = math_view->area;
    if (change < widget->allocation.width) {
      if (math_view->old_top_x < math_view->top_x) {
	// the window scrolled right
	gdk_draw_pixmap(math_view->pixmap,
			widget->style->white_gc,
			math_view->pixmap,
			change, 0, 0, 0,
			widget->allocation.width - change,
			widget->allocation.height);

	paint_widget_area(math_view,
			  widget->allocation.width - change, 0,
			  change, widget->allocation.height);
      } else {
	// the window scrolled left
	gdk_draw_pixmap(math_view->pixmap,
			widget->style->white_gc,
			math_view->pixmap,
			0, 0, change, 0,
			widget->allocation.width - change,
			widget->allocation.height);

	paint_widget_area(math_view,
			  0, 0,
			  change, widget->allocation.height);
      }
      
      gtk_widget_draw(math_view->area, NULL);
    } else
#endif
      paint_widget(math_view);
  }
}

static void
vadjustment_value_changed(GtkAdjustment* adj, GtkMathView* math_view)
{
  g_return_if_fail(adj != NULL);
  g_return_if_fail(math_view != NULL);

  if (adj->value > adj->upper - adj->page_size) adj->value = adj->upper - adj->page_size;
  if (adj->value < adj->lower) adj->value = adj->lower;

  math_view->old_top_y = math_view->top_y;
  math_view->top_y = adj->value;
  math_view->drawing_area->SetTopY(float2sp(adj->value));
  
  if (math_view->old_top_y != math_view->top_y) {
#if 0
    gint change = sp2ipx(float2sp(fabs(math_view->old_top_y - math_view->top_y)));
    GtkWidget* widget = math_view->area;
    if (change < widget->allocation.height) {
      if (math_view->old_top_y < math_view->top_y) {
	// the window scrolled down
	gdk_draw_pixmap(math_view->pixmap,
			widget->style->white_gc,
			math_view->pixmap,
			0, change, 0, 0,
			widget->allocation.width,
			widget->allocation.height - change);

	paint_widget_area(math_view,
			  0, widget->allocation.height - change,
			  widget->allocation.width, change);
      } else {
	// the window scrolled up
	gdk_draw_pixmap(math_view->pixmap,
			widget->style->white_gc,
			math_view->pixmap,
			0, 0, 0, change,
			widget->allocation.width,
			widget->allocation.height - change);

	paint_widget_area(math_view,
			  0, 0,
			  widget->allocation.width, change);
      }

      gtk_widget_draw(math_view->area, NULL);
    } else
#endif
      paint_widget(math_view);
  }
}

extern "C" GtkType
gtk_math_view_get_type()
{
  static guint math_view_type = 0;

  if (math_view_type == 0) {
    GtkTypeInfo math_view_info = {
      "GtkMathView",
      sizeof(GtkMathView),
      sizeof(GtkMathViewClass),
      (GtkClassInitFunc) gtk_math_view_class_init,
      (GtkObjectInitFunc) gtk_math_view_init,
      NULL,
      NULL,
      NULL
    };

    math_view_type = gtk_type_unique(gtk_event_box_get_type(), &math_view_info);
  }

  return math_view_type;
}

static void
gtk_math_view_class_init(GtkMathViewClass* klass)
{
  GtkObjectClass* object_class = (GtkObjectClass*) klass;
  GtkWidgetClass* widget_class = (GtkWidgetClass*) klass;

  klass->set_scroll_adjustments = gtk_math_view_set_adjustments;

  parent_class = (GtkEventBoxClass*) gtk_type_class(gtk_event_box_get_type());

  object_class->destroy = gtk_math_view_destroy;

  widget_class->size_request = gtk_math_view_size_request;
  widget_class->set_scroll_adjustments_signal =
    gtk_signal_new("set_scroll_adjustments",
		   GTK_RUN_LAST,
		   object_class->type,
		   GTK_SIGNAL_OFFSET (GtkMathViewClass, set_scroll_adjustments),
		   gtk_marshal_NONE__POINTER_POINTER,
		   GTK_TYPE_NONE, 2, GTK_TYPE_ADJUSTMENT, GTK_TYPE_ADJUSTMENT);

  Globals::InitGlobalData(getenv("MATHENGINECONF"));
}

static void
gtk_math_view_init(GtkMathView* math_view)
{
  g_return_if_fail(math_view != NULL);

  math_view->pixmap          = NULL;
  math_view->font_manager_id = FONT_MANAGER_UNKNOWN;
  math_view->font_manager    = NULL;
  math_view->drawing_area    = NULL;
  math_view->interface       = NULL;
  math_view->frozen          = FALSE;

  math_view->frame = gtk_frame_new(NULL);
  gtk_frame_set_shadow_type(GTK_FRAME(math_view->frame), GTK_SHADOW_IN);
  gtk_container_add(GTK_CONTAINER(math_view), math_view->frame);
  gtk_widget_show(math_view->frame);

  math_view->area = gtk_drawing_area_new();
  GTK_WIDGET_SET_FLAGS(GTK_WIDGET(math_view->area), GTK_CAN_FOCUS);
  gtk_container_add(GTK_CONTAINER(math_view->frame), math_view->area);
  gtk_widget_show(math_view->area);

  gtk_signal_connect(GTK_OBJECT(math_view->area), "configure_event",
		     GTK_SIGNAL_FUNC(gtk_math_view_configure_event), math_view);

  gtk_signal_connect(GTK_OBJECT(math_view->area), "expose_event",
		     GTK_SIGNAL_FUNC(gtk_math_view_expose_event), math_view);

  gtk_signal_connect(GTK_OBJECT(math_view->area), "realize",
		     GTK_SIGNAL_FUNC(gtk_math_view_realize), math_view);

  math_view->hadjustment = NULL;
  math_view->vadjustment = NULL;
}

extern "C" GtkWidget*
gtk_math_view_new(GtkAdjustment*, GtkAdjustment*)
{
  GtkMathView* math_view = (GtkMathView*) gtk_type_new(gtk_math_view_get_type());
  
  g_return_val_if_fail(math_view != NULL, NULL);

  math_view->top_x = math_view->top_y = 0;
  math_view->old_top_x = math_view->old_top_y = 0;
  math_view->interface = new MathMLRenderingEngine();

  gtk_math_view_set_font_manager_type(math_view, FONT_MANAGER_GTK);

  return GTK_WIDGET(math_view);
}

static void
gtk_math_view_destroy(GtkObject* object)
{
  GtkMathView* math_view;

  g_return_if_fail(object != NULL);
  g_return_if_fail(GTK_IS_MATH_VIEW(object));

  math_view = GTK_MATH_VIEW(object);
  g_assert(math_view != NULL);
  g_return_if_fail(math_view->interface != NULL);

  Globals::logger(LOG_DEBUG, "destroying the widget");

  delete math_view->interface;
  delete math_view->font_manager;
  delete math_view->drawing_area;

  /* FIXME: sometimes the frame has been destroyed already
   * if you disable the delete_event and close the window you'll see
   * some messages
   */

  /* ATTEMPT: since this class is derived from a container
   * then contained object will be destroyed by the parent class'
   * method
   */

  gtk_signal_disconnect_by_data(GTK_OBJECT(math_view->hadjustment), math_view);
  gtk_signal_disconnect_by_data(GTK_OBJECT(math_view->vadjustment), math_view);

  if (GTK_OBJECT_CLASS(parent_class)->destroy != NULL)
    (*GTK_OBJECT_CLASS(parent_class)->destroy)(object);
}

extern "C" gboolean
gtk_math_view_freeze(GtkMathView* math_view)
{
  gboolean old_frozen;

  g_return_val_if_fail(math_view != NULL, FALSE);

  old_frozen = math_view->frozen;
  math_view->frozen = TRUE;

  return old_frozen;
}

extern "C" gboolean
gtk_math_view_thaw(GtkMathView* math_view)
{
  gboolean old_frozen;

  g_return_val_if_fail(math_view != NULL, FALSE);

  old_frozen = math_view->frozen;
  math_view->frozen = FALSE;
  
  if (old_frozen) paint_widget(math_view);

  return old_frozen;
}

static void
gtk_math_view_realize(GtkWidget* widget, GtkMathView* math_view)
{
  g_return_if_fail(widget != NULL);
  g_return_if_fail(math_view != NULL);
  g_return_if_fail(math_view->drawing_area != NULL);

  math_view->drawing_area->Realize();
}

static void
gtk_math_view_size_request(GtkWidget* widget, GtkRequisition* requisition)
{
  g_return_if_fail(widget != NULL);
  g_return_if_fail(requisition != NULL);
  g_return_if_fail(GTK_IS_MATH_VIEW(widget));

  GtkMathView* math_view = GTK_MATH_VIEW(widget);
  g_assert(math_view != NULL);
  g_assert(math_view->interface != NULL);

  BoundingBox box;
  math_view->interface->GetDocumentBoundingBox(box);

  // the 10 is for the border, the frame thickness is missing. How can I get it?
  requisition->width = sp2ipx(box.width) + 10;
  requisition->height = sp2ipx(box.GetHeight()) + 10;
}

static gint
gtk_math_view_configure_event(GtkWidget* widget,
			      GdkEventConfigure* event,
			      GtkMathView* math_view)
{
  g_return_val_if_fail(widget != NULL, FALSE);
  g_return_val_if_fail(event != NULL, FALSE);
  g_return_val_if_fail(math_view != NULL, FALSE);
  g_return_val_if_fail(math_view->interface != NULL, FALSE);
  g_return_val_if_fail(math_view->drawing_area != NULL, FALSE);

  if (math_view->pixmap != NULL) gdk_pixmap_unref(math_view->pixmap);
  math_view->pixmap = gdk_pixmap_new(widget->window, event->width, event->height, -1);
  math_view->drawing_area->SetSize(px2sp(event->width), px2sp(event->height));
  math_view->drawing_area->SetPixmap(math_view->pixmap);
  setup_adjustments(math_view);
  paint_widget(math_view);

  return TRUE;
}

static gint
gtk_math_view_expose_event(GtkWidget* widget,
			   GdkEventExpose* event,
			   GtkMathView* math_view)
{
  g_return_val_if_fail(widget != NULL, FALSE);
  g_return_val_if_fail(event != NULL, FALSE);
  g_return_val_if_fail(math_view != NULL, FALSE);

  gdk_draw_pixmap(widget->window,
		  widget->style->fg_gc[GTK_WIDGET_STATE(widget)],
		  math_view->pixmap,
		  event->area.x, event->area.y,
		  event->area.x, event->area.y,
		  event->area.width, event->area.height);

  return FALSE;
}

static void
setup_adjustment(GtkAdjustment* adj, gfloat size, gfloat page_size)
{
  g_return_if_fail(adj != NULL);

  adj->lower = 0.0;
  adj->page_size = page_size;
  adj->step_increment = 10 * SCALED_POINTS_PER_PX;
  adj->page_increment = page_size;
  adj->upper = size + 10 * SCALED_POINTS_PER_PX;
  if (adj->upper < 0) adj->upper = 0.0;

  if (adj->value > adj->upper - page_size) {
    adj->value = floatMax(0, adj->upper - page_size);
    gtk_adjustment_value_changed(adj);
  }

  gtk_adjustment_changed(adj);
}

static void
reset_adjustments(GtkMathView* math_view)
{
  g_return_if_fail(math_view != NULL);

  math_view->old_top_x = math_view->old_top_y = math_view->top_x = math_view->top_y = 0;

  if (math_view->hadjustment != NULL)
    gtk_adjustment_set_value(math_view->hadjustment, 0.0);

  if (math_view->vadjustment != NULL)
    gtk_adjustment_set_value(math_view->vadjustment, 0.0);
}

static void
setup_adjustments(GtkMathView* math_view)
{
  g_return_if_fail(math_view != NULL);
  g_return_if_fail(math_view->area != NULL);
  g_return_if_fail(math_view->interface != NULL);

  BoundingBox box;
  math_view->interface->GetDocumentBoundingBox(box);

  if (math_view->hadjustment != NULL) {
    gfloat width = sp2float(box.width);
    gfloat page_width = sp2float(math_view->drawing_area->GetWidth());
    
    if (math_view->top_x > width - page_width)
      math_view->top_x = floatMax(0, width - page_width);

    setup_adjustment(math_view->hadjustment, width, page_width);
  }

  if (math_view->vadjustment != NULL) {
    gfloat height = sp2float(box.GetHeight());
    gfloat page_height = sp2float(math_view->drawing_area->GetHeight());

    if (math_view->top_y > height - page_height)
      math_view->old_top_y = math_view->top_y = floatMax(0, height - page_height);

    setup_adjustment(math_view->vadjustment, height, page_height);
  }
}

extern "C" gboolean
gtk_math_view_load_uri(GtkMathView* math_view, const gchar* name)
{
  g_return_val_if_fail(math_view != NULL, FALSE);
  g_return_val_if_fail(name != NULL, FALSE);
  g_return_val_if_fail(math_view->interface != NULL, FALSE);

  bool res = math_view->interface->Load(name);
  if (!res) return FALSE;

  setup_adjustments(math_view);
  reset_adjustments(math_view);
  paint_widget(math_view);

  return TRUE;
}

extern "C" gboolean
gtk_math_view_load_doc(GtkMathView* math_view, GdomeDocument* doc)
{
  g_return_val_if_fail(math_view != NULL, FALSE);
  g_return_val_if_fail(doc != NULL, FALSE);
  g_return_val_if_fail(math_view->interface != NULL, FALSE);

  bool res = math_view->interface->Load(GMetaDOM::Document(doc));
  if (!res) return FALSE;

  setup_adjustments(math_view);
  reset_adjustments(math_view);
  paint_widget(math_view);

  return TRUE;
}

#if 0
extern "C" gboolean
gtk_math_view_load_tree(GtkMathView* math_view, GdomeElement* elem)
{
  g_return_val_if_fail(math_view != NULL, FALSE);
  g_return_val_if_fail(elem != NULL, FALSE);
  g_return_val_if_fail(math_view->interface != NULL, FALSE);

  bool res = math_view->interface->Load(GMetaDOM::Element(elem));
  if (!res) return FALSE;

  setup_adjustments(math_view);
  reset_adjustments(math_view);
  paint_widget(math_view);

  return TRUE;
}
#endif

extern "C" void
gtk_math_view_unload(GtkMathView* math_view)
{
  g_return_if_fail(math_view != NULL);
  g_return_if_fail(math_view->interface != NULL);

  math_view->interface->Unload();
  setup_adjustments(math_view);
  reset_adjustments(math_view);
  paint_widget(math_view);
}

extern "C" GdkPixmap*
gtk_math_view_get_buffer(GtkMathView* math_view)
{
  g_return_val_if_fail(math_view != NULL, NULL);

  return math_view->pixmap;
}

extern "C" void
gtk_math_view_set_adjustments(GtkMathView* math_view,
			      GtkAdjustment* hadj,
			      GtkAdjustment* vadj)
{
  g_return_if_fail(math_view != NULL);
  g_return_if_fail(GTK_IS_MATH_VIEW(math_view));

  if (hadj != NULL)
    g_return_if_fail(GTK_IS_ADJUSTMENT(hadj));
  else
    hadj = GTK_ADJUSTMENT(gtk_adjustment_new(0.0, 0.0, 0.0, 0.0, 0.0, 0.0));

  if (vadj != NULL)
    g_return_if_fail(GTK_IS_ADJUSTMENT(vadj));
  else
    vadj = GTK_ADJUSTMENT(gtk_adjustment_new(0.0, 0.0, 0.0, 0.0, 0.0, 0.0));

  if (math_view->hadjustment != NULL && (math_view->hadjustment != hadj)) {
    gtk_signal_disconnect_by_data(GTK_OBJECT(math_view->hadjustment), math_view);
    gtk_object_unref(GTK_OBJECT(math_view->hadjustment));
  }

  if (math_view->vadjustment != NULL && (math_view->vadjustment != vadj)) {
    gtk_signal_disconnect_by_data(GTK_OBJECT(math_view->vadjustment), math_view);
    gtk_object_unref(GTK_OBJECT(math_view->vadjustment));
  }

  if (math_view->hadjustment != hadj) {
    math_view->hadjustment = hadj;
    gtk_object_ref(GTK_OBJECT(math_view->hadjustment));
    gtk_object_sink(GTK_OBJECT(math_view->hadjustment));

    math_view->hsignal = gtk_signal_connect(GTK_OBJECT(hadj), "value_changed",
					    GTK_SIGNAL_FUNC(hadjustment_value_changed),
					    math_view);
  }

  if (math_view->vadjustment != vadj) {
    math_view->vadjustment = vadj;
    gtk_object_ref(GTK_OBJECT(math_view->vadjustment));
    gtk_object_sink(GTK_OBJECT(math_view->vadjustment));

    math_view->vsignal = gtk_signal_connect(GTK_OBJECT(vadj), "value_changed",
					    GTK_SIGNAL_FUNC(vadjustment_value_changed),
					    math_view);
  }

  setup_adjustments(math_view);
}

extern "C" GtkAdjustment*
gtk_math_view_get_hadjustment(GtkMathView* math_view)
{
  g_return_val_if_fail(math_view != NULL, NULL);

  return math_view->hadjustment;
}

extern "C" GtkAdjustment*
gtk_math_view_get_vadjustment(GtkMathView* math_view)
{
  g_return_val_if_fail(math_view != NULL, NULL);

  return math_view->vadjustment;
}

extern "C" GtkFrame*
gtk_math_view_get_frame(GtkMathView* math_view)
{
  g_return_val_if_fail(math_view != NULL, NULL);
  return math_view->frame != NULL ? GTK_FRAME(math_view->frame) : NULL;
}

extern "C" GtkDrawingArea*
gtk_math_view_get_drawing_area(GtkMathView* math_view)
{
  g_return_val_if_fail(math_view != NULL, NULL);
  return math_view->area != NULL ? GTK_DRAWING_AREA(math_view->area) : NULL;
}

extern "C" void
gtk_math_view_set_font_size(GtkMathView* math_view, guint size)
{
  g_return_if_fail(math_view != NULL);
  g_return_if_fail(math_view->area != NULL);
  g_return_if_fail(math_view->interface != NULL);
  g_return_if_fail(size > 0);

  if (size == math_view->interface->GetDefaultFontSize()) return;

  math_view->interface->SetDefaultFontSize(size);
  setup_adjustments(math_view);
  paint_widget(math_view);
}

extern "C" guint
gtk_math_view_get_font_size(GtkMathView* math_view)
{
  g_return_val_if_fail(math_view != NULL, 0);
  g_return_val_if_fail(math_view->interface != NULL, 0);

  return math_view->interface->GetDefaultFontSize();
}

extern "C" void
gtk_math_view_set_selection(GtkMathView* math_view, GdomeElement* elem)
{
  g_return_if_fail(math_view != NULL);
  g_return_if_fail(math_view->interface != NULL);
  g_return_if_fail(elem != NULL);

  math_view->interface->SetSelected(findMathMLElement(math_view->interface->GetDocument(),
						      GMetaDOM::Element(elem)));
  paint_widget(math_view);
}

extern "C" void
gtk_math_view_reset_selection(GtkMathView* math_view, GdomeElement* elem)
{
  g_return_if_fail(math_view != NULL);
  g_return_if_fail(math_view->interface != NULL);
  g_return_if_fail(elem != NULL);

  math_view->interface->ResetSelected(findMathMLElement(math_view->interface->GetDocument(),
							GMetaDOM::Element(elem)));
  paint_widget(math_view);
}

extern "C" gboolean
gtk_math_view_is_selected(GtkMathView* math_view, GdomeElement* elem)
{
  g_return_val_if_fail(math_view != NULL, FALSE);
  g_return_val_if_fail(math_view->interface != NULL, FALSE);
  g_return_val_if_fail(elem != NULL, FALSE);

  Ptr<MathMLElement> el = findMathMLElement(math_view->interface->GetDocument(),
					    GMetaDOM::Element(elem));
  if (!el) return FALSE;

  return el->Selected() ? TRUE : FALSE;
}

extern "C" gint
gtk_math_view_get_width(GtkMathView* math_view)
{
  g_return_val_if_fail(math_view != NULL, FALSE);
  g_return_val_if_fail(math_view->area != NULL, FALSE);

  return math_view->area->allocation.width;
}

extern "C" gint
gtk_math_view_get_height(GtkMathView* math_view)
{
  g_return_val_if_fail(math_view != NULL, FALSE);
  g_return_val_if_fail(math_view->area != NULL, FALSE);

  return math_view->area->allocation.height;
}

extern "C" void
gtk_math_view_set_anti_aliasing(GtkMathView* math_view, gboolean anti_aliasing)
{
  g_return_if_fail(math_view != NULL);
  g_return_if_fail(math_view->interface != NULL);
  
  math_view->interface->SetAntiAliasing(anti_aliasing != FALSE);
  paint_widget(math_view);
}

extern "C" gboolean
gtk_math_view_get_anti_aliasing(GtkMathView* math_view)
{
  g_return_val_if_fail(math_view != NULL, FALSE);
  g_return_val_if_fail(math_view->interface != NULL, FALSE);

  return math_view->interface->GetAntiAliasing() ? TRUE : FALSE;
}

extern "C" void
gtk_math_view_set_transparency(GtkMathView* math_view, gboolean transparency)
{
  g_return_if_fail(math_view != NULL);
  g_return_if_fail(math_view->interface != NULL);
  
  math_view->interface->SetTransparency(transparency != FALSE);
  paint_widget(math_view);
}

extern "C" gboolean
gtk_math_view_get_transparency(GtkMathView* math_view)
{
  g_return_val_if_fail(math_view != NULL, FALSE);
  g_return_val_if_fail(math_view->interface != NULL, FALSE);

  return math_view->interface->GetTransparency() ? TRUE : FALSE;
}

extern "C" GdomeElement*
gtk_math_view_get_element_at(GtkMathView* math_view, gint x, gint y)
{
  g_return_val_if_fail(math_view != NULL, NULL);
  g_return_val_if_fail(math_view->interface != NULL, NULL);
  g_return_val_if_fail(math_view->vadjustment != NULL, NULL);
  g_return_val_if_fail(math_view->hadjustment != NULL, NULL);

  Ptr<MathMLElement> at = math_view->interface->GetElementAt(math_view->hadjustment->value + px2sp(x),
							     math_view->vadjustment->value + px2sp(y));

  GdomeException exc = 0;
  GdomeElement* res = gdome_cast_el(findDOMNode(at).gdome_object());
  if (res != 0) gdome_el_ref(res, &exc);
  assert(exc == 0);

  return res;
}

extern "C" gboolean
gtk_math_view_get_element_coords(GtkMathView* math_view, GdomeElement* elem, gint* x, gint* y)
{
  g_return_val_if_fail(math_view != NULL, FALSE);
  g_return_val_if_fail(math_view->interface != NULL, FALSE);
  g_return_val_if_fail(elem != NULL, FALSE);

  Ptr<MathMLElement> el = findMathMLElement(math_view->interface->GetDocument(),
					    GMetaDOM::Element(elem));
  if (!el) return FALSE;

  if (x != NULL) *x = static_cast<gint>(sp2px(el->GetX()));
  if (y != NULL) *y = static_cast<gint>(sp2px(el->GetY()));

  return TRUE;
}

extern "C" gboolean
gtk_math_view_get_element_rectangle(GtkMathView* math_view, GdomeElement* elem, GdkRectangle* rect)
{
  g_return_val_if_fail(math_view != NULL, FALSE);
  g_return_val_if_fail(math_view->interface != NULL, FALSE);
  g_return_val_if_fail(elem != NULL, FALSE);
  g_return_val_if_fail(rect != NULL, FALSE);

  Ptr<MathMLElement> el = findMathMLElement(math_view->interface->GetDocument(),
					    GMetaDOM::Element(elem));
  if (!el) return FALSE;

  const BoundingBox& box = el->GetBoundingBox();
  rect->x = sp2ipx(el->GetX());
  rect->y = sp2ipx(el->GetY() - box.ascent);
  rect->width = sp2ipx(box.width);
  rect->height = sp2ipx(box.GetHeight());

  return TRUE;
}

extern "C" void
gtk_math_view_get_top(GtkMathView* math_view, gint* x, gint* y)
{
  g_return_if_fail(math_view != NULL);
  g_return_if_fail(math_view->vadjustment != NULL);
  g_return_if_fail(math_view->hadjustment != NULL);

  if (x != NULL) *x = (gint) (math_view->hadjustment->value / SCALED_POINTS_PER_PX);
  if (y != NULL) *y = (gint) (math_view->vadjustment->value / SCALED_POINTS_PER_PX);
}

extern "C" void
gtk_math_view_set_top(GtkMathView* math_view, gint x, gint y)
{
  gboolean old_frozen;

  g_return_if_fail(math_view != NULL);
  g_return_if_fail(math_view->vadjustment != NULL);
  g_return_if_fail(math_view->hadjustment != NULL);

  math_view->hadjustment->value = px2sp(x);
  math_view->vadjustment->value = px2sp(y);

  old_frozen = math_view->frozen;
  math_view->frozen = TRUE;
  gtk_adjustment_value_changed(math_view->hadjustment);
  math_view->frozen = old_frozen;
  gtk_adjustment_value_changed(math_view->vadjustment);
}

extern "C" void
gtk_math_view_set_log_verbosity(GtkMathView*, gint level)
{
  Globals::SetVerbosity(level);
}

extern "C" gint
gtk_math_view_get_log_verbosity(GtkMathView*)
{
  return Globals::GetVerbosity();
}

extern "C" void
gtk_math_view_set_font_manager_type(GtkMathView* math_view, FontManagerId id)
{
  g_return_if_fail(math_view != NULL);
  g_return_if_fail(math_view->interface != NULL);
  g_return_if_fail(id != FONT_MANAGER_UNKNOWN);

  if (id == math_view->font_manager_id) return;

  Ptr<MathMLDocument> document = math_view->interface->GetDocument();
  if (document) document->ReleaseGCs();

  delete math_view->font_manager;
  delete math_view->drawing_area;

  math_view->font_manager = NULL;
  math_view->drawing_area = NULL;

  math_view->font_manager_id = id;

  GraphicsContextValues values;
  values.foreground = Globals::configuration.GetForeground();
  values.background = Globals::configuration.GetBackground();
  values.lineStyle  = LINE_STYLE_SOLID;
  values.lineWidth  = px2sp(1);

  switch (id) {
  case FONT_MANAGER_T1:
#ifdef HAVE_LIBT1
    math_view->font_manager = new PS_T1_FontManager;
    math_view->drawing_area = new T1_Gtk_DrawingArea(values, px2sp(5), px2sp(5),
						     GTK_WIDGET(math_view->area),
						     Globals::configuration.GetSelectForeground(),
						     Globals::configuration.GetSelectBackground());
    math_view->drawing_area->SetPixmap(math_view->pixmap);
    break;
#else
    Globals::logger(LOG_WARNING, "the widget was compiled without support for T1 fonts, falling back to GTK fonts");
#endif // HAVE_LIBT1
  case FONT_MANAGER_GTK:
    math_view->font_manager = new Gtk_FontManager;
    math_view->drawing_area = new Gtk_DrawingArea(values, px2sp(5), px2sp(5),
						  GTK_WIDGET(math_view->area),
						  Globals::configuration.GetSelectForeground(),
						  Globals::configuration.GetSelectBackground());
    math_view->drawing_area->SetPixmap(math_view->pixmap);
    break;
  default:
    Globals::logger(LOG_ERROR, "could not switch to font manager type %d", id);
    break;
  }

  math_view->interface->Init(math_view->drawing_area, math_view->font_manager);
  if (GTK_WIDGET_REALIZED(GTK_WIDGET(math_view))) math_view->drawing_area->Realize();
  setup_adjustments(math_view);
  paint_widget(math_view);
}

extern "C" FontManagerId
gtk_math_view_get_font_manager_type(GtkMathView* math_view)
{
  g_return_val_if_fail(math_view != NULL, FONT_MANAGER_UNKNOWN);
  return math_view->font_manager_id;
}

extern "C" void
gtk_math_view_export_to_postscript(GtkMathView* math_view,
				   gint w, gint h,
				   gint x0, gint y0,
				   gboolean disable_colors,
				   FILE* f)
{
  g_return_if_fail(math_view != NULL);
  g_return_if_fail(math_view->interface != NULL);
  g_return_if_fail(math_view->drawing_area != NULL);

  if (math_view->font_manager_id != FONT_MANAGER_T1) {
    Globals::logger(LOG_ERROR, "cannot export to PostScript if the Type1 Font Manager is not available");
    return;
  }

#ifdef HAVE_LIBT1
  PS_T1_FontManager* fm = TO_PS_T1_FONT_MANAGER(math_view->font_manager);
  g_assert(fm != NULL);

  PS_DrawingArea area(math_view->drawing_area->GetDefaultGraphicsContextValues(),
		      px2sp(x0), px2sp(y0), f);
  area.SetSize(px2sp(w), px2sp(h));
  if (disable_colors) area.DisableColors();

  if (Ptr<MathMLDocument> document = math_view->interface->GetDocument())
    {
      // the following invocations are needed just to mark the chars actually used :(
      fm->ResetUsedChars();
      area.SetOutputFile(NULL);
      document->SetDirty();
      document->Render(area);
      area.SetOutputFile(f);

      Rectangle rect;
      math_view->interface->GetDocumentRectangle(rect);
      area.DumpHeader(PACKAGE, "(no title)", rect);
      fm->DumpFontDictionary(f);
      area.DumpPreamble();
      document->SetDirty();
      document->Render(area);
      area.DumpEpilogue();
    }
#else
  g_assert_not_reached();
#endif // HAVE_LIBT1
}

extern "C" guint
gtk_math_view_action_get_selected(GtkMathView* math_view, GdomeElement* elem)
{
  g_return_val_if_fail(math_view != NULL, 0);
  g_return_val_if_fail(math_view->interface != NULL, 0);
  g_return_val_if_fail(elem != NULL, 0);

  Ptr<MathMLActionElement> action_element =
    smart_cast<MathMLActionElement>(findMathMLElement(math_view->interface->GetDocument(),
						      GMetaDOM::Element(elem)));
  if (!action_element) return 0;

  return action_element->GetSelectedIndex();
}

extern "C" void
gtk_math_view_action_set_selected(GtkMathView* math_view, GdomeElement* elem, guint idx)
{
  g_return_if_fail(math_view != NULL);
  g_return_if_fail(math_view->interface != NULL);
  g_return_if_fail(elem != NULL);

  Ptr<MathMLActionElement> action_element =
    smart_cast<MathMLActionElement>(findMathMLElement(math_view->interface->GetDocument(),
						      GMetaDOM::Element(elem)));
  if (!action_element) return;

  action_element->SetSelectedIndex(idx);
  setup_adjustments(math_view);
  paint_widget(math_view);
}

extern "C" void
gtk_math_view_action_toggle(GtkMathView* math_view, GdomeElement* elem)
{
  g_return_if_fail(math_view != NULL);
  g_return_if_fail(math_view->interface != NULL);
  g_return_if_fail(elem != NULL);

  Ptr<MathMLActionElement> action_element =
    smart_cast<MathMLActionElement>(findMathMLElement(math_view->interface->GetDocument(),
						      GMetaDOM::Element(elem)));
  if (!action_element) return;

  guint idx = action_element->GetSelectedIndex();
  if (idx < action_element->GetSize())
    idx++;
  else
    idx = 1;

  gtk_math_view_action_set_selected(math_view, elem, idx);
}

