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
#include "MathMLLinearContainerElement.hh"

MathMLLinearContainerElement::MathMLLinearContainerElement()
{
}

#if defined(HAVE_GMETADOM)
MathMLLinearContainerElement::MathMLLinearContainerElement(const GMetaDOM::Element& node)
  : MathMLContainerElement(node)
{
}
#endif

MathMLLinearContainerElement::~MathMLLinearContainerElement()
{
  for (Iterator<MathMLElement*> i(content); i.More(); i.Next()) {
    assert(i() != 0);
    i()->Release();
  }
}

void
MathMLLinearContainerElement::Normalize()
{
  if (HasDirtyStructure() || HasChildWithDirtyStructure())
    {
      // editing is supported with GMetaDOM only
#if defined(HAVE_GMETADOM)
      GMetaDOM::NodeList children = GetDOMElement().getElementsByTagNameNS(MATHML_NS_URI, "*");
      for (unsigned i = 0; i < children.get_length(); i++)
	{
	  GMetaDOM::Node node = children.item(i);
	  assert(node.get_nodeType() == GMetaDOM::Node::ELEMENT_NODE);

	  MathMLElement* elem = MathMLElement::getRenderingInterface(node);
	  // it might be that we get a NULL. In that case it would probably make
	  // sense to create a dummy element, because we filtered MathML
	  // elements only
	  assert(elem != 0);
	  SetChild(i, elem);
	  elem->Release();
	}

      // the following is to be sure that no spurious elements remain at the
      // end of the container
      SetSize(children.get_length());
#endif // HAVE_GMETADOM

      // it is better to normalize elements only after all the rendering
      // interfaces have been collected, because the structure might change
      // depending on the actual number of children

      for (Iterator<MathMLElement*> elem(content); elem.More(); elem.Next())
	{
	  assert(elem() != 0);
	  printf("this: %p normalize: %p\n", this, elem());
	  elem()->Normalize();
	}

      ResetDirtyStructure();
    }
}

void
MathMLLinearContainerElement::Setup(RenderingEnvironment* env)
{
  assert(env != NULL);

  background = env->GetBackgroundColor();

  for (Iterator<MathMLElement*> elem(content); elem.More(); elem.Next()) {
    assert(elem() != NULL);
    elem()->Setup(env);
  }
}

void
MathMLLinearContainerElement::DoBoxedLayout(LayoutId id, BreakId bid, scaled availWidth)
{
  if (!HasDirtyLayout(id, availWidth)) return;

  ResetLayout();

  if (IsBreakable()) {
    layout = new Layout(availWidth, bid);
    DoLayout(id, *layout);
    layout->DoLayout(id);
    if (id == LAYOUT_AUTO) DoStretchyLayout();
    layout->GetBoundingBox(box, id);

    ConfirmLayout(id);

#if 0
    cout << '`' << NameOfTagId(IsA()) << '\'';
    cout << " (" << id << ',' << bid << ',' << sp2ipx(availWidth) << ')';
    cout << box << endl;
#endif
  } else {
    // an unbreakable container element will have all of its
    // children boxed, however the minimum box is to be called
    // by the overriding method!
    for (Iterator<MathMLElement*> elem(content); elem.More(); elem.Next()) {
      assert(elem() != NULL);
      elem()->DoBoxedLayout(id, bid, availWidth);
    }
  }

  ResetDirtyLayout(id, availWidth);
}

void
MathMLLinearContainerElement::DoLayout(LayoutId id, Layout& layout)
{
  for (Iterator<MathMLElement*> elem(content); elem.More(); elem.Next()) {
    assert(elem() != NULL);
    if (elem()->IsBreakable()) elem()->DoLayout(id, layout);
    else layout.Append(elem(), 0);
  }

  ResetDirtyLayout(id);
}

void
MathMLLinearContainerElement::DoStretchyLayout()
{
  for (Iterator<MathMLElement*> elem(content); elem.More(); elem.Next()) {
    assert(elem() != NULL);
    elem()->DoStretchyLayout();
  }
}

void
MathMLLinearContainerElement::Freeze()
{
  for (Iterator<MathMLElement*> elem(content); elem.More(); elem.Next()) {
    assert(elem() != NULL);
    elem()->Freeze();
  }

  if (!IsBreakable() || HasLayout()) MathMLElement::Freeze();
  else {
    if (shape != NULL) delete shape;
    ShapeFactory shapeFactory;
    for (Iterator<MathMLElement*> elem(content); elem.More(); elem.Next()) {
      assert(elem() != NULL);
      shapeFactory.Add(elem()->GetShape());
      if (elem()->IsLast()) shapeFactory.SetNewRow();
    }
    shape = shapeFactory.GetShape();
  }
}

void
MathMLLinearContainerElement::Render(const DrawingArea& area)
{
  if (!HasDirtyChildren()) return;

  RenderBackground(area);

  for (Iterator<MathMLElement*> elem(content); elem.More(); elem.Next()) {
    assert(elem() != NULL);
    elem()->Render(area);
  }

  ResetDirty();
}

void
MathMLLinearContainerElement::GetLinearBoundingBox(BoundingBox& b) const
{
  if (!IsBreakable())
    b = box;
  else {
    b.Null();
    for (Iterator<MathMLElement*> elem(content); elem.More(); elem.Next()) {
      assert(elem() != NULL);
      BoundingBox elemBox;
      elem()->GetLinearBoundingBox(elemBox);
      b.Append(elemBox);
    }
  }
}

MathMLElement*
MathMLLinearContainerElement::Inside(scaled x, scaled y)
{
  if (!IsInside(x, y)) return NULL;

  for (Iterator<MathMLElement*> elem(content); elem.More(); elem.Next()) {
    assert(elem() != NULL);

    MathMLElement* inside = elem()->Inside(x, y);
    if (inside != NULL) return inside;
  }

  return this;
}

