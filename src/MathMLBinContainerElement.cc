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
#include "ChildList.hh"
#include "ShapeFactory.hh"
#include "RenderingEnvironment.hh"
#include "MathMLBinContainerElement.hh"

MathMLBinContainerElement::MathMLBinContainerElement()
{
}

#if defined(HAVE_GMETADOM)
MathMLBinContainerElement::MathMLBinContainerElement(const GMetaDOM::Element& node)
#endif
  : MathMLContainerElement(node)
{
}

MathMLBinContainerElement::~MathMLBinContainerElement()
{
}

void
MathMLBinContainerElement::Normalize()
{
  if (HasDirtyStructure() || HasChildWithDirtyStructure())
    {
#if defined(HAVE_GMETADOM)
      ChildList children(GetDOMElement(), MATHML_NS_URI, "*");
      if (children.get_length() > 0)
	{
	  GMetaDOM::Node node = children.item(0);
	  assert(node.get_nodeType() == GMetaDOM::Node::ELEMENT_NODE);

	  Ptr<MathMLElement> elem = MathMLElement::getRenderingInterface(node);
	  // it might be that we get a NULL. In that case it would probably make
	  // sense to create a dummy element, because we filtered MathML
	  // elements only
	  assert(elem != 0);
	  SetChild(elem);
	}
#endif // HAVE_GMETADOM

      if (child != 0) child->Normalize();
      ResetDirtyStructure();
    }
}

void
MathMLBinContainerElement::Setup(RenderingEnvironment* env)
{
  assert(env != NULL);

  background = env->GetBackgroundColor();
  if (child != 0) child->Setup(env);
  ResetDirtyAttribute();
}

void
MathMLBinContainerElement::DoLayout(LayoutId id, scaled availWidth)
{
  if (!HasDirtyLayout()) return;
  if (child != 0) child->DoLayout(id, availWidth);
  ResetDirtyLayout(id);
}

void
MathMLBinContainerElement::DoStretchyLayout()
{
  if (child != 0) child->DoStretchyLayout();
}

void
MathMLBinContainerElement::Render(const DrawingArea& area)
{
  if (!HasDirtyChildren()) return;

  RenderBackground(area);
  if (child != 0) child->Render(area);
  ResetDirty();
}

Ptr<MathMLElement>
MathMLBinContainerElement::Inside(scaled x, scaled y)
{
  if (!IsInside(x, y)) return NULL;

  if (child != 0)
    {
      Ptr<MathMLElement> inside = child->Inside(x, y);
      if (inside != 0) return inside;
    }
  
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
  dirtyBackground =
    (GetParent() != NULL && (GetParent()->IsSelected() != IsSelected())) ? 1 : 0;

  if (IsDirty()) return;
  if (rect != NULL && !GetRectangle().Overlaps(*rect)) return;

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

bool
MathMLBinContainerElement::IsExpanding() const
{
  if (child != 0) return child->IsExpanding();
  return false;
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
MathMLBinContainerElement::Remove(const Ptr<MathMLElement>& elem)
{
  assert(elem != 0);
  assert(elem == child);

  elem->SetParent(0);
  child = 0;

  SetDirtyStructure();
}

void
MathMLBinContainerElement::Replace(const Ptr<MathMLElement>& oldElem,
				   const Ptr<MathMLElement>& newElem)
{
  assert(oldElem != 0);
  assert(newElem != 0);
  assert(oldElem == child);

  if (oldElem != newElem)
    {
      Remove(oldElem);
      newElem->SetParent(this);
      child = newElem;
      SetDirtyStructure();
    }  
}

void
MathMLBinContainerElement::SetChild(const Ptr<MathMLElement>& elem)
{
  if (elem != 0) elem->SetParent(this);
  child = elem;
}
