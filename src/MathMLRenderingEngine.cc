// Copyright (C) 2000, Luca Padovani <luca.padovani@cs.unibo.it>.
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
// http://cs.unibo.it/~lpadovan/mml-widget/, or send a mail to
// <luca.padovani@cs.unibo.it>

#include <config.h>
#include <assert.h>

#include "Clock.hh"
#include "Globals.hh"
#include "CharMapper.hh"
#include "StringUnicode.hh"
#include "MathMLDocument.hh"
#include "MathMLParseFile.hh"
#include "MathMLActionElement.hh"
#include "RenderingEnvironment.hh"
#include "MathMLRenderingEngine.hh"
#include "FormattingContext.hh"

#include "config.dirs"

#ifdef HAVE_LIBT1
#include "T1_FontManager.hh"
#include "T1_Gtk_DrawingArea.hh"
#endif

MathMLRenderingEngine::MathMLRenderingEngine()
  : document(0), root(0)
{
  area = NULL;
  fontManager = NULL;
  charMapper = NULL;

  defaultFontSize = Globals::configuration.GetFontSize();
}

MathMLRenderingEngine::~MathMLRenderingEngine()
{
  Unload();
  assert(!document);
  assert(!root);

  delete charMapper;
  charMapper = NULL;

  // WARNING: do we have to delete the font manager?
  fontManager = NULL;
}

void
MathMLRenderingEngine::Init(class DrawingArea* a, class FontManager* fm)
{
  assert(a != NULL);
  assert(fm != NULL);

  area = a;
  fontManager = fm;

  if (charMapper != NULL) delete charMapper;
  charMapper = new CharMapper(*fm);

  if (!Globals::configuration.GetFonts().empty())
    for (std::vector<String*>::const_iterator cit = Globals::configuration.GetFonts().begin();
	 cit != Globals::configuration.GetFonts().end();
	 cit++)
      {
	assert(*cit);
	if (!charMapper->Load((*cit)->ToStaticC()))
	  Globals::logger(LOG_WARNING, "could not load `%s'", (*cit)->ToStaticC());
      }
  else
    {
      bool res = charMapper->Load("config/font-configuration.xml");
      if (!res) charMapper->Load(PKGDATADIR"/font-configuration.xml");
    }
}

bool
MathMLRenderingEngine::Load(const char* fileName)
{
  assert(fileName != NULL);

  Unload();

  Clock perf;
  perf.Start();
#if defined(HAVE_MINIDOM)
  mDOMDocRef doc = MathMLParseFile(fileName, true);
#elif defined(HAVE_GMETADOM)
  GMetaDOM::Document doc = MathMLParseFile(fileName, true);
#endif
  perf.Stop();
  Globals::logger(LOG_INFO, "parsing time: %dms", perf());

  if (!doc) {
    Globals::logger(LOG_WARNING, "error while parsing `%s'", fileName);
    return false;
  }

  return Load(doc);
}

#if defined(HAVE_GMETADOM)
bool
MathMLRenderingEngine::Load(const GMetaDOM::Document& doc)
{
  assert(doc);

  Unload();

  Ptr<MathMLDocument> document = MathMLDocument::create(doc);
  assert(document);

  Clock perf;
  perf.Start();
  document->Normalize();
  perf.Stop();
  Globals::logger(LOG_INFO, "normalization time: %dms", perf());
    
  root = document->GetRoot();
  assert(root);

  Setup();

  return true;
}

bool
MathMLRenderingEngine::Load(const GMetaDOM::Element& elem)
{
  assert(elem);

  Unload();

  Ptr<MathMLDocument> document = MathMLDocument::create(elem);
  assert(document);

  Clock perf;
  perf.Start();
  document->Normalize();
  perf.Stop();
  Globals::logger(LOG_INFO, "normalization time: %dms", perf());
    
  root = document->GetRoot();
  assert(root);

  Setup();

  return true;
}
#endif

void
MathMLRenderingEngine::Unload()
{
  document = 0;
  root = 0;
}

void
MathMLRenderingEngine::Setup()
{
  if (!root) return;

  Clock perf;

  UnitValue size;
  size.Set(defaultFontSize, UNIT_PT);

  assert(charMapper != NULL);
  RenderingEnvironment env(*charMapper);
  env.SetFontSize(size);
  perf.Start();
  root->Setup(&env);
  perf.Stop();
  Globals::logger(LOG_INFO, "setup time: %dms", perf());
  root->SetDirtyLayout(true);

  MinMaxLayout();
}

void
MathMLRenderingEngine::MinMaxLayout()
{
  if (!root) return;

  Clock perf;

  perf.Start();
  root->DoLayout(FormattingContext(LAYOUT_MIN,0));
  perf.Stop();
  Globals::logger(LOG_INFO, "minimum layout time: %dms", perf());

#if 0
  perf.Start();
  root->DoLayout(FormattingContext(LAYOUT_MAX,0));
  perf.Stop();
  Globals::logger(LOG_INFO, "maximum layout time: %dms", perf());
#endif
}

