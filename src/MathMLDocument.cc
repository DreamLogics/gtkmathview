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

#include "Globals.hh"
#include "MathMLDocument.hh"

MathMLDocument::MathMLDocument()
  : DOMdoc(0)
{
}

#if defined(HAVE_GMETADOM)
MathMLDocument::MathMLDocument(const GMetaDOM::Document& doc)
  : MathMLBinContainerElement()
  , DOMdoc(doc)
{
  assert(doc != 0);
}
#endif

MathMLDocument::~MathMLDocument()
{
  DOMdoc = 0;
}

void
MathMLDocument::Normalize()
{
  if (HasDirtyStructure() || HasChildWithDirtyStructure())
    {
#if defined(HAVE_GMETADOM)
      GMetaDOM::Element node = GetDOMDocument().get_documentElement();
      assert(node != 0);
      assert(node.get_nodeType() == GMetaDOM::Node::ELEMENT_NODE);

      MathMLElement* elem = MathMLElement::getRenderingInterface(node);
      assert(elem != 0);
      SetChild(elem);
      elem->Release();
#endif // HAVE_GMETADOM

      if (child != 0) child->Normalize();
      ResetDirtyStructure();
    }
}

bool
MathMLDocument::IsDocument() const
{
  return true;
}
