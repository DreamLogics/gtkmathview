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

MathMLElement*
findEmbellishedOperatorRoot(MathMLElement* root)
{
  assert(root != NULL);

  if (root->GetParent() == NULL) return root;

  assert(root->GetParent()->IsContainer());
  MathMLContainerElement* rootParent = TO_CONTAINER(root->GetParent());
  assert(rootParent != NULL);

  switch (rootParent->IsA()) {
  case TAG_MROW:
    {
      MathMLRowElement* row = TO_ROW(rootParent);
      assert(row != 0);

      for (Iterator<MathMLElement*> i(row->GetContent()); i.More(); i.Next())
	{
	  MathMLElement* elem = i();
	  assert(elem != NULL);
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
      MathMLLinearContainerElement* cont = TO_LINEAR_CONTAINER(rootParent);
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

MathMLOperatorElement*
findStretchyOperator(MathMLElement* elem)
{
  if (elem == NULL) return NULL;

  if (elem->IsEmbellishedOperator()) {
    MathMLEmbellishedOperatorElement* eop = TO_EMBELLISHED_OPERATOR(elem);
    assert(eop != NULL);
    MathMLOperatorElement* op = eop->GetCoreOperator();
    assert(op != NULL);

    if (op->IsStretchy()) return op;
    else return NULL;
  } else if (elem->IsOperator()) {
    MathMLOperatorElement* op = TO_OPERATOR(elem);
    if (op->IsStretchy()) return op;
    else return NULL;
  } else
    return NULL;
}

MathMLOperatorElement*
findStretchyOperator(MathMLElement* elem, StretchId id)
{
  MathMLOperatorElement* op = findStretchyOperator(elem);
  if (op == NULL) return NULL;
  if (op->GetStretch() != id) return NULL;
  return op;
}

MathMLElement*
findCommonAncestor(MathMLElement* first, MathMLElement* last)
{
  assert(first != NULL);
  assert(last != NULL);

  if (first != last) {
    unsigned firstDepth = first->GetDepth();
    unsigned lastDepth  = last->GetDepth();

    while (firstDepth > lastDepth) {
      first = first->GetParent();
      firstDepth--;
    }

    while (lastDepth > firstDepth) {
      last = last->GetParent();
      lastDepth--;
    }

    assert(firstDepth == lastDepth);

    while (first != NULL && last != NULL && first != last) {
      first = first->GetParent();
      last = last->GetParent();
    }
  }

  assert(first == last);

  return first;
}

MathMLActionElement*
findActionElement(MathMLElement* elem)
{
  while (elem != NULL && elem->IsA() != TAG_MACTION) elem = elem->GetParent();
  return (elem != NULL) ? TO_ACTION(elem) : NULL;
}

#if defined(HAVE_GMETADOM)

GMetaDOM::Element
findDOMNode(MathMLElement* elem)
{
  while (elem != NULL && elem->GetDOMNode() == 0) elem = elem->GetParent();
  return (elem != NULL) ? elem->GetDOMNode() : 0;
}

MathMLElement*
getRenderingInterface(const GMetaDOM::Element& node)
{
  // WARNING: the following is a very dangerous operation. It relies
  // of the assumption that the user will NEVER modify the user data field
  // in the DOM tree elements!!!
  MathMLElement* elem = (MathMLElement*) node.get_userData();
  if (elem == 0) return 0;
  assert(elem->GetDOMNode() == node);
  return elem;
}

void
setRenderingInterface(const GMetaDOM::Element& node, MathMLElement* elem)
{
  assert(node != 0);

  MathMLElement* oldElem = ::getRenderingInterface(node);
  if (elem != 0) elem->AddRef();
  if (oldElem != 0) oldElem->Release();

  node.set_userData(elem);
}

MathMLElement*
findMathMLElement(const GMetaDOM::Element& node)
{
  MathMLElement* elem = ::getRenderingInterface(node);
  assert(elem != NULL);

  while (elem->IsA() == TAG_MROW && TO_ROW(elem)->GetContent().GetSize() == 1) {
    MathMLRowElement* row = TO_ROW(elem);
    assert(row != NULL);
    MathMLElement* child = row->GetContent().GetFirst();
    assert(child != NULL);
    child->AddRef();
    elem->Release();
    elem = child;
  }

  return elem;
}

#endif // HAVE_GMETADOM

MathMLElement*
findRightmostChild(MathMLElement* elem)
{
  if (elem == NULL) return NULL;

  if (elem->IsA() != TAG_MROW)
    {
      elem->AddRef();
      return elem;
    }

  MathMLRowElement* row = TO_ROW(elem);
  assert(row != NULL);
  if (row->GetContent().GetSize() == 0)
    {
      elem->AddRef();
      return elem;
    }

  return findRightmostChild(row->GetContent().GetLast());
}

MathMLElement*
findLeftmostChild(MathMLElement* elem)
{
  if (elem == NULL) return NULL;

  if (elem->IsA() != TAG_MROW)
    {
      elem->AddRef();
      return elem;
    }

  MathMLRowElement* row = TO_ROW(elem);
  assert(row != NULL);
  if (row->GetContent().GetSize() == 0)
    {
      elem->AddRef();
      return elem;
    }

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

MathMLElement*
findRightSibling(MathMLElement* elem)
{
  GMetaDOM::Node p = findDOMNode(elem);
  if (p == 0) return NULL;

  for (p = p.get_nextSibling();
       p != 0 && p.get_userData() == NULL;
       p = p.get_nextSibling()) ;
  
  if (p != 0) return findLeftmostChild(findMathMLElement(p));
  else return findRightmostChild(findRightSibling(elem->GetParent()));
}

MathMLElement*
findLeftSibling(MathMLElement* elem)
{
  GMetaDOM::Node p = findDOMNode(elem);
  if (p == NULL) return NULL;

  for (p = p.get_previousSibling();
       p != 0 && p.get_userData() == NULL;
       p = p.get_previousSibling()) ;

  if (p != 0) return findRightmostChild(findMathMLElement(p));
  else return findLeftmostChild(findLeftSibling(elem->GetParent()));
}

#endif // HAVE_GMETADOM
