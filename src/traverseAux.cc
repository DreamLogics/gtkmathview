// Copyright (C) 2000-2001, Luca Padovani <luca.padovani@cs.unibo.it>.
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

#include "Iterator.hh"
#include "traverseAux.hh"
#include "MathMLElement.hh"
#include "MathMLRowElement.hh"
#include "MathMLActionElement.hh"
#include "MathMLOperatorElement.hh"
// the following are needed for the dynamic casts
#include "MathMLScriptElement.hh"
#include "MathMLUnderOverElement.hh"
#include "MathMLFractionElement.hh"
#include "MathMLMultiScriptsElement.hh"
#include "MathMLSemanticsElement.hh"
#include "MathMLStyleElement.hh"
#include "MathMLPhantomElement.hh"
#include "MathMLPaddedElement.hh"
// ...
#include "MathMLEmbellishedOperatorElement.hh"

Ptr<MathMLElement>
findEmbellishedOperatorRoot(const Ptr<MathMLElement>& op)
{
  Ptr<MathMLElement> root = op;

  while (root && root->GetParent())
    {
      Ptr<MathMLElement> newRoot = op->GetParent()->GetCoreOperator();
      if (newRoot != root) break;
      else root = op->GetParent();
    }

  return root;
}

#if 0
Ptr<MathMLElement>
findEmbellishedOperatorRoot(const Ptr<MathMLElement>& root)
{
  assert(root);

  if (!root->GetParent()) return root;

  Ptr<MathMLContainerElement> rootParent = smart_cast<MathMLContainerElement>(root->GetParent());
  assert(rootParent);

  if (is_a<MathMLRowElement>(rootParent))
    {
      Ptr<MathMLRowElement> row = smart_cast<MathMLRowElement>(rootParent);
      assert(row);

      for (std::vector< Ptr<MathMLElement> >::const_iterator i = row->GetContent().begin();
	   i != row->GetContent().end();
	   i++)
	{
	  Ptr<MathMLElement> elem = *i;
	  assert(elem);
	  if (!elem->IsSpaceLike() && root != elem) return root;
	}

      return findEmbellishedOperatorRoot(rootParent);
    }
  else if (is_a<MathMLScriptElement>(rootParent) ||
	   is_a<MathMLUnderOverElement>(rootParent) ||
	   is_a<MathMLMultiScriptsElement>(rootParent) ||
	   is_a<MathMLFractionElement>(rootParent) ||
	   is_a<MathMLSemanticsElement>(rootParent))
    {
      Ptr<MathMLLinearContainerElement> cont = smart_cast<MathMLLinearContainerElement>(rootParent);
      assert(cont);

      if (cont->GetSize() > 0 && cont->GetChild(0) != root) return root;
      else return findEmbellishedOperatorRoot(rootParent);
    }
  else if (is_a<MathMLStyleElement>(rootParent) ||
	   is_a<MathMLPhantomElement>(rootParent) ||
	   is_a<MathMLPaddedElement>(rootParent))
    return findEmbellishedOperatorRoot(rootParent);
  else
    return root;
}
#endif

Ptr<MathMLOperatorElement>
findStretchyOperator(const Ptr<MathMLElement>& elem)
{
  if (!elem) return Ptr<MathMLOperatorElement>(0);

  if (is_a<MathMLEmbellishedOperatorElement>(elem)) {
    Ptr<MathMLEmbellishedOperatorElement> eop = smart_cast<MathMLEmbellishedOperatorElement>(elem);
    assert(eop);
    Ptr<MathMLOperatorElement> op = eop->GetCoreOperator();
    assert(op);

    if (op->IsStretchy()) return op;
    else return NULL;
  } else if (is_a<MathMLOperatorElement>(elem)) {
    Ptr<MathMLOperatorElement> op = smart_cast<MathMLOperatorElement>(elem);
    assert(op);
    if (op->IsStretchy()) return op;
    else return 0;
  } else
    return 0;
}

Ptr<MathMLOperatorElement>
findStretchyOperator(const Ptr<MathMLElement>& elem, StretchId id)
{
  Ptr<MathMLOperatorElement> op = findStretchyOperator(elem);
  if (!op) return 0;
  if (op->GetStretch() != id) return Ptr<MathMLOperatorElement>(0);
  return op;
}

Ptr<MathMLElement>
findCommonAncestor(const Ptr<MathMLElement>& first, const Ptr<MathMLElement>& last)
{
  assert(first);
  assert(last);

  Ptr<MathMLElement> firstP(first);
  Ptr<MathMLElement> lastP(last);

  if (firstP != lastP)
    {
      unsigned firstDepth = first->GetDepth();
      unsigned lastDepth  = last->GetDepth();

      while (firstP && firstDepth > lastDepth)
	{
	  firstP = firstP->GetParent();
	  firstDepth--;
	}

      while (lastP && lastDepth > firstDepth)
	{
	  lastP = lastP->GetParent();
	  lastDepth--;
	}

      assert(firstDepth == lastDepth);

      while (firstP && lastP && firstP != lastP)
	{
	  firstP = firstP->GetParent();
	  lastP = lastP->GetParent();
	}
    }
  
  return firstP;
}

