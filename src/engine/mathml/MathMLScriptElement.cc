// Copyright (C) 2000-2003, Luca Padovani <luca.padovani@cs.unibo.it>.
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
// http://helm.cs.unibo.it/mml-widget, or send a mail to
// <luca.padovani@cs.unibo.it>

#include <config.h>

#include <cassert>

#include "ChildList.hh"
#include "MathMLDummyElement.hh"
#include "MathMLFormattingEngineFactory.hh"
#include "MathMLOperatorElement.hh"
#include "MathMLScriptElement.hh"
#include "MathMLView.hh"
#include "ValueConversion.hh"
#include "defs.h"
//#include "traverseAux.hh"
#include "MathFormattingContext.hh"
#include "MathGraphicDevice.hh"
#include "MathMLAttributeSignatures.hh"

MathMLScriptElement::MathMLScriptElement(const SmartPtr<class MathMLView>& view)
  : MathMLContainerElement(view)
{ }

MathMLScriptElement::~MathMLScriptElement()
{ }

void
MathMLScriptElement::construct()
{
  if (dirtyStructure())
    {
#if defined(HAVE_GMETADOM)
      if (getDOMElement())
	{
	  assert(IsA() == T_MSUB || IsA() == T_MSUP || IsA() == T_MSUBSUP);
	  ChildList children(getDOMElement(), MATHML_NS_URI, "*");
	  
	  if (SmartPtr<MathMLElement> e = getFormattingNode(children.item(0)))
	    setBase(e);
	  else if (!is_a<MathMLDummyElement>(getBase()))
	    setBase(getFactory()->createDummyElement(getView()));

	  switch (IsA())
	    {
	    case T_MSUB:
	      if (SmartPtr<MathMLElement> e = getFormattingNode(children.item(1)))
		setSubScript(e);
	      else if (!is_a<MathMLDummyElement>(getSubScript()))
		setSubScript(getFactory()->createDummyElement(getView()));
	      setSuperScript(0);
	      break;
	    case T_MSUP:
	      setSubScript(0);
	      if (SmartPtr<MathMLElement> e = getFormattingNode(children.item(1)))
		setSuperScript(e);
	      else if (!is_a<MathMLDummyElement>(getSuperScript()))
		setSuperScript(getFactory()->createDummyElement(getView()));
	      break;
	    case T_MSUBSUP:
	      if (SmartPtr<MathMLElement> e = getFormattingNode(children.item(1)))
		setSubScript(e);
	      else if (!is_a<MathMLDummyElement>(getSubScript()))
		setSubScript(getFactory()->createDummyElement(getView()));
	      if (SmartPtr<MathMLElement> e = getFormattingNode(children.item(2)))
		setSuperScript(e);
	      else if (!is_a<MathMLDummyElement>(getSuperScript()))
		setSuperScript(getFactory()->createDummyElement(getView()));
	      break;
	    default:
	      assert(false);
	    }
	}
#endif

      if (getBase()) getBase()->construct();
      if (getSubScript()) getSubScript()->construct();
      if (getSuperScript()) getSuperScript()->construct();

      resetDirtyStructure();
    }
}

void
MathMLScriptElement::refine(AbstractRefinementContext& context)
{
  if (dirtyAttribute() || dirtyAttributeP())
    {
      REFINE_ATTRIBUTE(context, MathML, Script, subscriptshift);
      REFINE_ATTRIBUTE(context, MathML, Script, superscriptshift);
      if (getBase()) getBase()->refine(context);
      if (getSubScript()) getSubScript()->refine(context);
      if (getSuperScript()) getSuperScript()->refine(context);
      MathMLContainerElement::refine(context);
    }
}

AreaRef
MathMLScriptElement::format(MathFormattingContext& ctxt)
{
  if (dirtyLayout())
    {
      ctxt.push(this);

      assert(getBase());
      AreaRef baseArea = getBase()->format(ctxt);

      ctxt.addScriptLevel(1);
      ctxt.setDisplayStyle(false);

      Length subScriptShift;
      AreaRef subScriptArea;
      if (getSubScript())
	{
	  subScriptArea = getSubScript()->format(ctxt);

	  if (SmartPtr<Value> value = GET_ATTRIBUTE_VALUE(MathML, Script, subscriptshift))
	    {
	      assert(IsLength(value));
	      subScriptShift = ToLength(value);
	    }
	}

      Length superScriptShift;
      AreaRef superScriptArea;
      if (getSuperScript())
	{
	  superScriptArea = getSuperScript()->format(ctxt);

	  if (SmartPtr<Value> value = GET_ATTRIBUTE_VALUE(MathML, Script, superscriptshift))
	    {
	      assert(IsLength(value));
	      superScriptShift = ToLength(value);
	    }
	}

      AreaRef res = ctxt.getDevice()->script(ctxt, baseArea,
					     subScriptArea, subScriptShift,
					     superScriptArea, superScriptShift);
      res = formatEmbellishment(this, ctxt, res);
      setArea(ctxt.getDevice()->wrapper(ctxt, res));

      ctxt.pop();
      resetDirtyLayout();
    }

  return getArea();
}

SmartPtr<class MathMLOperatorElement>
MathMLScriptElement::getCoreOperator()
{
  return getBase() ? getBase()->getCoreOperator() : 0;
}

void
MathMLScriptElement::setFlagDown(Flags f)
{
  MathMLContainerElement::setFlagDown(f);
  base.setFlagDown(f);
  subScript.setFlagDown(f);
  superScript.setFlagDown(f);
}

void
MathMLScriptElement::resetFlagDown(Flags f)
{
  MathMLContainerElement::resetFlagDown(f);
  base.resetFlagDown(f);
  subScript.resetFlagDown(f);
  superScript.resetFlagDown(f);
}
