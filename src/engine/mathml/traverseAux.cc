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

#include "traverseAux.hh"
#include "MathMLElement.hh"
#include "MathMLRowElement.hh"
#include "MathMLActionElement.hh"
#include "MathMLOperatorElement.hh"
#include "MathMLView.hh"
#include "MathMLDOMLinker.hh"
// the following are needed for the dynamic casts
#include "MathMLScriptElement.hh"
#include "MathMLUnderOverElement.hh"
#include "MathMLFractionElement.hh"
#include "MathMLMultiScriptsElement.hh"
#include "MathMLSemanticsElement.hh"
#include "MathMLStyleElement.hh"
#include "MathMLPhantomElement.hh"
#include "MathMLPaddedElement.hh"

SmartPtr<MathMLElement>
findEmbellishedOperatorRoot(const SmartPtr<MathMLElement>& op)
{
  SmartPtr<MathMLElement> root = op;

  while (root && root->getParent())
    {
      SmartPtr<MathMLElement> newRoot = op->getParent()->getCoreOperator();
      if (newRoot != root) break;
      else root = op->getParent();
    }

  return root;
}

#if 0
SmartPtr<MathMLElement>
findEmbellishedOperatorRoot(const SmartPtr<MathMLElement>& root)
{
  assert(root);

  if (!root->getParent()) return root;

  SmartPtr<MathMLContainerElement> rootParent = smart_cast<MathMLContainerElement>(root->getParent());
  assert(rootParent);

  if (is_a<MathMLRowElement>(rootParent))
    {
      SmartPtr<MathMLRowElement> row = smart_cast<MathMLRowElement>(rootParent);
      assert(row);

      for (std::vector< SmartPtr<MathMLElement> >::const_iterator i = row->GetContent().begin();
	   i != row->GetContent().end();
	   i++)
	{
	  SmartPtr<MathMLElement> elem = *i;
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
      SmartPtr<MathMLLinearContainerElement> cont = smart_cast<MathMLLinearContainerElement>(rootParent);
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

SmartPtr<MathMLOperatorElement>
findStretchyOperator(const SmartPtr<MathMLElement>& elem)
{
  if (elem)
    if (SmartPtr<MathMLOperatorElement> coreOp = elem->getCoreOperator())
      if (coreOp->IsStretchy()) return coreOp;
  return 0;
}

SmartPtr<MathMLElement>
findCommonAncestor(const SmartPtr<MathMLElement>& first, const SmartPtr<MathMLElement>& last)
{
  assert(first);
  assert(last);

  SmartPtr<MathMLElement> firstP(first);
  SmartPtr<MathMLElement> lastP(last);

  if (firstP != lastP)
    {
      unsigned firstDepth = first->GetDepth();
      unsigned lastDepth  = last->GetDepth();

      while (firstP && firstDepth > lastDepth)
	{
	  firstP = firstP->getParent();
	  firstDepth--;
	}

      while (lastP && lastDepth > firstDepth)
	{
	  lastP = lastP->getParent();
	  lastDepth--;
	}

      assert(firstDepth == lastDepth);

      while (firstP && lastP && firstP != lastP)
	{
	  firstP = firstP->getParent();
	  lastP = lastP->getParent();
	}
    }
  
  return firstP;
}

SmartPtr<MathMLActionElement>
findActionElement(const SmartPtr<MathMLElement>& elem)
{
  SmartPtr<MathMLElement> elemP(elem);

  while (elemP && elemP->IsA() != T_MACTION)
    elemP = elemP->getParent();

  return (elemP) ? smart_cast<MathMLActionElement>(elemP) : SmartPtr<MathMLActionElement>(0);
}

#if defined(HAVE_GMETADOM)

DOM::Element
findDOMNode(const SmartPtr<MathMLElement>& elem)
{
  SmartPtr<MathMLElement> elemP(elem);

  while (elemP && !elemP->getDOMElement())
    elemP = elemP->getParent();

  if (elemP) return elemP->getDOMElement();
  else return DOM::Element(0);
}

SmartPtr<MathMLElement>
findMathMLElement(const SmartPtr<MathMLView>& view, const DOM::Element& node)
{
  if (SmartPtr<MathMLElement> elem = view->getContext()->linker->get(node))
    {
      while (SmartPtr<MathMLRowElement> row = smart_cast<MathMLRowElement>(elem))
	{
	  if (row->getSize() != 1) break;
	  elem = row->getChild(0);
	}
      return elem;
    }
  else
    return 0;
}

#endif // HAVE_GMETADOM

SmartPtr<MathMLElement>
findRightmostChild(const SmartPtr<MathMLElement>& elem)
{
  if (SmartPtr<MathMLRowElement> row = smart_cast<MathMLRowElement>(elem))
    {
      if (row->getSize() == 0) return elem;
      else return findRightmostChild(row->getChild(row->getSize() - 1));
    }
  else
    return elem;
}

SmartPtr<MathMLElement>
findLeftmostChild(const SmartPtr<MathMLElement>& elem)
{
  if (SmartPtr<MathMLRowElement> row = smart_cast<MathMLRowElement>(elem))
    {
      if (row->getSize() == 0) return elem;
      else return findLeftmostChild(row->getChild(0));
    }
  else
    return elem;
}

SmartPtr<MathMLElement>
findRightSibling(const SmartPtr<MathMLElement>& elem)
{
  if (!elem)
    return 0;
  else if (SmartPtr<MathMLRowElement> row = smart_cast<MathMLRowElement>(elem->getParent()))
    {
      std::vector< SmartPtr<MathMLElement> >::const_iterator p =
	std::find(row->getContent().begin(), row->getContent().end(), elem);
      assert(p != row->getContent().end());
      if (p + 1 != row->getContent().end()) return findLeftmostChild(*(p + 1));
      else return findRightSibling(row);
    }
  else
    return findRightSibling(elem->getParent());
}

SmartPtr<MathMLElement>
findLeftSibling(const SmartPtr<MathMLElement>& elem)
{
  if (!elem)
    return 0;
  else if (SmartPtr<MathMLRowElement> row = smart_cast<MathMLRowElement>(elem->getParent()))
    {
      std::vector< SmartPtr<MathMLElement> >::const_iterator p =
	std::find(row->getContent().begin(), row->getContent().end(), elem);
      assert(p != row->getContent().end());
      if (p != row->getContent().begin()) return findRightmostChild(*(p - 1));
      else return findLeftSibling(row);
    }
  else
    return findLeftSibling(elem->getParent());
}