Ptr<MathMLActionElement>
findActionElement(const Ptr<MathMLElement>& elem)
{
  Ptr<MathMLElement> elemP(elem);

  while (elemP && elemP->IsA() != TAG_MACTION)
    elemP = elemP->GetParent();

  return (elemP) ? smart_cast<MathMLActionElement>(elemP) : Ptr<MathMLActionElement>(0);
}

#if defined(HAVE_GMETADOM)

GMetaDOM::Element
findDOMNode(const Ptr<MathMLElement>& elem)
{
  Ptr<MathMLElement> elemP(elem);

  while (elemP && !elemP->GetDOMElement())
    elemP = elemP->GetParent();

  if (elemP) return elemP->GetDOMElement();
  else return GMetaDOM::Element(0);
}

Ptr<MathMLElement>
getRenderingInterface(const GMetaDOM::Element& node)
{
  // WARNING: the following is a very dangerous operation. It relies
  // of the assumption that the user will NEVER modify the user data field
  // in the DOM tree elements!!!
  Ptr<MathMLElement> elem((MathMLElement*) node.get_userData());
  assert(!elem || elem->GetDOMElement() == node);
  return elem;
}

void
setRenderingInterface(const GMetaDOM::Element& node, const Ptr<MathMLElement>& elem)
{
  assert(node);
  assert(elem);
  node.set_userData(elem);
}

Ptr<MathMLElement>
findMathMLElement(const GMetaDOM::Element& node)
{
  Ptr<MathMLElement> elem = ::getRenderingInterface(node);
  assert(elem);

  while (Ptr<MathMLRowElement> row = smart_cast<MathMLRowElement>(elem))
    {
      if (row->GetSize() != 1) break;
      elem = row->GetChild(0);
    }

  return elem;
}

#endif // HAVE_GMETADOM

Ptr<MathMLElement>
findRightmostChild(const Ptr<MathMLElement>& elem)
{
  if (!elem || elem->IsA() != TAG_MROW) return elem;

  Ptr<MathMLRowElement> row = smart_cast<MathMLRowElement>(elem);
  assert(row);
  if (row->GetSize() == 0) return elem;

  return findRightmostChild(row->GetChild(row->GetSize() - 1));
}

Ptr<MathMLElement>
findLeftmostChild(const Ptr<MathMLElement>& elem)
{
  if (!elem || elem->IsA() != TAG_MROW) return elem;

  Ptr<MathMLRowElement> row = smart_cast<MathMLRowElement>(elem);
  assert(row);
  if (row->GetSize() == 0) return elem;

  return findLeftmostChild(row->GetChild(0));
}

#if defined(HAVE_MINIDOM)

MathMLElement*
findRightSibling(MathMLElement* elem)
{
  mDOMNodeRef p = findDOMNode(elem);
  if (p == NULL) return NULL;

  for (p = mdom_node_get_next_sibling(p);
       p != NULL && mdom_node_get_user_data(p) == NULL;
       p = mdom_node_get_next_sibling(p)) ;

  if (p != NULL) return findLeftmostChild(findMathMLElement(p));
  else return findRightmostChild(findRightSibling(elem->GetParent()));
}

MathMLElement*
findLeftSibling(MathMLElement* elem)
{
  mDOMNodeRef p = findDOMNode(elem);
  if (p == NULL) return NULL;

  for (p = mdom_node_get_prev_sibling(p);
       p != NULL && mdom_node_get_user_data(p) == NULL;
       p = mdom_node_get_prev_sibling(p)) ;

  if (p != NULL) return findRightmostChild(findMathMLElement(p));
  else return findLeftmostChild(findLeftSibling(elem->GetParent()));
}

#elif defined(HAVE_GMETADOM)

Ptr<MathMLElement>
findRightSibling(const Ptr<MathMLElement>& elem)
{
  GMetaDOM::Node p = findDOMNode(elem);
  if (!p) return 0;

  for (p = p.get_nextSibling();
       p && p.get_userData() == NULL;
       p = p.get_nextSibling()) ;
  
  if (p) return findLeftmostChild(findMathMLElement(p));
  else return findRightmostChild(findRightSibling(elem->GetParent()));
}

Ptr<MathMLElement>
findLeftSibling(const Ptr<MathMLElement>& elem)
{
  GMetaDOM::Node p = findDOMNode(elem);
  if (!p) return 0;

  for (p = p.get_previousSibling();
       p && p.get_userData() == NULL;
       p = p.get_previousSibling()) ;

  if (p) return findRightmostChild(findMathMLElement(p));
  else return findLeftmostChild(findLeftSibling(elem->GetParent()));
}

#endif // HAVE_GMETADOM
