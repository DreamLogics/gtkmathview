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

#include "scaledAux.hh"
#include "HorizontalSpaceArea.hh"

HorizontalSpaceArea::~HorizontalSpaceArea()
{
}

BoundingBox
HorizontalSpaceArea::box() const
{
  return BoundingBox(width, scaled::min(), scaled::min());
}

DOM::Element
HorizontalSpaceArea::dump(const DOM::Document& doc) const
{
  DOM::Element el = doc.createElementNS(STD_AREAMODEL_NAMESPACE_URI, "a:h-space");
  el.setAttribute("width", toString(width));
  return el;
}

scaled
HorizontalSpaceArea::leftEdge() const
{
  return scaled::min();
}

scaled
HorizontalSpaceArea::rightEdge() const
{
  return scaled::min();
}
