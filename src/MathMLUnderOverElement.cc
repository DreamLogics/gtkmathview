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
#include "operatorAux.hh"
#include "traverseAux.hh"
#include "MathMLCharNode.hh"
#include "MathMLDummyElement.hh"
#include "RenderingEnvironment.hh"
#include "MathMLUnderOverElement.hh"
#include "MathMLOperatorElement.hh"
#include "MathMLEmbellishedOperatorElement.hh"

MathMLUnderOverElement::MathMLUnderOverElement()
{
  underScript = overScript = 0;
}

#if defined(HAVE_GMETADOM)
MathMLUnderOverElement::MathMLUnderOverElement(const GMetaDOM::Element& node)
  : MathMLLinearContainerElement(node)
{
}
#endif

MathMLUnderOverElement::~MathMLUnderOverElement()
{
}

const AttributeSignature*
MathMLUnderOverElement::GetAttributeSignature(AttributeId id) const
{
  static AttributeSignature underSig[] = {
    { ATTR_ACCENTUNDER, booleanParser, NULL, NULL },
    { ATTR_NOTVALID,    NULL,          NULL, NULL }
  };

  static AttributeSignature overSig[] = {
    { ATTR_ACCENT,      booleanParser, NULL, NULL },
    { ATTR_NOTVALID,    NULL,          NULL, NULL }
  };

  const AttributeSignature* signature = NULL;
  if (IsA() == TAG_MUNDER || IsA() == TAG_MUNDEROVER)
    signature = GetAttributeSignatureAux(id, underSig);
  if (signature == NULL && (IsA() == TAG_MOVER || IsA() == TAG_MUNDEROVER))
    signature = GetAttributeSignatureAux(id, overSig);    
  if (signature == NULL)
    signature = MathMLLinearContainerElement::GetAttributeSignature(id);
  
  return signature;
}

void
MathMLUnderOverElement::Normalize()
{
  unsigned n = (IsA() == TAG_MUNDEROVER) ? 3 : 2;

  while (content.GetSize() > n)
    content.RemoveLast();

  while (content.GetSize() < n)
    {
      Ptr<MathMLElement> mdummy = MathMLDummyElement::create();
      assert(mdummy != 0);
      Append(mdummy);
    }

  // BEWARE: normalization has to be done here, since it may
  // change the content!!!
  MathMLLinearContainerElement::Normalize();

  base = content.GetFirst();
  assert(base != 0);

  if (IsA() == TAG_MUNDER) underScript = content.GetLast();
  else if (IsA() == TAG_MOVER) overScript = content.GetLast();
  else
    {
      underScript = content.Get(1);
      overScript  = content.Get(2);
    }
}

void
MathMLUnderOverElement::Setup(RenderingEnvironment* env)
{
  assert(env != NULL);
  assert(base != NULL);

  bool displayStyle = env->GetDisplayStyle();

  ScriptSetup(env);

  scaled smallSpacing = ruleThickness;
  scaled bigSpacing   = 3 * ruleThickness;

  base->Setup(env);
  Ptr<MathMLOperatorElement> op = base->GetCoreOperator();

  if (op != 0)
    scriptize = !displayStyle && op->HasMovableLimits();
  else
    scriptize = false;

  env->Push();
  env->SetDisplayStyle(false);

  accentUnder = false;
  underSpacing = 0;
  if (underScript != 0)
    {
      if (!scriptize)
	{
	  const Value* value = GetAttributeValue(ATTR_ACCENTUNDER, env, false);
	  if (value != NULL) accentUnder = value->ToBoolean();
	  else
	    {
	      Ptr<MathMLOperatorElement> op = underScript->GetCoreOperator();
	      if (op != 0)
		{
		  underScript->Setup(env);
		  accentUnder = op->IsAccent();
		}
	    }
	}

      if (accentUnder) underSpacing = smallSpacing;
      else
	{
	  env->AddScriptLevel(1);
	  underSpacing = displayStyle ? bigSpacing : smallSpacing;
	}
      underScript->Setup(env);
    }

  env->Drop();
  env->Push();
  env->SetDisplayStyle(false);

  accent = false;
  overSpacing = 0;
  if (overScript != 0)
    {
      if (!scriptize)
	{
	  const Value* value = GetAttributeValue(ATTR_ACCENT, env, false);
	  if (value != NULL) accent = value->ToBoolean();
	  else
	    {
	      Ptr<MathMLOperatorElement> op = overScript->GetCoreOperator();
	      if (op != 0)
		{
		  overScript->Setup(env);
		  accent = op->IsAccent();
		}
	    }
	}

      if (accent)
	{
	  overSpacing = smallSpacing;
	} 
      else
	{
	  env->AddScriptLevel(1);
	  overSpacing = displayStyle ? bigSpacing : smallSpacing;
	}
      overScript->Setup(env);
    }
  
  env->Drop();
}

