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
#include "CharMap.hh"
#include "Iterator.hh"
#include "operatorAux.hh"
#include "traverseAux.hh"
#include "MathMLRowElement.hh"
#include "MathMLSpaceElement.hh"
#include "RenderingEnvironment.hh"
#include "MathMLOperatorElement.hh"
#include "MathMLEmbellishedOperatorElement.hh"

MathMLRowElement::MathMLRowElement()
{
  lastElement = NULL;
}

#if defined(HAVE_GMETADOM)
MathMLRowElement::MathMLRowElement(const GMetaDOM::Element& node)
  : MathMLLinearContainerElement(node)
{
  lastElement = NULL;
}
#endif

MathMLRowElement::~MathMLRowElement()
{
}

#if 0
void
MathMLRowElement::Normalize()
{
}
#endif

void
MathMLRowElement::Setup(RenderingEnvironment* env)
{
  MathMLLinearContainerElement::Setup(env);

  Iterator<MathMLElement*> i(content);
  i.ResetLast();
  while (i.More() && i() != NULL &&
	 (i()->IsSpaceLike() || i()->IsEmbellishedOperator())) {
    i.Prev();
  }
	      
  if (i.More() && i() != NULL) lastElement = i();
}

void
MathMLRowElement::DoLayout(LayoutId id, Layout& layout)
{
  enum { STATE_A, STATE_B, STATE_C, STATE_D, STATE_E } state = STATE_A;

  layout.In();

  BreakId lastBreakability = BREAK_AUTO;

  for (Iterator<MathMLElement*> i(content); i.More(); i.Next()) {
    MathMLElement* elem = i();
    assert(elem != NULL);

    BreakId bid = BREAK_NO;

    if (elem == lastElement) {
      // if elem is the last element but it preceeded by a separator,
      // then we have still to set the right breakability after that
      // separator, which otherwise would get a BREAK_NO.
      if (state == STATE_C && !elem->IsEmbellishedOperator() && !elem->IsSpaceLike()) bid = BREAK_BAD;
      state = STATE_E;
    } else {
      switch (state) {
      case STATE_A:
	if (!elem->IsEmbellishedOperator() && !elem->IsSpaceLike()) state = STATE_B;
	break;
      case STATE_B:
	if (elem->IsEmbellishedOperator()) {
	  MathMLOperatorElement* op = elem->GetCoreOperator();
	  assert(op != NULL);
	  // we cannot allow the expression to be broken
	  // before or after the operator if it is non-marking
	  // (i.e. it is only made of non-marking characters)
	  if (!op->IsNonMarking()) {
	    if (op->IsSeparator()) state = STATE_C;
	    else {
	      bid = BREAK_BAD;
	      state = STATE_D;
	    }
	  }
	  op->Release();
	}
	break;
      case STATE_C:
	if (elem->IsSpaceLike()) {
	  bid = BREAK_BAD;
	  state = STATE_D;
	} else if (elem->IsEmbellishedOperator()) {
	  MathMLOperatorElement* op = elem->GetCoreOperator();
	  assert(op != NULL);
	  if (!op->IsSeparator()) {
	    bid = BREAK_BAD;
	    state = STATE_D;
	    op->Release();
	  }
	} else {
	  bid = BREAK_BAD;
	  state = STATE_B;
	}
	break;
      case STATE_D:
	if (!elem->IsSpaceLike() && !elem->IsEmbellishedOperator()) state = STATE_B;
	break;
      case STATE_E:
	break;
      }
    }

    if (lastBreakability != BREAK_AUTO) bid = lastBreakability;

    lastBreakability = elem->GetBreakability();
    layout.SetLastBreakability(bid);

    if (elem->IsBreakable()) elem->DoLayout(id, layout);
    else layout.Append(elem, 0);
  }

  layout.Out();

  ResetDirtyLayout(id);
}

void
MathMLRowElement::DoStretchyLayout()
{
  MathMLLinearContainerElement::DoStretchyLayout();

  unsigned nStretchy = 0; // # of stretchy operators in this line
  unsigned nOther    = 0; // # of non-stretchy elements in this line

  BoundingBox rowBox;
  BoundingBox opBox;
  rowBox.Null();
  opBox.Null();

  for (Iterator<MathMLElement*> elem(content); elem.More(); elem.Next()) {
    assert(elem() != NULL);
    BoundingBox elemLinearBox;

    MathMLOperatorElement* op = findStretchyOperator(elem(), STRETCH_VERTICAL);
    if (op != NULL) {
      opBox.Append(op->GetMinBoundingBox());
      nStretchy++;      
    } else {
      elem()->GetLinearBoundingBox(elemLinearBox);
      rowBox.Append(elemLinearBox);
      nOther++;
    }
  }

  if (nStretchy > 0) {
    scaled toAscent  = (nOther == 0) ? opBox.ascent : rowBox.ascent;
    scaled toDescent = (nOther == 0) ? opBox.descent : rowBox.descent;

#if 0
    printf("%s(%p): found %d stretchy (%d other), now stretch to %d %d\n",
	   NameOfTagId(IsA()), this, nStretchy, nOther, sp2ipx(toAscent), sp2ipx(toDescent));
#endif

    for (Iterator<MathMLElement*> elem(content); elem.More(); elem.Next()) {
      assert(elem() != NULL);
      MathMLOperatorElement* op = findStretchyOperator(elem(), STRETCH_VERTICAL);

      if (op != NULL) {
	op->VerticalStretchTo(toAscent, toDescent);
	elem()->DoBoxedLayout(LAYOUT_AUTO, BREAK_NO, elem()->GetMaxBoundingBox().width);
      }
    }
  }
}

bool
MathMLRowElement::IsSpaceLike() const
{
  for (Iterator<MathMLElement*> elem(content); elem.More(); elem.Next()) {
    assert(elem() != NULL);
    if (!elem()->IsSpaceLike()) return false;
  }

  return true;
}

bool
MathMLRowElement::IsBreakable() const
{
  return true;
}

bool
MathMLRowElement::IsExpanding() const
{
  for (Iterator<MathMLElement*> elem(content); elem.More(); elem.Next()) {
    assert(elem() != NULL);
    if (elem()->IsExpanding()) return true;
  }

  return false;
}

OperatorFormId
MathMLRowElement::GetOperatorForm(MathMLElement* eOp) const
{
  assert(eOp != 0);

  OperatorFormId res = OP_FORM_INFIX;

  unsigned rowLength = 0;
  unsigned position  = 0;
  for (Iterator<MathMLElement*> i(content); i.More(); i.Next())
    {
      const MathMLElement* p = i();
      assert(p != NULL);

      if (!p->IsSpaceLike())
	{
	  if (p == eOp) position = rowLength;
	  rowLength++;
	}
    }
    
  if (rowLength > 1) 
    {
      if (position == 0) res = OP_FORM_PREFIX;
      else if (position == rowLength - 1) res = OP_FORM_POSTFIX;
    }

  return res;
}

class MathMLOperatorElement*
MathMLRowElement::GetCoreOperator()
{
  MathMLElement* core = NULL;

  for (Iterator<MathMLElement*> i(content); i.More(); i.Next())
    {
      MathMLElement* elem = i();
      assert(elem != NULL);

      if (!elem->IsSpaceLike()) {
	if (core == NULL) core = elem;
	else return NULL;
      }
    }

  return (core != NULL) ? core->GetCoreOperator() : NULL;
}
