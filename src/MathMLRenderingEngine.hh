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

#ifndef MathMLRenderingEngine_hh
#define MathMLRenderingEngine_hh

#if defined(HAVE_GMETADOM)
#include "gmetadom.hh"
#endif

#include "Ptr.hh"
#include "scaled.hh"
#include "RGBValue.hh"
#include "MathMLElement.hh"

class MathMLRenderingEngine
{
public:
  MathMLRenderingEngine(void);
  ~MathMLRenderingEngine();

  void  Init(class DrawingArea*, class FontManager*);

#if defined(HAVE_GMETADOM)
  bool  Load(const char*);
  bool  Load(const GMetaDOM::Document&);
#endif
  void  Unload(void);

  void  Setup(void);

  void  MinMaxLayout(void);
  void  Layout(void);
  void  SetDirty(const struct Rectangle* = NULL);
  void  Render(void);
  void  Render(const struct Rectangle*);
  void  Update(const struct Rectangle* = NULL);

  Ptr<MathMLElement> GetRoot(void) const { return root; }
  Ptr<MathMLElement> GetElementAt(scaled, scaled) const;

  enum SelectionMode
  {
    STRUCTURED,
    LINEAR
  };

#if 0
  void                 SetSelectionMode(SelectionMode);
  SelectionMode        GetSelectionMode(void) const { return selectionMode; }
  void                 AddSelected(const Ptr<MathMLElement>&);
#endif
  void                 SetSelected(const Ptr<MathMLElement>&);
  const Ptr<MathMLElement>& GetSelected(void) const { return selected; }

  // BoundingBox, and Rectangle are structs, not classes, 
  void GetDocumentBoundingBox(struct BoundingBox&) const;
  void GetDocumentRectangle(struct Rectangle&) const;

  void     SetDefaultFontSize(unsigned);
  unsigned GetDefaultFontSize(void) const { return defaultFontSize; }

  // T1 specific methods (they produce a warning when used with a
  // GTK drawing area)
  void SetAntiAliasing(bool);
  bool GetAntiAliasing(void) const;
  void SetKerning(bool);
  bool GetKerning(void) const;
  void SetTransparency(bool);
  bool GetTransparency(void) const;

private:
  SelectionMode         selectionMode;
  unsigned              defaultFontSize;

  Ptr<class MathMLDocument> document;
  Ptr<class MathMLElement>  root;
  Ptr<class MathMLElement>  selected;
#if 0
  Ptr<class MathMLElement>  firstSelected;
  Ptr<class MathMLElement>  lastSelected;
  Ptr<class MathMLElement>  rootSelected;
#endif

  class DrawingArea*    area;
  class FontManager*    fontManager;
  class CharMapper*     charMapper;
};

#endif // MathMLRenderingEngine_hh

