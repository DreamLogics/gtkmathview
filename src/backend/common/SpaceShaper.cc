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

#include "MathMLElement.hh"
#include "AreaFactory.hh"
#include "ShapingContext.hh"
#include "MathGraphicDevice.hh"
#include "ShaperManager.hh"
#include "SpaceShaper.hh"

struct FixedSpaceData
{
  Char16 ch;
  int space;
} fixedSpaceMap[] = {
  { 0x0009, 9 * 8 }, // Tab
  { 0x0020,  9 }, // Space
  { 0x00a0,  9 }, // NonBreakingSpace
  { 0x2000,  9 }, // EN QUAD
  { 0x2001, 18 }, // EM QUAD
  { 0x2002,  9 }, // EN SPACE (HALF AN EM)
  { 0x2003, 18 }, // EM SPACE 
  { 0x2004,  6 }, // THREE-PER-EM SPACE (THICK SPACE)
  { 0x2005,  4 }, // FOUR-PER-EM SPACE (MID SPACE)
  { 0x2006,  3 }, // SIX-PER-EM SPACE (THIN SPACE)
  { 0x2009,  3 }, // THIN SPACE
  { 0x200a,  1 }, // HAIR SPACE (VERY THIN SPACE)
  { 0x200b,  0 }, // ZERO-WIDTH SPACE
  { 0x205f,  4 }, // MEDIUM MATH SPACE

  { 0x0000,  0 }
};

void
SpaceShaper::registerShaper(const SmartPtr<ShaperManager>& sm, unsigned shaperId)
{
  assert(sm);

  for (unsigned i = 0; fixedSpaceMap[i].ch; i++)
    sm->registerChar(fixedSpaceMap[i].ch, GlyphSpec(shaperId, 0, i));
}

void
SpaceShaper::unregisterShaper(const SmartPtr<ShaperManager>&, unsigned)
{
  // nothing to do???
}

void
SpaceShaper::shape(ShapingContext& context) const
{
  assert(!context.done());
  shapeFixedSpace(context, context.getSpec());
}

void
SpaceShaper::pushSpace(ShapingContext& context, int space, unsigned n)
{
  assert(n > 0);
  context.pushArea(n, context.getFactory()->horizontalSpace(context.getSize() * space / 18));
}

void
SpaceShaper::shapeFixedSpace(ShapingContext& context, const GlyphSpec& spec)
{
  unsigned n = 1;
  int space = fixedSpaceMap[spec.getGlyphId()].space;
  if (context.nextChar() == 0xfe00)
    {
      space = -space;
      n++;
    }
  pushSpace(context, space, n);
}
