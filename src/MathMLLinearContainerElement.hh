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
// http://cs.unibo.it/~lpadovan/mml-widget, or send a mail to
// <luca.padovani@cs.unibo.it>

#ifndef MathMLLinearContainerElement_hh
#define MathMLLinearContainerElement_hh

#if defined(HAVE_GMETADOM)
#include "gmetadom.hh"
#endif

#include "MathMLContainerElement.hh"

class MathMLLinearContainerElement: public MathMLContainerElement
{
protected:
  MathMLLinearContainerElement(void);
#if defined(HAVE_GMETADOM)
  MathMLLinearContainerElement(const GMetaDOM::Element&);
#endif
  virtual ~MathMLLinearContainerElement();

public:
  virtual void Normalize(void);
  virtual void Setup(RenderingEnvironment*);
  virtual void DoLayout(LayoutId, Layout&);
  virtual void DoBoxedLayout(LayoutId, BreakId = BREAK_NO, scaled = 0);
  virtual void DoStretchyLayout(void);
  virtual void Freeze(void);
  virtual void Render(const DrawingArea&);
  virtual void ReleaseGCs(void);
  virtual MathMLElement* Inside(scaled, scaled);

  virtual void SetDirtyLayout(bool = false);
  virtual void SetDirty(const Rectangle* = NULL);
  virtual void SetSelected(void);
  virtual void ResetSelected(void);
  virtual void ResetLast(void);

  virtual bool IsLast(void) const;
  virtual bool IsExpanding(void) const;
  virtual void GetLinearBoundingBox(BoundingBox&) const;
  virtual BreakId GetBreakability(void) const;
  virtual scaled GetLeftEdge(void) const;
  virtual scaled GetRightEdge(void) const;

  // the content can be accessed directly, but only in a read-only
  // way, because other operation involves SetParent and other
  // memory-management issues
  const Container<MathMLElement*>& GetContent(void) const { return content; }

  unsigned GetSize(void) const { return content.GetSize(); }
  void     SetSize(unsigned);
  MathMLElement* GetChild(unsigned) const;
  void     SetChild(unsigned, MathMLElement*);

  virtual void Append(MathMLElement*);
  virtual void Remove(MathMLElement*);
  virtual void Replace(MathMLElement*, MathMLElement*);
  void         Prepend(MathMLElement*);
  void         RemoveFirst(void);
  void         RemoveLast(void);

protected:
  Container<MathMLElement*> content;
};

#define TO_LINEAR_CONTAINER(object) (dynamic_cast<MathMLLinearContainerElement*>(object))

#endif // MathMLLinearContainerElement_hh
