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

#include "Layout.hh"
#include "MathMLCharNode.hh"
#include "RenderingEnvironment.hh"
#include "MathMLOperatorElement.hh"
#include "MathMLEmbellishedOperatorElement.hh"
#include "FormattingContext.hh"

MathMLEmbellishedOperatorElement::
MathMLEmbellishedOperatorElement(const Ptr<MathMLOperatorElement>& op)
  : coreOp(op)
{
  assert(coreOp != 0);
  script = false;
}

MathMLEmbellishedOperatorElement::~MathMLEmbellishedOperatorElement()
{
}

void
MathMLEmbellishedOperatorElement::Normalize()
{
  if (HasDirtyStructure() || HasChildWithDirtyStructure())
    {
      assert(child != 0);

      Ptr<MathMLElement> p = GetParent();
      assert(p != 0);

      Ptr<MathMLContainerElement> pContainer = smart_cast<MathMLContainerElement>(p);
      assert(pContainer != 0);
      pContainer->Replace(this, child);

      child->Normalize();

      ResetDirtyStructure();
    }
}

void
MathMLEmbellishedOperatorElement::Setup(RenderingEnvironment* env)
{
  assert(env != NULL);
  script = env->GetScriptLevel() > 0;
  MathMLBinContainerElement::Setup(env);
}

void
MathMLEmbellishedOperatorElement::DoLayout(const class FormattingContext& ctxt)
{
  if (!HasDirtyLayout()) return;

  assert(child != 0);
  assert(coreOp != 0);

  scaled totalPadding = script ? 0 : coreOp->GetLeftPadding() + coreOp->GetRightPadding();

  child->DoLayout(ctxt);
  box = child->GetBoundingBox();

  // WARNING: maybe in this case we should ask for the LAST char node...
  Ptr<const MathMLCharNode> node = coreOp->GetCharNode();
  if (node != NULL && isIntegral(node->GetChar())) {
    // WARNING
    // the following patch is needed in order to have integral sign working
    box.width = scaledMax(box.width, box.rBearing);
  }
  box.width += totalPadding;

#ifdef ENABLE_EXTENSIONS
  box.ascent += coreOp->GetTopPadding();
  box.tAscent += coreOp->GetTopPadding();
  box.descent += coreOp->GetBottomPadding();
  box.tDescent += coreOp->GetBottomPadding();
#endif // ENABLE_EXTENSIONS

  ResetDirtyLayout(ctxt.GetLayoutType());
}

void
MathMLEmbellishedOperatorElement::SetPosition(scaled x, scaled y)
{
  assert(coreOp != 0);
  assert(child != 0);

  position.x = x;
  position.y = y;

#ifdef ENABLE_EXTENSIONS
  child->SetPosition(x + (script ? 0 : coreOp->GetLeftPadding()), y + coreOp->GetTopPadding());
#else
  child->SetPosition(x + (script ? 0 : coreOp->GetLeftPadding()), y);
#endif // ENABLE_EXTENSIONS
}

bool
MathMLEmbellishedOperatorElement::IsEmbellishedOperator() const
{
  return true;
}

Ptr<MathMLCharNode>
MathMLEmbellishedOperatorElement::GetCharNode() const
{
  if (coreOp == 0 || child != Ptr<MathMLElement>(coreOp)) return 0;
  return coreOp->GetCharNode();
}
