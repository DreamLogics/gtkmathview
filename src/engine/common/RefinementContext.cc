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

#include <cassert>

#include "RefinementContext.hh"

SmartPtr<Attribute>
RefinementContext::get(const AttributeSignature& sig) const
{
  for (std::list<Context>::const_iterator p = context.begin(); p != context.end(); p++)
    {
      const Context& c = *p;

      if (SmartPtr<Attribute> attr = c.attributes->get(ATTRIBUTE_ID_OF_SIGNATURE(sig)))
	return attr;
      else if (c.elem.hasAttribute(sig.name))
	{
	  SmartPtr<Attribute> attr = Attribute::create(sig, c.elem.getAttribute(sig.name));
	  c.attributes->set(attr);
	  return attr;
	}
    }

  return 0;
}

void
RefinementContext::push(const DOM::Element& elem)
{
  assert(elem);
  context.push_front(Context(elem, AttributeList::create()));
}

void
RefinementContext::pop()
{
  assert(!context.empty());
  context.pop_front();
}
