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

#include "Globals.hh"
#include "ShapeFactory.hh"
#include "StringUnicode.hh"
#include "AttributeParser.hh"
#include "MathMLActionElement.hh"

MathMLActionElement::MathMLActionElement(void)
{
  selection = 0;
}

#if defined(HAVE_GMETADOM)
  MathMLActionElement::MathMLActionElement(const GMetaDOM::Element& node)
  : MathMLLinearContainerElement(node)
{
  selection = 0;
}
#endif

MathMLActionElement::~MathMLActionElement()
{
}

const AttributeSignature*
MathMLActionElement::GetAttributeSignature(AttributeId id) const
{
  static AttributeSignature sig[] = {
    { ATTR_ACTIONTYPE, NULL,          NULL,             NULL },
    { ATTR_SELECTION,  integerParser, new StringC("1"), NULL },
    { ATTR_NOTVALID,   NULL,          NULL,             NULL }
  };

  const AttributeSignature* signature = GetAttributeSignatureAux(id, sig);
  if (signature == NULL) signature = MathMLContainerElement::GetAttributeSignature(id);

  return signature;
}

void
MathMLActionElement::Setup(RenderingEnvironment* env)
{
  assert(env != NULL);

  const String* sValue = GetAttribute(ATTR_ACTIONTYPE, env, false);
  if (sValue != NULL) {
    if (!sValue->Equal("toggle"))
      Globals::logger(LOG_WARNING, "action `%s' is not supported (ignored)", sValue->ToStaticC());
  } else
    Globals::logger(LOG_WARNING, "no action specified for `maction' element");

  const Value* value = GetAttributeValue(ATTR_SELECTION, env);
  if (value != NULL) {
    selection = value->ToInteger() - 1;
    if (selection >= content.GetSize()) selection = content.GetSize() - 1;
  }

  MathMLLinearContainerElement::Setup(env);
}

void
MathMLActionElement::DoBoxedLayout(LayoutId id, BreakId bid, scaled availWidth)
{
  if (!HasDirtyLayout(id, availWidth)) return;

  Ptr<MathMLElement> elem = GetSelectedElement();

  if (elem != 0) {
    elem->DoBoxedLayout(id, bid, availWidth);
    box = elem->GetBoundingBox();
  } else
    box.Null();

  ConfirmLayout(id);

  ResetDirtyLayout(id, availWidth);
}

void
MathMLActionElement::DoLayout(LayoutId id, Layout& layout)
{
  Ptr<MathMLElement> elem = GetSelectedElement();
  if (elem != 0) elem->DoLayout(id, layout);
  ResetDirtyLayout(id);
}

void
MathMLActionElement::DoStretchyLayout()
{
  Ptr<MathMLElement> elem = GetSelectedElement();
  if (elem != 0) elem->DoStretchyLayout();
}

void
MathMLActionElement::SetPosition(scaled x, scaled y)
{
  position.x = x;
  position.y = y;

  Ptr<MathMLElement> elem = GetSelectedElement();
  if (elem != 0) elem->SetPosition(x, y);
}

void
MathMLActionElement::Freeze()
{
  Ptr<MathMLElement> elem = GetSelectedElement();
  assert(elem != 0);

  elem->Freeze();

  if (!IsBreakable() || HasLayout()) MathMLElement::Freeze();
  else {
    if (shape != NULL) delete shape;
    ShapeFactory shapeFactory;
    shapeFactory.Add(elem->GetShape());
    if (elem->IsLast()) shapeFactory.SetNewRow();
    shape = shapeFactory.GetShape();
  }
}

void
MathMLActionElement::Render(const DrawingArea& area)
{
  if (!HasDirtyChildren()) return;

  Ptr<MathMLElement> elem = GetSelectedElement();
  if (elem != 0) elem->Render(area);

  ResetDirty();
}

void
MathMLActionElement::SetDirty(const Rectangle* rect)
{
  Ptr<MathMLElement> elem = GetSelectedElement();
  if (elem != 0) {
    elem->SetDirty(rect);
    // dirty-children has to be called explicitly because if the child is already
    // dirty, then it does not invoke SetDirtyChildren by itself
    // (see MathMLFrame.hh)
    SetDirtyChildren();
    //dirty = elem->IsDirty();
  }
}

bool
MathMLActionElement::IsBreakable() const
{
  Ptr<MathMLElement> elem = GetSelectedElement();
  return (elem != 0) ? elem->IsBreakable() : false;
}

bool
MathMLActionElement::IsExpanding() const
{
  Ptr<MathMLElement> elem = GetSelectedElement();
  return (elem != 0) ? elem->IsExpanding() : false;
}

bool
MathMLActionElement::IsLast() const
{
  if (last != 0) return true;
  Ptr<MathMLElement> elem = GetSelectedElement();
  return (elem != 0) ? elem->IsLast() : false;
}

Ptr<MathMLElement>
MathMLActionElement::GetSelectedElement() const
{
  return (selection < content.GetSize()) ? content.Get(selection) : 0;
}

void
MathMLActionElement::SetSelectedIndex(unsigned i)
{
  assert(i > 0 && i <= content.GetSize());
  if (selection == i - 1) return;
  selection = i - 1;

  Ptr<MathMLElement> elem = GetSelectedElement();
  if (elem != 0) elem->SetDirtyLayout(true);
}

unsigned
MathMLActionElement::GetSelectedIndex() const
{
  return (content.GetSize() > 0) ? selection + 1 : 0;
}

BreakId
MathMLActionElement::GetBreakability() const
{
  Ptr<MathMLElement> elem = GetSelectedElement();
  return (elem != 0) ? elem->GetBreakability() : BREAK_AUTO;
}

scaled
MathMLActionElement::GetLeftEdge() const
{
  Ptr<MathMLElement> elem = GetSelectedElement();
  return (elem != 0) ? elem->GetLeftEdge() : GetX();
}

scaled
MathMLActionElement::GetRightEdge() const
{
  Ptr<MathMLElement> elem = GetSelectedElement();
  return (elem != 0) ? elem->GetRightEdge() : GetX();
}

Ptr<MathMLElement>
MathMLActionElement::Inside(scaled x, scaled y)
{
  if (!IsInside(x, y)) return 0;

  Ptr<MathMLElement> elem = GetSelectedElement();
  return (elem != 0) ? elem->Inside(x, y) : this;
}

void
MathMLActionElement::SetSelected()
{
  if (IsSelected()) return;

  selected = 1;

  Ptr<MathMLElement> elem = GetSelectedElement();
  if (elem != 0) elem->SetSelected();

  SetDirty();
}

void
MathMLActionElement::ResetSelected()
{
  if (!IsSelected()) return;

  SetDirty();

  Ptr<MathMLElement> elem = GetSelectedElement();
  if (elem != 0) elem->ResetSelected();

  selected = 0;
}

void
MathMLActionElement::ResetLast()
{
  last = 0;
  Ptr<MathMLElement> elem = GetSelectedElement();
  if (elem != 0) elem->ResetLast();
}

void
MathMLActionElement::SetDirtyLayout(bool children)
{
  MathMLElement::SetDirtyLayout(children);
  if (children) {
    Ptr<MathMLElement> elem = GetSelectedElement();
    if (elem != 0) elem->SetDirtyLayout(children);
  }
}