void
MathMLLinearContainerElement::SetDirtyLayout(bool children)
{
  MathMLElement::SetDirtyLayout(children);
  if (children) {
    for (Iterator<MathMLElement*> elem(content); elem.More(); elem.Next()) {
      assert(elem() != NULL);
      elem()->SetDirtyLayout(children);
    }
  }
}

void
MathMLLinearContainerElement::SetDirty(const Rectangle* rect)
{
  assert(IsShaped());

#if 0
  if (rect != NULL) {
    cout << NameOfTagId(IsA()) << " container set dirty: shape " << *shape << " rect " << *rect;
    cout << " overlaps? " << shape->Overlaps(*rect) << endl;
  } else {
    cout << NameOfTagId(IsA()) << " container set dirty!" << endl;
  }
#endif

  dirtyBackground =
    (GetParent() != NULL && (GetParent()->IsSelected() != IsSelected())) ? 1 : 0;

  if (IsDirty()) return;
  if (rect != NULL && !shape->Overlaps(*rect)) return;

  dirty = 1;
  SetDirtyChildren();

  for (Iterator<MathMLElement*> elem(content); elem.More(); elem.Next()) {
    assert(elem() != NULL);
    elem()->SetDirty(rect);
  }
}

void
MathMLLinearContainerElement::SetSelected()
{
  if (IsSelected()) return;

  selected = 1;

  for (Iterator<MathMLElement*> elem(content); elem.More(); elem.Next()) {
    assert(elem() != NULL);
    elem()->SetSelected();
  }

  SetDirty();
}

void
MathMLLinearContainerElement::ResetSelected()
{
  if (!IsSelected()) return;

  SetDirty();

  for (Iterator<MathMLElement*> elem(content); elem.More(); elem.Next()) {
    assert(elem() != NULL);
    elem()->ResetSelected();
  }  

  selected = 0;
}

void
MathMLLinearContainerElement::ResetLast()
{
  last = 0;
  for (Iterator<MathMLElement*> i(content); i.More(); i.Next()) {
    MathMLElement* elem = i();
    assert(elem != NULL);

    elem->ResetLast();
  }  
}

bool
MathMLLinearContainerElement::IsExpanding() const
{
  for (Iterator<MathMLElement*> i(content); i.More(); i.Next()) {
    MathMLElement* elem = i();
    assert(elem != NULL);

    if (elem->IsExpanding()) return true;
  }  
  
  return false;
}

bool
MathMLLinearContainerElement::IsLast() const
{
  if (last != 0) return true;
  if (content.GetSize() > 0) {
    assert(content.GetLast() != NULL);
    return content.GetLast()->IsLast();
  } else
    return false;
}

BreakId
MathMLLinearContainerElement::GetBreakability() const
{
  if (content.GetSize() > 0 &&
      content.GetLast() != NULL &&
      IsBreakable())
    return content.GetLast()->GetBreakability();

  return BREAK_AUTO;
}

scaled
MathMLLinearContainerElement::GetLeftEdge() const
{
  scaled edge = 0;

  for (Iterator<MathMLElement*> elem(content); elem.More(); elem.Next()) {
    assert(elem() != NULL);
    if (elem.IsFirst()) edge = elem()->GetLeftEdge();
    else edge = scaledMin(edge, elem()->GetLeftEdge());
  }

  return edge;
}

scaled
MathMLLinearContainerElement::GetRightEdge() const
{
  scaled edge = 0;

  for (Iterator<MathMLElement*> elem(content); elem.More(); elem.Next()) {
    assert(elem() != NULL);
    if (elem.IsFirst()) edge = elem()->GetRightEdge();
    else edge = scaledMax(edge, elem()->GetRightEdge());
  }

  return edge;
}

void
MathMLLinearContainerElement::ReleaseGCs()
{
  MathMLElement::ReleaseGCs();

  for (Iterator<MathMLElement*> elem(content); elem.More(); elem.Next()) {
    assert(elem() != NULL);
    elem()->ReleaseGCs();
  }
}

void
MathMLLinearContainerElement::SetSize(unsigned size)
{
  assert(size <= GetSize());
  while (size < GetSize()) Remove(content.GetLast());
}

MathMLElement*
MathMLLinearContainerElement::GetChild(unsigned i) const
{
  assert(i < GetSize());
  MathMLElement* elem = content.Get(i);
  assert(elem != 0);
  elem->AddRef();
  return elem;
}

void
MathMLLinearContainerElement::SetChild(unsigned i, MathMLElement* elem)
{
  assert(i <= GetSize());
  assert(elem != 0);

  if (i == GetSize()) Append(elem);
  else
    {
      MathMLElement* oldElem = content.Get(i);
      assert(oldElem != 0);
      if (oldElem != elem)
	{
	  elem->AddRef();
	  elem->SetParent(this);
	  oldElem->Release();
	  content.Set(i, elem);
	  SetDirtyStructure();
	}
    }
}

void
MathMLLinearContainerElement::Append(MathMLElement* elem)
{
  assert(elem != 0);
  elem->AddRef();
  elem->SetParent(this);
  content.Append(elem);
  SetDirtyStructure();
}

void
MathMLLinearContainerElement::Remove(MathMLElement* elem)
{
  assert(elem != 0);
  assert(content.Contains(elem));
  content.Remove(elem);
  elem->Release();
  SetDirtyStructure();
}

void
MathMLLinearContainerElement::Replace(MathMLElement* oldElem, MathMLElement* newElem)
{
  assert(oldElem != 0);
  assert(newElem != 0);
  assert(content.Contains(oldElem));
  SetChild(content.IndexOf(oldElem), newElem);
}

