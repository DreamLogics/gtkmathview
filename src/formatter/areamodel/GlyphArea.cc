// Copyright (C) 2000-2004, Luca Padovani <luca.padovani@cs.unibo.it>.
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
// http://helm.cs.unibo.it/mml-widget/, or send a mail to
// <lpadovan@cs.unibo.it>

#include <config.h>

#include "GlyphArea.hh"
#include "Rectangle.hh"

#include <iostream>

GlyphArea::~GlyphArea()
{ }

bool
GlyphArea::searchByIndex(AreaId&, CharIndex index) const
{
  std::cerr << "GlyphArea::searchByIndex " << index << " length = " << length() << std::endl;
  return (index >= 0 && index <= length());
}

bool
GlyphArea::indexOfPosition(const scaled& x, const scaled& y, CharIndex& index) const
{
  const BoundingBox bbox = box();
  if (Rectangle(scaled::zero(), scaled::zero(), bbox).isInside(x, y))
    {
      index = (x < bbox.width / 2) ? 0 : length();
      return true;
    }
  else
    return false;
}

bool
GlyphArea::positionOfIndex(CharIndex index, scaled& x, scaled& y) const
{
  if (index >= 0 && index < length())
    {
      x = y = 0;
      return true;
    }
  else
    return false;
}