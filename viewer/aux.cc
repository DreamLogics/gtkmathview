// Copyright (C) 2000-2002, Luca Padovani <luca.padovani@cs.unibo.it>.
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
// http://www.cs.unibo.it/helm/mml-widget, or send a mail to
// <luca.padovani@cs.unibo.it>

#include <config.h>
#include <assert.h>

#include <gdome.h>
#include <gdome-util.h>

#include "gmetadom.hh"

static unsigned
getDepth(const DOM::Element& elem)
{
  unsigned length = 0;
  DOM::Element p = elem;

  while (p)
    {
      p = p.get_parentNode();
      length++;
    }

  return length;
}

static DOM::Element
findCommonAncestor(const DOM::Element& first, const DOM::Element& last)
{
  assert(first);
  assert(last);

  DOM::Element p(first);
  DOM::Element q(last);

  if (p != q)
    {
      unsigned pDepth = getDepth(p);
      unsigned qDepth  = getDepth(q);

      while (p && pDepth > qDepth)
	{
	  p = p.get_parentNode();
	  pDepth--;
	}

      while (q && qDepth > pDepth)
	{
	  q = q.get_parentNode();
	  qDepth--;
	}

      assert(pDepth == qDepth);

      while (p && q && p != q)
	{
	  p = p.get_parentNode();
	  q = q.get_parentNode();
	}
    }
  
  return p;
}

static void
findCommonSiblings(const DOM::Element& first, const DOM::Element& last,
		   DOM::Element& firstS, DOM::Element& lastS)
{
  assert(first);
  assert(last);

  DOM::Element p(first);
  DOM::Element q(last);

  if (p != q)
    {
      unsigned pDepth = getDepth(p);
      unsigned qDepth  = getDepth(q);

      while (p && pDepth > qDepth)
	{
	  p = p.get_parentNode();
	  pDepth--;
	}

      while (q && qDepth > pDepth)
	{
	  q = q.get_parentNode();
	  qDepth--;
	}

      assert(pDepth == qDepth);

      while (p && q && p.get_parentNode() != q.get_parentNode())
	{
	  p = p.get_parentNode();
	  q = q.get_parentNode();
	}
    }

  firstS = p;
  lastS = q;
}

static DOM::Node
leftmostChild(const DOM::Node& node)
{
  if (!node) return node;

  DOM::Node firstChild = node.get_firstChild();
  if (!firstChild) return node;

  return leftmostChild(firstChild);
}

static DOM::Node
rightmostChild(const DOM::Node& node)
{
  if (!node) return node;

  DOM::Node lastChild = node.get_lastChild();
  if (!lastChild) return node;

  return rightmostChild(lastChild);
}

static DOM::Node
leftSibling(const DOM::Node& node)
{
  DOM::Node p = node;

  if (!p) return p;

  while (p.get_parentNode() && p.get_parentNode().get_firstChild() == p)
    p = p.get_parentNode();

  if (!p.get_parentNode()) return DOM::Node(0);

  DOM::Node prevSibling = p.get_previousSibling();
  assert(prevSibling);

  return rightmostChild(prevSibling);
}

static DOM::Node
rightSibling(const DOM::Node& node)
{
  DOM::Node p = node;

  if (!p) return p;

  DOM::Node firstChild = p.get_firstChild();
  if (firstChild) return firstChild;

  while (p.get_parentNode() && p.get_parentNode().get_lastChild() == p)
    p = p.get_parentNode();

  if (!p.get_parentNode()) return DOM::Node(0);

  DOM::Node nextSibling = p.get_nextSibling();
  assert(nextSibling);

  return leftmostChild(nextSibling);
}

extern "C" GdomeElement*
find_common_ancestor(GdomeElement* first, GdomeElement* last)
{
  DOM::Element p(first);
  DOM::Element q(last);
  if (GdomeElement* res = gdome_cast_el(findCommonAncestor(p, q).gdome_object()))
    {
      GdomeException exc = 0;
      gdome_el_ref(res, &exc);
      assert(exc == 0);
      return res;
    }
  else
    return 0;
}

extern "C" void
find_common_siblings(GdomeElement* first, GdomeElement* last,
		     GdomeElement** firstS, GdomeElement** lastS)
{
  DOM::Element fs(0);
  DOM::Element ls(0);

  findCommonSiblings(DOM::Element(first), DOM::Element(last), fs, ls);

  if (firstS != NULL) *firstS = gdome_cast_el(fs.gdome_object());
  if (lastS != NULL) *lastS = gdome_cast_el(ls.gdome_object());
}

extern "C" void
delete_element(GdomeElement* elem)
{
  DOM::Element p(elem);

  DOM::Element parent = p.get_parentNode();
  assert(parent);

  parent.removeChild(p);
}

