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
}

#if defined(HAVE_GMETADOM)
MathMLRowElement::MathMLRowElement(const GMetaDOM::Element& node)
  : MathMLLinearContainerElement(node)
{
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

  Iterator< Ptr<MathMLElement> > i(content);
  i.ResetLast();
  while (i.More() && i() != 0 &&
	 (i()->IsSpaceLike() || i()->IsEmbellishedOperator()))
    i.Prev();
	      
  if (i.More() && i() != NULL) lastElement = i();
}

void
MathMLRowElement::DoLayout(LayoutId id, scaled availWidth)
{
  if (!HasDirtyLayout()) return;

  box.Null();
  for (Iterator< Ptr<MathMLElement> > i(content); i.More(); i.Next())
    {
      Ptr<MathMLElement> elem = i();
      assert(elem != 0);
      elem->DoLayout(id, availWidth);
      box.Append(elem->GetBoundingBox());
    }

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

  for (Iterator< Ptr<MathMLElement> > elem(content); elem.More(); elem.Next())
    {
      assert(elem() != 0);
      BoundingBox elemBox;

      Ptr<MathMLOperatorElement> op = findStretchyOperator(elem(), STRETCH_VERTICAL);
      if (op != 0)
	{
	  opBox.Append(op->GetMinBoundingBox());
	  nStretchy++;      
	} 
      else
	{
	  rowBox.Append(elem()->GetBoundingBox());
	  nOther++;
	}
    }

  if (nStretchy > 0)
    {
      scaled toAscent  = (nOther == 0) ? opBox.ascent : rowBox.ascent;
      scaled toDescent = (nOther == 0) ? opBox.descent : rowBox.descent;

#if 0
      printf("%s(%p): found %d stretchy (%d other), now stretch to %d %d\n",
	     NameOfTagId(IsA()), this, nStretchy, nOther, sp2ipx(toAscent), sp2ipx(toDescent));
#endif

      for (Iterator< Ptr<MathMLElement> > elem(content); elem.More(); elem.Next())
	{
	  assert(elem() != 0);
	  Ptr<MathMLOperatorElement> op = findStretchyOperator(elem(), STRETCH_VERTICAL);

	  if (op != 0)
	    {
	      op->VerticalStretchTo(toAscent, toDescent);
	      elem()->DoLayout(LAYOUT_AUTO, 0);
	    }
	}
    }
}

bool
MathMLRowElement::IsSpaceLike() const
{
  for (Iterator< Ptr<MathMLElement> > elem(content); elem.More(); elem.Next())
    {
      assert(elem() != 0);
      if (!elem()->IsSpaceLike()) return false;
    }

  return true;
}

bool
MathMLRowElement::IsExpanding() const
{
  for (Iterator< Ptr<MathMLElement> > elem(content); elem.More(); elem.Next())
    {
      assert(elem() != 0);
      if (elem()->IsExpanding()) return true;
    }

  return false;
}

OperatorFormId
MathMLRowElement::GetOperatorForm(const Ptr<MathMLElement>& eOp) const
{
  assert(eOp != 0);

  OperatorFormId res = OP_FORM_INFIX;

  unsigned rowLength = 0;
  unsigned position  = 0;
  for (Iterator< Ptr<MathMLElement> > i(content); i.More(); i.Next())
    {
      Ptr<const MathMLElement> p = i();
      assert(p != 0);

      if (!p->IsSpaceLike())
	{
	  if (p == Ptr<const MathMLElement>(eOp)) position = rowLength;
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

Ptr<class MathMLOperatorElement>
MathMLRowElement::GetCoreOperator()
{
  Ptr<MathMLElement> core = 0;

  for (Iterator< Ptr<MathMLElement> > i(content); i.More(); i.Next())
    {
      Ptr<MathMLElement> elem = i();
      assert(elem != 0);

      if (!elem->IsSpaceLike())
	{
	  if (core == 0) core = elem;
	  else return 0;
	}
    }

  return (core != 0) ? core->GetCoreOperator() : 0;
}
