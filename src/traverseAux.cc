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
#include "MathMLEmbellishedOperatorElement.hh"

Ptr<MathMLElement>
findEmbellishedOperatorRoot(const Ptr<MathMLElement>& root)
{
  assert(root != 0);

  if (root->GetParent() == 0) return root;

  assert(root->GetParent()->IsContainer());
  Ptr<MathMLContainerElement> rootParent = smart_cast<MathMLContainerElement>(root->GetParent());
  assert(rootParent != 0);

  switch (rootParent->IsA()) {
  case TAG_MROW:
    {
      Ptr<MathMLRowElement> row = smart_cast<MathMLRowElement>(rootParent);
      assert(row != 0);

      for (Iterator< Ptr<MathMLElement> > i(row->GetContent()); i.More(); i.Next())
	{
	  Ptr<MathMLElement> elem = i();
	  assert(elem != 0);
	  if (!elem->IsSpaceLike() && root != elem) return root;
	}

      return findEmbellishedOperatorRoot(rootParent);
    }
  case TAG_MSUP:
  case TAG_MSUB:
  case TAG_MSUBSUP:
  case TAG_MUNDER:
  case TAG_MOVER:
  case TAG_MUNDEROVER:
  case TAG_MMULTISCRIPTS:
  case TAG_MFRAC:
  case TAG_SEMANTICS:
    {
      Ptr<MathMLLinearContainerElement> cont = smart_cast<MathMLLinearContainerElement>(rootParent);
      assert(cont != 0);

      if (cont->GetContent().GetSize() > 0 &&
	  cont->GetContent().GetFirst() != root) return root;
      else return findEmbellishedOperatorRoot(rootParent);
    }
  case TAG_MSTYLE:
  case TAG_MPHANTOM:
  case TAG_MPADDED:
    return findEmbellishedOperatorRoot(rootParent);
  default:
    return root;
  }
}

Ptr<MathMLOperatorElement>
findStretchyOperator(const Ptr<MathMLElement>& elem)
{
  if (elem == 0) return 0;

  if (elem->IsEmbellishedOperator()) {
    Ptr<MathMLEmbellishedOperatorElement> eop = smart_cast<MathMLEmbellishedOperatorElement>(elem);
    assert(eop != 0);
    Ptr<MathMLOperatorElement> op = eop->GetCoreOperator();
    assert(op != 0);

    if (op->IsStretchy()) return op;
    else return NULL;
  } else if (elem->IsOperator()) {
    Ptr<MathMLOperatorElement> op = smart_cast<MathMLOperatorElement>(elem);
    if (op->IsStretchy()) return op;
    else return 0;
  } else
    return 0;
}

Ptr<MathMLOperatorElement>
findStretchyOperator(const Ptr<MathMLElement>& elem, StretchId id)
{
  Ptr<MathMLOperatorElement> op = findStretchyOperator(elem);
  if (op == 0) return 0;
  if (op->GetStretch() != id) return 0;
  return op;
}

Ptr<MathMLElement>
findCommonAncestor(const Ptr<MathMLElement>& first, const Ptr<MathMLElement>& last)
{
  assert(first != 0);
  assert(last != 0);

  Ptr<MathMLElement> firstP(first);
  Ptr<MathMLElement> lastP(last);

  if (firstP != lastP)
    {
      unsigned firstDepth = first->GetDepth();
      unsigned lastDepth  = last->GetDepth();

      while (firstP != 0 && firstDepth > lastDepth)
	{
	  firstP = firstP->GetParent();
	  firstDepth--;
	}

      while (lastP != 0 && lastDepth > firstDepth)
	{
	  lastP = lastP->GetParent();
	  lastDepth--;
	}

      assert(firstDepth == lastDepth);

      while (firstP != 0 && lastP != 0 && firstP != lastP)
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

  while (elemP != 0 && elemP->IsA() != TAG_MACTION)
    elemP = elemP->GetParent();

  return (elemP != 0) ? smart_cast<MathMLActionElement>(elemP) : 0;
}

#if defined(HAVE_GMETADOM)

GMetaDOM::Element
findDOMNode(const Ptr<MathMLElement>& elem)
{
  Ptr<MathMLElement> elemP(elem);

  while (elemP != 0 && elemP->GetDOMElement() == 0)
    elemP = elemP->GetParent();

  return (elemP != 0) ? elemP->GetDOMElement() : 0;
}

Ptr<MathMLElement>
getRenderingInterface(const GMetaDOM::Element& node)
{
  // WARNING: the following is a very dangerous operation. It relies
  // of the assumption that the user will NEVER modify the user data field
  // in the DOM tree elements!!!
  Ptr<MathMLElement> elem((MathMLElement*) node.get_userData());
  assert(elem == 0 || elem->GetDOMElement() == node);
  return elem;
}

void
setRenderingInterface(const GMetaDOM::Element& node, const Ptr<MathMLElement>& elem)
{
  assert(node != 0);
  // WARNING: this will allow to decrement the counter of the element whose
  // pointer is currently stored inside node as soon as oldElem goes
  // out of scope
  Ptr<MathMLElement> oldElem(0);
  oldElem.set(node.get_userData());
  node.set_userData(elem.get());
}

Ptr<MathMLElement>
findMathMLElement(const GMetaDOM::Element& node)
{
  Ptr<MathMLElement> elem = ::getRenderingInterface(node);
  assert(elem != NULL);

  while (elem->IsA() == TAG_MROW &&
	 smart_cast<MathMLRowElement>(elem)->GetContent().GetSize() == 1)
    {
      Ptr<MathMLRowElement> row = smart_cast<MathMLRowElement>(elem);
      assert(row != 0);
      elem = row->GetContent().GetFirst();
    }

  return elem;
}

#endif // HAVE_GMETADOM

Ptr<MathMLElement>
findRightmostChild(const Ptr<MathMLElement>& elem)
{
  if (elem == 0 || elem->IsA() != TAG_MROW) return elem;

  Ptr<MathMLRowElement> row = smart_cast<MathMLRowElement>(elem);
  assert(row != 0);
  if (row->GetContent().GetSize() == 0) return elem;

  return findRightmostChild(row->GetContent().GetLast());
}

Ptr<MathMLElement>
findLeftmostChild(const Ptr<MathMLElement>& elem)
{
  if (elem == 0 || elem->IsA() != TAG_MROW) return elem;

  Ptr<MathMLRowElement> row = smart_cast<MathMLRowElement>(elem);
  assert(row != 0);
  if (row->GetContent().GetSize() == 0) return elem;

  return findLeftmostChild(row->GetContent().GetFirst());
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
  if (p == 0) return 0;

  for (p = p.get_nextSibling();
       p != 0 && p.get_userData() == NULL;
       p = p.get_nextSibling()) ;
  
  if (p != 0) return findLeftmostChild(findMathMLElement(p));
  else return findRightmostChild(findRightSibling(elem->GetParent()));
}

Ptr<MathMLElement>
findLeftSibling(const Ptr<MathMLElement>& elem)
{
  GMetaDOM::Node p = findDOMNode(elem);
  if (p == 0) return 0;

  for (p = p.get_previousSibling();
       p != 0 && p.get_userData() == NULL;
       p = p.get_previousSibling()) ;

  if (p != 0) return findRightmostChild(findMathMLElement(p));
  else return findLeftmostChild(findLeftSibling(elem->GetParent()));
}

#endif // HAVE_GMETADOM
