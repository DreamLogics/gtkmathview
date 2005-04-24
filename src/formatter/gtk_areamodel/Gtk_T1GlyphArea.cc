// Copyright (C) 2000-2005, Luca Padovani <luca.padovani@cs.unibo.it>.
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

#include <cassert>

#include <t1lib.h>
#include <t1libx.h>
#include <gdk/gdkx.h>

#include "Char.hh"
#include "T1Font.hh"
#include "Gtk_RenderingContext.hh"
#include "Gtk_T1GlyphArea.hh"

Gtk_T1GlyphArea::Gtk_T1GlyphArea(const SmartPtr<T1Font>& f, Char8 i)
  : font(f), index(i)
{ }

Gtk_T1GlyphArea::~Gtk_T1GlyphArea()
{ }

SmartPtr<Gtk_T1GlyphArea>
Gtk_T1GlyphArea::create(const SmartPtr<T1Font>& font, Char8 index)
{ return new Gtk_T1GlyphArea(font, index); }

SmartPtr<T1Font>
Gtk_T1GlyphArea::getFont() const
{
  return font;
}

BoundingBox
Gtk_T1GlyphArea::box() const
{
  const float scale = font->getScale();
  const BBox charBox = T1_GetCharBBox(font->getFontId(), index);
  return BoundingBox(scale * T1_GetCharWidth(font->getFontId(), index) / 1000.0f,
		     scale * charBox.ury / 1000.0f,
		     scale * (-charBox.lly) / 1000.0f);
}

scaled
Gtk_T1GlyphArea::leftEdge() const
{
  return font->getScale() * T1_GetCharBBox(font->getFontId(), index).llx / 1000.0f;
}

scaled
Gtk_T1GlyphArea::rightEdge() const
{
  return font->getScale() * T1_GetCharBBox(font->getFontId(), index).urx / 1000.0f;
}

void
Gtk_T1GlyphArea::render(RenderingContext& c, const scaled& x, const scaled& y) const
{
  Gtk_RenderingContext& context = dynamic_cast<Gtk_RenderingContext&>(c);
  context.draw(x, y, font, index);
}