void
MathMLRenderingEngine::Layout()
{
  assert(area != NULL);

  if (!root) return;

  Clock perf;
  perf.Start();
  root->DoLayout(FormattingContext(LAYOUT_AUTO, scaledMax(0, area->GetWidth() -  2 * area->GetXMargin())));
  root->SetPosition(area->GetXMargin(), area->GetYMargin() + root->GetBoundingBox().ascent);
  perf.Stop();
  Globals::logger(LOG_INFO, "layout time: %dms", perf());
}

void
MathMLRenderingEngine::SetDirty(const Rectangle* rect)
{
  if (!root) return;
  root->SetDirty(rect);
}

void
MathMLRenderingEngine::Render(const Rectangle* rect)
{
  if (root) root->SetDirty(rect);
  Update(rect);
}

void
MathMLRenderingEngine::Update(const Rectangle* rect)
{
  assert(area != NULL);

  if (root)
    {
      Clock perf;
      perf.Start();
      root->Render(*area);
      perf.Stop();
      Globals::logger(LOG_INFO, "rendering time: %dms", perf());
    }

  if (rect != NULL) area->Update(*rect);
  else area->Update();
}

void
MathMLRenderingEngine::GetDocumentBoundingBox(BoundingBox& box) const
{
  if (!root)
    {
      box.Null();
      return;
    }

  box = root->GetBoundingBox();  
}

void
MathMLRenderingEngine::GetDocumentRectangle(Rectangle& rect) const
{
  if (root) {
    BoundingBox box;
    GetDocumentBoundingBox(box);
    rect = box.GetRectangle(root->GetX(), root->GetY());
  } else
    rect.Zero();
}

void
MathMLRenderingEngine::SetSelected(const Ptr<MathMLElement>& elem)
{
  if (elem) elem->SetSelected();
  else if (root) root->SetSelected();
    
  Update();
}

void
MathMLRenderingEngine::ResetSelected(const Ptr<MathMLElement>& elem)
{
  if (elem) elem->ResetSelected();
  else if (root) root->ResetSelected();

  Update();
}

Ptr<MathMLElement>
MathMLRenderingEngine::GetElementAt(scaled x, scaled y) const
{
  if (!root) return root;
  // WARNING: x and y must be absolute coordinates w.r.t. the drawing area, because
  // at this level we do not known whether the drawing area is scrollable (as in
  // the case of Gtk_DrawingArea) or not (PS_DrawingArea). The caller must
  // properly adjust x and y before calling this method
  return root->Inside(x, y);
}

void
MathMLRenderingEngine::SetDefaultFontSize(unsigned size)
{
  assert(size > 0);
  defaultFontSize = size;
}

void
MathMLRenderingEngine::SetAntiAliasing(bool aa)
{
  assert(area != NULL);

#ifdef HAVE_LIBT1
  T1_Gtk_DrawingArea* t1_area = TO_T1_GTK_DRAWING_AREA(area);
  if (t1_area != NULL)
    t1_area->SetAntiAliasing(aa);
  else
#endif
    Globals::logger(LOG_WARNING, "anti-aliasing is available with the T1 font manager only");
}

bool
MathMLRenderingEngine::GetAntiAliasing() const
{
  assert(area != NULL);

#ifdef HAVE_LIBT1
  T1_Gtk_DrawingArea* t1_area = TO_T1_GTK_DRAWING_AREA(area);
  if (t1_area != NULL) return t1_area->GetAntiAliasing();
#endif
  Globals::logger(LOG_WARNING, "anti-aliasing is available with the T1 font manager only");
  return false;
}

void
MathMLRenderingEngine::SetKerning(bool b)
{
  assert(area != NULL);

#ifdef HAVE_LIBT1
  T1_Gtk_DrawingArea* t1_area = TO_T1_GTK_DRAWING_AREA(area);
  if (t1_area != NULL)
    t1_area->SetKerning(b);
  else
#endif
    Globals::logger(LOG_WARNING, "kerning is available with the T1 font manager only");
}

bool
MathMLRenderingEngine::GetKerning() const
{
  assert(area != NULL);

#ifdef HAVE_LIBT1
  T1_Gtk_DrawingArea* t1_area = TO_T1_GTK_DRAWING_AREA(area);
  if (t1_area != NULL) return t1_area->GetKerning();
#endif
  Globals::logger(LOG_WARNING, "kerning is available with the T1 font manager only");
  return false;
}

void
MathMLRenderingEngine::SetTransparency(bool b)
{
  assert(area != NULL);

#ifdef HAVE_LIBT1
  T1_Gtk_DrawingArea* t1_area = TO_T1_GTK_DRAWING_AREA(area);
  if (t1_area != NULL)
    t1_area->SetTransparency(b);
  else
#endif
    Globals::logger(LOG_WARNING, "transparency is available with the T1 font manager only");
}

bool
MathMLRenderingEngine::GetTransparency() const
{
  assert(area != NULL);

#ifdef HAVE_LIBT1
  T1_Gtk_DrawingArea* t1_area = TO_T1_GTK_DRAWING_AREA(area);
  if (t1_area != NULL) return t1_area->GetTransparency();
#endif
  Globals::logger(LOG_WARNING, "kerning is available with the T1 font manager only");
  return false;
}
