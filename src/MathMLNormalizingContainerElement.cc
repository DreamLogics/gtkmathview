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
#include <stddef.h>

#include "Layout.hh"
#include "MathMLRowElement.hh"
#include "MathMLDummyElement.hh"
#include "MathMLNormalizingContainerElement.hh"

MathMLNormalizingContainerElement::MathMLNormalizingContainerElement()
{
}

#if defined(HAVE_GMETADOM)
MathMLNormalizingContainerElement::MathMLNormalizingContainerElement(const GMetaDOM::Element& node)
  : MathMLBinContainerElement(node)
{
}
#endif

MathMLNormalizingContainerElement::~MathMLNormalizingContainerElement()
{
}

#include <stdio.h>

void
MathMLNormalizingContainerElement::Normalize()
{
  if (HasDirtyStructure() || HasChildWithDirtyStructure())
    {
#if defined(HAVE_GMETADOM)
      GMetaDOM::NodeList children = GetDOMElement().getElementsByTagNameNS(MATHML_NS_URI, "*");
      if (children.get_length() == 1)
	{
	  GMetaDOM::Node node = children.item(0);
	  assert(node.get_nodeType() == GMetaDOM::Node::ELEMENT_NODE);
	  MathMLElement* elem = MathMLElement::getRenderingInterface(node);
	  assert(elem != 0);
	  SetChild(elem);
	  elem->Release();
	}
      else if (child == NULL || child->GetDOMElement() != GetDOMElement())
	{
	  MathMLElement* row = MathMLRowElement::create(GetDOMElement());
	  assert(row != NULL);
	  SetChild(row);
	  row->Release();
	}
#else
      if (child == NULL)
	{
	  MathMLElement* row = MathMLRowElement::create();
	  assert(row != NULL);
	  SetChild(row);
	  row->Release();
	}
#endif

      assert(child != NULL);
      child->Normalize();

      ResetDirtyStructure();
    }
}

void
MathMLNormalizingContainerElement::DoBoxedLayout(LayoutId id, BreakId bid, scaled maxWidth)
{
  if (!HasDirtyLayout(id, maxWidth)) return;

  assert(child != NULL);

  child->DoBoxedLayout(id, bid, maxWidth);
  box = child->GetBoundingBox();

  ConfirmLayout(id);

  ResetDirtyLayout(id, maxWidth);

#if 0
  printf("`%s' DoBoxedLayout (%d,%d,%d) [%d,%d]\n",
	 NameOfTagId(IsA()), id, bid, sp2ipx(maxWidth),
	 sp2ipx(box.width), sp2ipx(box.GetHeight()));
#endif  
}

void
MathMLNormalizingContainerElement::RecalcBoundingBox(LayoutId id, scaled minWidth)
{
  assert(child != NULL);

  child->RecalcBoundingBox(id, minWidth);
  box = child->GetBoundingBox();

  ConfirmLayout(id);
}

void
MathMLNormalizingContainerElement::SetPosition(scaled x, scaled y)
{
  position.x = x;
  position.y = y;

  if (HasLayout()) layout->SetPosition(x, y);
  else {
    assert(child != NULL);
    child->SetPosition(x, y);
  }
}

void
MathMLNormalizingContainerElement::Render(const DrawingArea& area)
{
  if (!HasDirtyChildren()) return;

  RenderBackground(area);

  assert(child != 0);
  child->Render(area);

  ResetDirty();
}

bool
MathMLNormalizingContainerElement::IsExpanding() const
{
  assert(child != NULL);
  return child->IsExpanding();
}

class MathMLOperatorElement*
MathMLNormalizingContainerElement::GetCoreOperator()
{
  assert(child != NULL);

  switch (IsA())
    {
    case TAG_MSTYLE:
    case TAG_MPHANTOM:
    case TAG_MPADDED:
      return child->GetCoreOperator();
    default:
      return NULL;
    }
}
