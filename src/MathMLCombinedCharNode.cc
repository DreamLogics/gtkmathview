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
#include "stringAux.hh"
#include "CharMapper.hh"
#include "MathMLElement.hh"
#include "RenderingEnvironment.hh"
#include "MathMLCombinedCharNode.hh"

MathMLCombinedCharNode::MathMLCombinedCharNode(Char c, Char cc) :
  MathMLCharNode(c)
{
  cChar = MathMLCharNode::create(cc);
}

MathMLCombinedCharNode::~MathMLCombinedCharNode()
{
}

void
MathMLCombinedCharNode::Setup(RenderingEnvironment& env)
{
  MathMLCharNode::Setup(env);

  env.Push();
  env.SetFontMode(FONT_MODE_ANY);
  env.SetFontStyle(FONT_STYLE_NORMAL);

  // this is really ugly, but in some sense is also true...
  cChar->SetParent(GetParent());
  cChar->Setup(env);

  if (cChar->GetFont() != fChar.font)
    Globals::logger(LOG_WARNING, "base char `U+%04x' and combining char `U+%04x' use different fonts",
		    ch, cChar->GetChar());

  env.Drop();
}

void
MathMLCombinedCharNode::DoLayout(const FormattingContext& ctxt)
{
  MathMLCharNode::DoLayout(ctxt);
  assert(cChar);
  cChar->DoLayout(ctxt);

  if (IsFontified() && cChar->IsFontified()) {
    const BoundingBox& cBox = cChar->GetBoundingBox();

    bool res = CombineWith(cChar, shiftX, shiftY);
    assert(res);

    box.ascent = scaledMax(charBox.ascent, cBox.ascent + shiftY);
    box.descent = scaledMax(charBox.descent, cBox.descent - shiftY);
    if (cChar->GetChar() != 0x20dd)
      box.width = scaledMax(charBox.width, cBox.width + shiftX);
    else
      box.width = scaledMax(charBox.width, cBox.width);
    box.lBearing = scaledMin(charBox.lBearing, cBox.lBearing + shiftX);
    box.rBearing = scaledMax(charBox.rBearing, cBox.rBearing + shiftX);
    charBox = box; // WARNING: watch this
  }
}

void
MathMLCombinedCharNode::SetPosition(scaled x, scaled y)
{
  //printf("set position: x %d y %d dx %d dy %d\n", sp2ipx(x), sp2ipx(y), sp2ipx(shiftX), sp2ipx(shiftY));
  if (cChar->GetChar() != 0x20dd)
    {
      MathMLCharNode::SetPosition(x, y);
      assert(cChar != NULL);
      cChar->SetPosition(x + shiftX, y - shiftY);
    }
  else
    {
      const BoundingBox& cBox = cChar->GetBoundingBox();
      
      MathMLCharNode::SetPosition(x - cBox.lBearing - shiftX, y);
      assert(cChar != NULL);
      cChar->SetPosition(x - cBox.lBearing, y - shiftY);
    }
}

void
MathMLCombinedCharNode::Render(const DrawingArea& area)
{
  MathMLCharNode::Render(area);
  assert(cChar);
  if (cChar->IsFontified()) cChar->Render(area);
}

bool
MathMLCombinedCharNode::IsCombinedChar() const
{
  return true;
}

String*
MathMLCombinedCharNode::GetRawContent() const
{
  assert(cChar);

  Char c[2];

  c[0] = ch;
  c[1] = cChar->GetChar();

  return allocString(c, 2);
}
