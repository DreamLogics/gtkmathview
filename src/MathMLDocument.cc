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
#if defined(HAVE_GMETADOM)
  : DOMdoc(0), DOMroot(0)
#endif
{
}

#if defined(HAVE_GMETADOM)
MathMLDocument::MathMLDocument(const GMetaDOM::Document& doc)
  : MathMLBinContainerElement()
  , DOMdoc(doc), DOMroot(0)
{
  DOMroot = DOMdoc.get_documentElement();
  Init();
}

MathMLDocument::MathMLDocument(const GMetaDOM::Element& root)
  : MathMLBinContainerElement()
  , DOMdoc(0)
  , DOMroot(root)
{
  Init();
}

void
MathMLDocument::Init()
{
  if (DOMroot)
    {
      GMetaDOM::EventTarget et(DOMroot);
      assert(et);

      et.addEventListener("DOMNodeRemoved", subtreeModifiedListener, false);
      et.addEventListener("DOMAttrModified", attrModifiedListener, false);
    }
}
#endif

MathMLDocument::~MathMLDocument()
{
#if defined(HAVE_GMETADOM)
  if (DOMroot)
    {
      GMetaDOM::EventTarget et(DOMroot);
      assert(et);

      et.removeEventListener("DOMSubtreeModified", subtreeModifiedListener, false);
      et.removeEventListener("DOMAttrModified", attrModifiedListener, false);
    }
#endif
}

void
MathMLDocument::Normalize()
{
  if (HasDirtyStructure() || HasChildWithDirtyStructure())
    {
#if defined(HAVE_GMETADOM)
      GMetaDOM::NodeList nodeList = GetDOMDocument().getElementsByTagNameNS(MATHML_NS_URI, "math");
      if (nodeList.get_length() > 0)
	{
	  Ptr<MathMLElement> elem = MathMLElement::getRenderingInterface(nodeList.item(0));
	  assert(elem);
	  SetChild(elem);
	}	  
#endif // HAVE_GMETADOM

      if (child) child->Normalize();
      ResetDirtyStructure();
    }
}

#if defined(HAVE_GMETADOM)

void
MathMLDocument::DOMSubtreeModifiedListener::handleEvent(const GMetaDOM::Event& ev)
{
  const GMetaDOM::MutationEvent& me(ev);
  assert(me);
  printf("subtree modified\n");
}

void
MathMLDocument::DOMAttrModifiedListener::handleEvent(const GMetaDOM::Event& ev)
{
  const GMetaDOM::MutationEvent& me(ev);
  assert(me);
  printf("an attribute changed\n");
}

#endif // HAVE_GMETADOM
