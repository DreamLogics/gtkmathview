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

#include "unidefs.h"
#include "stringAux.hh"
#include "Globals.hh"
#include "Iterator.hh"
#include "ChildList.hh"
#include "StringUnicode.hh"
#include "EntitiesTable.hh"
#include "MathMLCharNode.hh"
#include "MathMLRowElement.hh"
#include "MathMLDummyElement.hh"
#include "MathMLRadicalElement.hh"
#include "RenderingEnvironment.hh"
#include "MathMLOperatorElement.hh"
#include "FormattingContext.hh"

MathMLRadicalElement::MathMLRadicalElement()
{
  radical = 0;
}

#if defined(HAVE_GMETADOM)
MathMLRadicalElement::MathMLRadicalElement(const GMetaDOM::Element& node)
  : MathMLLinearContainerElement(node)
{
  radical = 0;
}
#endif

MathMLRadicalElement::~MathMLRadicalElement()
{
}

void
MathMLRadicalElement::Normalize()
{
  if (HasDirtyStructure() || HasChildWithDirtyStructure())
    {
#if defined(HAVE_GMETADOM)
      if (GetDOMElement() != 0)
	{
	  if (IsA() == TAG_MSQRT)
	    // should do normalizing containter::Layout but now
	    // it's a bin, so radical cannot derive from it any more
	    MathMLNormalizingContainerElement::Normalize();
	  else
	    {
	      assert(IsA() == TAG_MROOT);	      
	      ChildList children(GetDOMElement(), MATHML_NS_URI, "*");
	      for (unsigned i = 0; i < 2; i++)
		if (i < children.get_length())
		  {
		    GMetaDOM::Node node = children.item(i);
		    assert(node.get_nodeType() == GMetaDOM::Node::ELEMENT_NODE);
		    Ptr<MathMLElement> elem = MathMLElement::getRenderingInterface(node);
		    assert(elem != 0);
		    SetChild(i, elem);
		  }
		else
		  {
		    Ptr<MathMLElement> mdummy = MathMLDummyElement::create();
		    assert(mdummy != 0);
		    SetChild(i, mdummy);
		  }
	    }
	}
#endif
      for (Iterator< Ptr<MathMLElement> > elem(content); elem.More(); elem.Next())
	{
	  assert(elem() != 0);
	  elem()->Normalize();
	}
  
      if (radical == 0) radical = MathMLCharNode::create(U_SQRT);
      assert(radical != 0);
      radical->SetParent(this);

      ResetDirtyStructure();
    }
}

void
MathMLRadicalElement::Setup(RenderingEnvironment* env)
{
  assert(env != NULL);

  spacing       = env->ToScaledPoints(env->GetMathSpace(MATH_SPACE_MEDIUM));
  color         = env->GetColor();
  background    = env->GetBackgroundColor();
  lineThickness = env->GetRuleThickness();

  Ptr<MathMLElement> base = content.GetFirst();
  assert(base != 0);
  base->Setup(env);

  assert(radical != 0);
  radical->Setup(env);

  if (IsA() == TAG_MROOT)
    {
      Ptr<MathMLElement> script = content.GetLast();
      assert(script != 0);
    
      env->Push();
      env->SetDisplayStyle(false);
      env->AddScriptLevel(2);
      script->Setup(env);
      env->Drop();
    }
}

void
MathMLRadicalElement::DoLayout(const class FormattingContext& ctxt)
{
  if (!HasDirtyLayout()) return;

  Ptr<MathMLElement> base = content.GetFirst();
  assert(base != 0);

  base->DoLayout(ctxt);
  box = base->GetBoundingBox();

  assert(radical != 0);
  radical->DoLayout(ctxt);
  radical->DoVerticalStretchyLayout(box.ascent + lineThickness, box.descent, 0, false);
  const BoundingBox& radBox = radical->GetBoundingBox();

  box.width += radBox.width;
  box.rBearing += radBox.width;
  box.ascent = scaledMax(box.ascent + spacing, radBox.ascent);
  box.tAscent = scaledMax(box.tAscent, box.ascent);
  box.descent = scaledMax(box.descent, radBox.descent);
  box.tDescent = scaledMax(box.tDescent, box.descent);

  if (IsA() == TAG_MROOT)
    {
      Ptr<MathMLElement> script = content.GetLast();
      assert(script != 0);

      script->DoLayout(ctxt);
      const BoundingBox& scriptBox = script->GetBoundingBox();

      box.width += scriptBox.width;

      if (box.GetHeight() / 2 < scriptBox.GetHeight())
	{
	  box.ascent += scriptBox.GetHeight() - box.GetHeight() / 2;
	  box.tAscent += scriptBox.descent + scriptBox.tAscent - box.GetHeight() / 2;
	}
    }

  ResetDirtyLayout(ctxt.GetLayoutType());
}

void
MathMLRadicalElement::SetPosition(scaled x, scaled y)
{
  position.x = x;
  position.y = y;

  Ptr<MathMLElement> base = content.GetFirst();
  assert(base != 0);

  assert(radical != 0);
  const BoundingBox& radBox = radical->GetBoundingBox();

  if (IsA() == TAG_MROOT)
    {
      Ptr<MathMLElement> script = content.GetLast();
      assert(script != 0);

      const BoundingBox& baseBox   = base->GetBoundingBox();
      const BoundingBox& scriptBox = script->GetBoundingBox();

      script->SetPosition(x, y + (baseBox.GetHeight() / 2 - baseBox.ascent) - scriptBox.descent);
      radical->SetPosition(x + scriptBox.width, y);
      base->SetPosition(x + scriptBox.width + radBox.width, y);
    } 
  else
    {
      radical->SetPosition(x, y - box.ascent + radBox.ascent);
      base->SetPosition(x + radBox.width, y);
    }
}

void
MathMLRadicalElement::Render(const DrawingArea& area)
{
  if (!HasDirtyChildren()) return;

  MathMLLinearContainerElement::Render(area);

  if (fGC[IsSelected()] == NULL) {
    GraphicsContextValues values;
    values.foreground = IsSelected() ? area.GetSelectionForeground() : color;
    values.background = IsSelected() ? area.GetSelectionBackground() : background;
    values.lineWidth = lineThickness;
    fGC[IsSelected()] = area.GetGC(values, GC_MASK_FOREGROUND | GC_MASK_BACKGROUND | GC_MASK_LINE_WIDTH);
  }

  assert(radical != 0);
  radical->SetDirty();
  radical->Render(area);

  Ptr<MathMLElement> base = content.GetFirst();
  assert(base != 0);

  const BoundingBox& radBox = radical->GetBoundingBox();

  area.MoveTo(radical->GetX() + radBox.width, radical->GetY() - radBox.ascent + lineThickness / 2);
  area.DrawLineToDelta(fGC[IsSelected()], base->GetBoundingBox().width, 0);

  ResetDirty();
}

bool
MathMLRadicalElement::IsExpanding() const
{
  Ptr<MathMLElement> base = content.GetFirst();
  assert(base != 0);
  return base->IsExpanding();
}

scaled
MathMLRadicalElement::GetLeftEdge() const
{
  assert(radical != 0);
  return scaledMin(MathMLLinearContainerElement::GetLeftEdge(), radical->GetLeftEdge());
}
