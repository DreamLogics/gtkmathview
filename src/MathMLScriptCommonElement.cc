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

#include "RenderingEnvironment.hh"
#include "MathMLScriptCommonElement.hh"

MathMLScriptCommonElement::MathMLScriptCommonElement(mDOMNodeRef node, TagId id) :
  MathMLContainerElement(node, id)
{
  assert(id == TAG_MSUP || id == TAG_MSUB || id == TAG_MSUBSUP || id == TAG_MMULTISCRIPTS);
  base = NULL;
}

MathMLScriptCommonElement::~MathMLScriptCommonElement()
{
}

const AttributeSignature*
MathMLScriptCommonElement::GetAttributeSignature(AttributeId id) const
{
  static AttributeSignature subSig[] = {
    { ATTR_SUBSCRIPTSHIFT, numberUnitParser, NULL, NULL },
    { ATTR_NOTVALID,       NULL,             NULL, NULL }
  };

  static AttributeSignature supSig[] = {
    { ATTR_SUPERSCRIPTSHIFT, numberUnitParser, NULL, NULL },
    { ATTR_NOTVALID,         NULL,             NULL, NULL }
  };

  const AttributeSignature* signature = NULL;
  if (IsA() == TAG_MSUB || IsA() == TAG_MSUBSUP || IsA() == TAG_MMULTISCRIPTS)
    signature = GetAttributeSignatureAux(id, subSig);
  if ((IsA() == TAG_MSUP || IsA() == TAG_MSUBSUP || IsA() == TAG_MMULTISCRIPTS) && signature == NULL)
    signature = GetAttributeSignatureAux(id, supSig);

  if (signature == NULL) signature = MathMLContainerElement::GetAttributeSignature(id);

  return signature;
}

void
MathMLScriptCommonElement::Setup(RenderingEnvironment* env)
{
  ruleThickness = env->GetRuleThickness();
  sppex = env->GetScaledPointsPerEx();
  subMinShift = float2sp(sp2float(env->GetFontAttributes().size.ToScaledPoints()) * 0.247217);
  superMinShift = float2sp(sp2float(env->GetFontAttributes().size.ToScaledPoints()) * 0.362892);
  scriptAxis    = env->GetAxis();
  scriptSpacing = pt2sp(0); //pt2sp(1); // taken from the TeXbook
  background    = env->GetBackgroundColor();
}

void
MathMLScriptCommonElement::DoLayoutAux(const BoundingBox& baseBox,
				       const BoundingBox& subScriptBox,
				       const BoundingBox& superScriptBox)
{
  assert(base != NULL);

  scaled u;
  scaled v;

  if (base->IsToken() && base->IsA() != TAG_MO) {
    u = v = 0;
  } else {
    u = baseBox.ascent - scriptAxis;
    v = baseBox.descent + scriptAxis / 2;
  }

  if (superScriptBox.IsNull()) {
    u = 0;
    v = scaledMax(v, scaledMax(subMinShift, subScriptBox.ascent - (4 * sppex) / 5));
  } else {
    u = scaledMax(u, scaledMax(superMinShift, superScriptBox.descent + sppex / 4));

    if (subScriptBox.IsNull()) {
      v = 0;
    } else {
      v = scaledMax(v, subMinShift);

      if ((u - superScriptBox.descent) - (subScriptBox.ascent - v) < 4 * ruleThickness) {
	v = 4 * ruleThickness - u + superScriptBox.descent + subScriptBox.ascent;

	scaled psi = (4 * sppex) / 5 - (u - superScriptBox.descent);
	if (psi > 0) {
	  u += psi;
	  v -= psi;
	}
      }
    }
  }

  superShift = u;
  subShift = v;
}

bool
MathMLScriptCommonElement::IsExpanding() const
{
  assert(base != NULL);
  return base->IsExpanding();
}
