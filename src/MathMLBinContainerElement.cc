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

#include <config.h>

#include <assert.h>
#include <stdio.h>

#include "Layout.hh"
#include "Iterator.hh"
#include "ShapeFactory.hh"
#include "RenderingEnvironment.hh"
#include "MathMLBinContainerElement.hh"

MathMLBinContainerElement::MathMLBinContainerElement()
{
  child = NULL;
}

#if defined(HAVE_GMETADOM)
MathMLBinContainerElement::MathMLBinContainerElement(const GMetaDOM::Element& node)
#endif
  : MathMLContainerElement(node)
{
  child = NULL;
}

MathMLBinContainerElement::~MathMLBinContainerElement()
{
  if (child != NULL)
    {
      child->Release();
      child = NULL;
    }
}

void
MathMLBinContainerElement::Normalize()
{
  if (HasDirtyStructure() || HasChildWithDirtyStructure())
    {
      if (child != 0) child->Normalize();
      ResetDirtyStructure();
    }
}

void
MathMLBinContainerElement::Setup(RenderingEnvironment* env)
{
  assert(env != NULL);

  if (HasDirtyAttribute() || HasChildWithDirtyAttribute())
    {
      background = env->GetBackgroundColor();
      if (child != 0) child->Setup(env);
      ResetDirtyAttribute();
    }
}

void
MathMLBinContainerElement::DoBoxedLayout(LayoutId id, BreakId bid, scaled availWidth)
{
  if (!HasDirtyLayout(id, availWidth)) return;

  ResetLayout();
  if (child != 0) child->DoBoxedLayout(id, bid, availWidth);
  ResetDirtyLayout(id, availWidth);
}

void
MathMLBinContainerElement::DoLayout(LayoutId id, Layout& layout)
{
  if (child != 0)
    {
      if (child->IsBreakable()) child->DoLayout(id, layout);
      else layout.Append(child, 0);
    }

  ResetDirtyLayout(id);
}

void
MathMLBinContainerElement::DoStretchyLayout()
{
  if (child != 0) child->DoStretchyLayout();
}

void
MathMLBinContainerElement::Freeze()
{
  if (child != 0) child->Freeze();

  if (!IsBreakable() || HasLayout()) MathMLElement::Freeze();
  else
    {
      if (shape != NULL) delete shape;
      ShapeFactory shapeFactory;
      if (child != 0)
	{
	  shapeFactory.Add(child->GetShape());
	  if (child->IsLast()) shapeFactory.SetNewRow();
	}
      shape = shapeFactory.GetShape();
    }
}

void
MathMLBinContainerElement::Render(const DrawingArea& area)
{
  if (!HasDirtyChildren()) return;

  RenderBackground(area);
  if (child != 0) child->Render(area);
  ResetDirty();
}

void
MathMLBinContainerElement::GetLinearBoundingBox(BoundingBox& b) const
{
  if (!IsBreakable())
    b = box;
  else
    {
      b.Null();
      if (child != 0) 
	{
	  BoundingBox elemBox;
	  child->GetLinearBoundingBox(elemBox);
	  b.Append(elemBox);
	}
    }
}

MathMLElement*
MathMLBinContainerElement::Inside(scaled x, scaled y)
{
  if (!IsInside(x, y)) return NULL;

  if (child != 0)
    {
      MathMLElement* inside = child->Inside(x, y);
      if (inside != 0) return inside;
    }
  
  AddRef();

  return this;
}

void
MathMLBinContainerElement::SetDirtyLayout(bool children)
{
  MathMLElement::SetDirtyLayout(children);
  if (children && child != 0) child->SetDirtyLayout(children);
}

void
MathMLBinContainerElement::SetDirty(const Rectangle* rect)
{
  assert(IsShaped());

  dirtyBackground =
    (GetParent() != NULL && (GetParent()->IsSelected() != IsSelected())) ? 1 : 0;

  if (IsDirty()) return;
  if (rect != NULL && !shape->Overlaps(*rect)) return;

  dirty = 1;
  SetDirtyChildren();

  if (child != 0) child->SetDirty(rect);
}

void
MathMLBinContainerElement::SetSelected()
{
  if (IsSelected()) return;

  selected = 1;
  if (child != 0) child->SetSelected();

  SetDirty();
}

void
MathMLBinContainerElement::ResetSelected()
{
  if (!IsSelected()) return;

  SetDirty();

  if (child != 0) child->ResetSelected();
  selected = 0;
}

void
MathMLBinContainerElement::ResetLast()
{
  last = 0;
  if (child != 0) child->ResetLast();
}

bool
MathMLBinContainerElement::IsExpanding() const
{
  if (child != 0) return child->IsExpanding();
  return false;
}

bool
MathMLBinContainerElement::IsLast() const
{
  if (last != 0) return true;
  if (child != 0) return child->IsLast();
  return false;
}

BreakId
MathMLBinContainerElement::GetBreakability() const
{
  if (child != 0 && IsBreakable())
    return child->GetBreakability();

  return BREAK_AUTO;
}

scaled
MathMLBinContainerElement::GetLeftEdge() const
{
  if (child != 0) return child->GetLeftEdge();
  else return 0;
}

scaled
MathMLBinContainerElement::GetRightEdge() const
{
  if (child != 0) return child->GetRightEdge();
  else return 0;
}

void
MathMLBinContainerElement::ReleaseGCs()
{
  MathMLElement::ReleaseGCs();
  if (child != 0) child->ReleaseGCs();
}

void
MathMLBinContainerElement::Remove(MathMLElement* elem)
{
  assert(elem != 0);
  assert(elem == child);

  elem->SetParent(0);
  elem->Release();
  child = 0;

  SetDirtyStructure();
}

void
MathMLBinContainerElement::Replace(MathMLElement* oldElem, MathMLElement* newElem)
{
  assert(oldElem != 0);
  assert(newElem != 0);
  assert(oldElem == child);

  if (oldElem != newElem)
    {
      Remove(oldElem);
      newElem->AddRef();
      newElem->SetParent(this);
      child = newElem;
      SetDirtyStructure();
    }  
}

MathMLElement*
MathMLBinContainerElement::GetChild() const
{
  if (child != NULL) child->AddRef();
  return child;
}

void
MathMLBinContainerElement::SetChild(MathMLElement* elem)
{
  if (elem != NULL) elem->AddRef();
  if (child != NULL) elem->Release();
  child = elem;
}
