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

MathMLScriptCommonElement::MathMLScriptCommonElement()
{
  base = NULL;
}

void
MathMLScriptCommonElement::ScriptSetup(RenderingEnvironment* env)
{
  ruleThickness = env->GetRuleThickness();
#ifdef TEXISH_MATHML
  sppex = env->GetScaledPointsPerEx();
  subMinShift = float2sp(sp2float(env->GetFontAttributes().size.ToScaledPoints()) * 0.247217);
  superMinShift = float2sp(sp2float(env->GetFontAttributes().size.ToScaledPoints()) * 0.362892);
  scriptSpacing = pt2sp(0); //pt2sp(1); // taken from the TeXbook
#else
  sppex = subMinShift = superMinShift = env->GetAxis();
  scriptSpacing = env->ToScaledPoints(env->GetMathSpace(MATH_SPACE_VERYVERYTHIN));
#endif // TEXISH_MATHML
  scriptAxis    = env->GetAxis();
}

void
MathMLScriptCommonElement::DoScriptLayout(const BoundingBox& baseBox,
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