void
MathMLUnderOverElement::DoLayout(LayoutId id, scaled maxWidth)
{
  if (!HasDirtyLayout()) return;

  assert(base != NULL);

  scaled overClearance = 0;
  scaled underClearance = 0;

  if (scriptize)
    {
      base->DoLayout(id, maxWidth / 2);
      if (overScript != NULL) overScript->DoLayout(id, maxWidth / 2);
      if (underScript != NULL) underScript->DoLayout(id, maxWidth / 2);

      const BoundingBox& baseBox = base->GetBoundingBox();
      BoundingBox underBox;
      BoundingBox overBox;
    
      if (underScript != NULL) underBox = underScript->GetBoundingBox();
      else underBox.Null();

      if (overScript != NULL) overBox = overScript->GetBoundingBox();
      else overBox.Null();

      DoScriptLayout(baseBox, underBox, overBox, underShiftX, underShiftY, overShiftX, overShiftY);
      underClearance = overClearance = 0;

      baseShiftX = 0;
    } 
  else
    {    
      if (id != LAYOUT_AUTO)
	{
	  base->DoLayout(id);
	  if (underScript != 0) underScript->DoLayout(id);
	  if (overScript != 0) overScript->DoLayout(id);
	} 
      else
	{
	  unsigned nOp    = 0;
	  unsigned nOther = 0;

	  scaled wOp      = 0;
	  scaled wOther   = 0;

	  Ptr<MathMLOperatorElement> baseOp  = findStretchyOperator(base, STRETCH_HORIZONTAL);
	  Ptr<MathMLOperatorElement> underOp = findStretchyOperator(underScript, STRETCH_HORIZONTAL);
	  Ptr<MathMLOperatorElement> overOp  = findStretchyOperator(overScript, STRETCH_HORIZONTAL);

	  Globals::logger(LOG_DEBUG, "stretchy: %p %p %p", baseOp, underOp, overOp);

	  if (baseOp == 0) base->DoLayout(id, maxWidth);
	  if (underScript != 0 && underOp == 0) underScript->DoLayout(id, maxWidth);
	  if (overScript != 0 && overOp == 0) overScript->DoLayout(id, maxWidth);

	  if (baseOp == 0)
	    {
	      wOther = base->GetBoundingBox().width;
	      nOther++;
	    } 
	  else
	    {
	      wOp = baseOp->GetMinBoundingBox().width;
	      nOp++;
	    }

	  if (underScript != 0)
	    {
	      if (underOp == 0)
		{
		  wOther = scaledMax(wOther, underScript->GetBoundingBox().width);
		  nOther++;
		} 
	      else
		{
		  wOp = scaledMax(wOp, underOp->GetMinBoundingBox().width);
		  nOp++;
		}
	    }

	  if (overScript != 0)
	    {
	      if (overOp == 0)
		{
		  wOther = scaledMax(wOther, overScript->GetBoundingBox().width);
		  nOther++;
		} 
	      else
		{
		  wOp = scaledMax(wOp, overOp->GetMinBoundingBox().width);
		  nOp++;
		}
	    }

	  if (nOp > 0) {
	    scaled w = (nOther == 0) ? wOp : wOther;

	    if (baseOp != 0) baseOp->HorizontalStretchTo(w);
	    if (underOp != 0) underOp->HorizontalStretchTo(w);
	    if (overOp != 0) overOp->HorizontalStretchTo(w);
	  }

	  if (baseOp != 0) base->DoLayout(id);
	  if (underScript != 0 && underOp != 0) underScript->DoLayout(id);
	  if (overScript != 0 && overOp != 0) overScript->DoLayout(id);
	}

      const BoundingBox& baseBox = base->GetBoundingBox();
      Ptr<const MathMLCharNode> bChar = base->GetCharNode();

      if (underScript != 0)
	{
	  Ptr<MathMLCharNode> cChar = underScript->GetCharNode();

	  if (accentUnder &&
	      bChar != 0 && cChar != 0 &&
	      isCombiningBelow(cChar->GetChar()) &&
	      bChar->CombineWith(cChar, underShiftX, underShiftY))
	    {
	      Globals::logger(LOG_DEBUG, "this is the special handling for U+%04X used as accent under U+%04X",
			      cChar->GetChar(), bChar->GetChar());

	      underShiftY = -underShiftY;

#if defined(ENABLE_EXTENSIONS)
	      if (underScript->IsEmbellishedOperator())
		{
		  Ptr<MathMLEmbellishedOperatorElement> eOp =
		    smart_cast<MathMLEmbellishedOperatorElement>(underScript);
		  assert(eOp != 0);
		  Ptr<MathMLOperatorElement> coreOp = eOp->GetCoreOperator();
		  assert(coreOp != 0);
		  underShiftY += coreOp->GetTopPadding();
		}
#endif
	    } 
	  else
	    {
	      const BoundingBox& scriptBox = underScript->GetBoundingBox();

	      underShiftX = (baseBox.width - scriptBox.width) / 2;
	      underShiftY = baseBox.descent + underSpacing + scriptBox.ascent;
	      underClearance = ruleThickness;
	    }
	}

      if (overScript != 0)
	{
	  Ptr<MathMLCharNode> cChar = overScript->GetCharNode();

	  if (accent &&
	      bChar != 0 && cChar != 0 &&
	      isCombiningAbove(cChar->GetChar()) &&
	      bChar->CombineWith(cChar, overShiftX, overShiftY))
	    {
	      Globals::logger(LOG_DEBUG, "this is the special handling for U+%04X used as accent over U+%04X",
			      cChar->GetChar(), bChar->GetChar());

#if defined(ENABLE_EXTENSIONS)
	      if (overScript->IsEmbellishedOperator())
		{
		  Ptr<MathMLEmbellishedOperatorElement> eOp =
		    smart_cast<MathMLEmbellishedOperatorElement>(overScript);
		  assert(eOp != 0);
		  Ptr<MathMLOperatorElement> coreOp = eOp->GetCoreOperator();
		  assert(coreOp != 0);
		  Globals::logger(LOG_DEBUG, "the accent will get en extra spacing of %d", sp2ipx(coreOp->GetBottomPadding()));
		  overShiftY += coreOp->GetBottomPadding();
		}
#endif
	    } 
	  else
	    {
	      const BoundingBox& scriptBox = overScript->GetBoundingBox();

	      overShiftX = (baseBox.width - scriptBox.width) / 2 + scaledMax(0, baseBox.rBearing - baseBox.width);
	      overShiftY = baseBox.ascent + overSpacing + scriptBox.descent;
	      overClearance = ruleThickness;
	    }
	}

      baseShiftX = scaledMax(0, - scaledMin(overShiftX, underShiftX));
    }

  overShiftX += baseShiftX;
  underShiftX += baseShiftX;

  box = base->GetBoundingBox();
  box.width += baseShiftX;
  box.lBearing += baseShiftX;

  if (underScript != 0)
    {
      const BoundingBox& scriptBox = underScript->GetBoundingBox();

      box.width = scaledMax(box.width, underShiftX + scriptBox.width);
      box.rBearing = scaledMax(box.rBearing, underShiftX + scriptBox.rBearing);
      box.lBearing = scaledMin(box.lBearing, underShiftX + scriptBox.lBearing);
      box.ascent   = scaledMax(box.ascent, scriptBox.ascent - underShiftY);
      box.tAscent  = scaledMax(box.tAscent, scriptBox.tAscent - underShiftY);
      box.descent  = scaledMax(box.descent, scriptBox.descent + underShiftY);
      box.tDescent = scaledMax(box.tDescent, scriptBox.tDescent + underShiftY);
      box.descent += underClearance;
    }

  if (overScript != 0)
    {
      const BoundingBox& scriptBox = overScript->GetBoundingBox();

      box.width = scaledMax(box.width, overShiftX + scriptBox.width);
      box.rBearing = scaledMax(box.rBearing, overShiftX + scriptBox.rBearing);
      box.lBearing = scaledMin(box.lBearing, overShiftX + scriptBox.lBearing);
      box.ascent   = scaledMax(box.ascent, scriptBox.ascent + overShiftY);
      box.tAscent  = scaledMax(box.tAscent, scriptBox.tAscent + overShiftY);
      box.descent  = scaledMax(box.descent, scriptBox.descent - overShiftY);
      box.tDescent = scaledMax(box.tDescent, scriptBox.tDescent - overShiftY);
      box.ascent += overClearance;
    }
  
  ResetDirtyLayout(id);
}

void
MathMLUnderOverElement::SetPosition(scaled x, scaled y)
{
  assert(base != 0);

  position.x = x;
  position.y = y;

  base->SetPosition(x + baseShiftX, y);

  if (underScript != 0)
    underScript->SetPosition(x + underShiftX, y + underShiftY);

  if (overScript != 0)
    overScript->SetPosition(x + overShiftX, y - overShiftY);
}

bool
MathMLUnderOverElement::IsExpanding() const
{
  assert(base != 0);
  return base->IsExpanding();
}

Ptr<class MathMLOperatorElement>
MathMLUnderOverElement::GetCoreOperator()
{
  assert(base != 0);
  return base->GetCoreOperator();
}
