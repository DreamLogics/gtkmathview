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

#include <functional>
#include <algorithm>

#include <assert.h>
#include <stddef.h>

#include "Layout.hh"
#include "CharMap.hh"
#include "operatorAux.hh"
#include "traverseAux.hh"
#include "MathMLRowElement.hh"
#include "MathMLSpaceElement.hh"
#include "RenderingEnvironment.hh"
#include "MathMLOperatorElement.hh"
#include "MathMLEmbellishedOperatorElement.hh"
#include "FormattingContext.hh"

/////// START OF ADAPTORS ///////

struct IsSpaceLikePredicate
  : public std::unary_function<Ptr<MathMLElement>,bool>
{
  bool operator()(const Ptr<MathMLElement>& elem) const
  { return elem->IsSpaceLike(); }
};

/////// END OF ADAPTORS ///////

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

void
MathMLRowElement::DoLayout(const class FormattingContext& ctxt)
{
  if (!HasDirtyLayout()) return;

  box.Null();
  for (std::vector< Ptr<MathMLElement> >::iterator elem = content.begin();
       elem != content.end();
       elem++)
    {
      (*elem)->DoLayout(ctxt);
      box.Append((*elem)->GetBoundingBox());
    }

  DoStretchyLayout();

  ResetDirtyLayout(ctxt.GetLayoutType());
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

  for (std::vector< Ptr<MathMLElement> >::iterator elem = content.begin();
       elem != content.end();
       elem++)
    {
      Ptr<MathMLOperatorElement> op = findStretchyOperator(*elem, STRETCH_VERTICAL);
      if (op)
	{
	  opBox.Append(op->GetMinBoundingBox());
	  nStretchy++;      
	} 
      else
	{
	  rowBox.Append((*elem)->GetBoundingBox());
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

      for (std::vector< Ptr<MathMLElement> >::iterator elem = content.begin();
	   elem != content.end();
	   elem++)
	{
	  Ptr<MathMLOperatorElement> op = findStretchyOperator(*elem, STRETCH_VERTICAL);

	  if (op)
	    {
	      op->VerticalStretchTo(toAscent, toDescent);
	      (*elem)->DoLayout(FormattingContext(LAYOUT_AUTO, 0));
	    }
	}
    }
}

void
MathMLRowElement::SetPosition(scaled x, scaled y)
{
  MathMLLinearContainerElement::SetPosition(x, y);
  for (std::vector< Ptr<MathMLElement> >::iterator elem = content.begin();
       elem != content.end();
       elem++)
    {
      (*elem)->SetPosition(x, y);
      x += (*elem)->GetBoundingBox().width;
    }
}

bool
MathMLRowElement::IsSpaceLike() const
{
  return std::find_if(content.begin(), content.end(),
		      std::not1(IsSpaceLikePredicate())) != content.end();
}

scaled
MathMLRowElement::GetLeftEdge() const
{
  scaled edge = 0;
  
  for (std::vector< const Ptr<MathMLElement> >::iterator elem = content.begin();
       elem != content.end();
       elem++)
    {
      if (elem == content.begin()) edge = (*elem)->GetLeftEdge();
      else edge = scaledMin(edge, (*elem)->GetX() + (*elem)->GetLeftEdge());
    }

  return edge;
}

scaled
MathMLRowElement::GetRightEdge() const
{
  scaled edge = 0;

  for (std::vector< const Ptr<MathMLElement> >::iterator elem = content.begin();
       elem != content.end();
       elem++)
    {
      if (elem == content.begin()) edge = (*elem)->GetRightEdge();
      else edge = scaledMax(edge, (*elem)->GetX() + (*elem)->GetRightEdge());
    }

  return edge;
}

OperatorFormId
MathMLRowElement::GetOperatorForm(const Ptr<MathMLElement>& eOp) const
{
  assert(eOp);

  OperatorFormId res = OP_FORM_INFIX;

  unsigned rowLength = 0;
  unsigned position  = 0;
  for (std::vector< const Ptr<MathMLElement> >::iterator elem = content.begin();
       elem != content.end();
       elem++)
    {
      Ptr<const MathMLElement> p = *elem;

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

  for (std::vector< Ptr<MathMLElement> >::iterator elem = content.begin();
       elem != content.end();
       elem++)
    {
      if (!(*elem)->IsSpaceLike())
	{
	  if (!core) core = *elem;
	  else return 0;
	}
    }

  return core ? core->GetCoreOperator() : Ptr<class MathMLOperatorElement>(0);
}
