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
#include "ChildList.hh"
#include "stringAux.hh"
#include "allocTextNode.hh"
#include "StringUnicode.hh"
#include "MathMLCharNode.hh"
#include "MathMLRowElement.hh"
#include "MathMLStringNode.hh"
#include "MathMLFencedElement.hh"
#include "MathMLOperatorElement.hh"

MathMLFencedElement::MathMLFencedElement()
{
  normalized = false;
  openFence = closeFence = separators = NULL;
}

#if defined(HAVE_GMETADOM)
MathMLFencedElement::MathMLFencedElement(const GMetaDOM::Element& node)
  : MathMLBinContainerElement(node)
{
  normalized = false;
  openFence = closeFence = separators = NULL;
}
#endif

MathMLFencedElement::~MathMLFencedElement()
{
  delete openFence;
  delete closeFence;
  delete separators;
}

const AttributeSignature*
MathMLFencedElement::GetAttributeSignature(AttributeId id) const
{
  static AttributeSignature sig[] = {
    { ATTR_OPEN,       fenceParser,      new StringC("("), NULL },
    { ATTR_CLOSE,      fenceParser,      new StringC(")"), NULL },
    { ATTR_SEPARATORS, separatorsParser, new StringC(","), NULL },
    { ATTR_NOTVALID,   NULL,             NULL,             NULL }
  };

  const AttributeSignature* signature = GetAttributeSignatureAux(id, sig);
  if (signature == NULL) signature = MathMLBinContainerElement::GetAttributeSignature(id);

  return signature;
}

void
MathMLFencedElement::Normalize()
{
  if (HasDirtyStructure() || HasChildWithDirtyStructure())
    {
      // normalization is delayed after setup, because it depends on
      // the value of some attributes
      // maybe it can be optimized if it knows that none of its children were
      // added or removed, because that way it can just propagate the
      // normalization method
      normalized = false;
      ResetDirtyStructure();
    }
}

void
MathMLFencedElement::Setup(RenderingEnvironment* env)
{
  assert(env != NULL);

  const Value* value = NULL;

  value = GetAttributeValue(ATTR_OPEN, env);
  if (value != NULL && value->ToString() != NULL) openFence = value->ToString()->Clone();
  else openFence = NULL;
  delete value;

  value = GetAttributeValue(ATTR_CLOSE, env);
  if (value != NULL && value->ToString() != NULL) closeFence = value->ToString()->Clone();
  else closeFence = NULL;
  delete value;

  value = GetAttributeValue(ATTR_SEPARATORS, env);
  if (value != NULL && value->ToString() != NULL) separators = value->ToString()->Clone();
  else separators = NULL;
  delete value;

  DelayedNormalize();

  MathMLBinContainerElement::Setup(env);
}

void
MathMLFencedElement::DelayedNormalize()
{
  if (!normalized)
    {
#if defined(HAVE_GMETADOM)
      ChildList children(GetDOMElement(), MATHML_NS_URI, "*");
      unsigned nChildren = children.get_length();

      for (unsigned i = 0; i < nChildren; i++)
	{
	  GMetaDOM::Node node = children.item(i);
	  assert(node.get_nodeType() == GMetaDOM::Node::ELEMENT_NODE);
	  Ptr<MathMLElement> elem = MathMLElement::getRenderingInterface(node);
	  assert(elem != 0);
	  // we detach the element from its parent, which can be an
	  // element created by mfenced when it expanded
	  elem->SetParent(0);
	}
#endif // HAVE_GMETADOM

      Ptr<MathMLRowElement> mainRow = smart_cast<MathMLRowElement>(MathMLRowElement::create());
      assert(mainRow != 0);

      Ptr<MathMLRowElement> mrow = 0;
      Ptr<MathMLOperatorElement> fence = 0;

      if (openFence != 0 && openFence->GetLength() > 0)
	{
	  fence = smart_cast<MathMLOperatorElement>(MathMLOperatorElement::create());
	  assert(fence != 0);
	  fence->Append(openFence);
	  fence->SetFence();
	  mainRow->Append(fence);
	}

#if defined(HAVE_GMETADOM)
      bool moreArguments = nChildren > 1;

      if (moreArguments) mrow = smart_cast<MathMLRowElement>(MathMLRowElement::create());
      else mrow = mainRow;
      assert(mrow != 0);

      for (unsigned i = 0; i < nChildren; i++)
	{
	  GMetaDOM::Node node = children.item(i);
	  assert(node.get_nodeType() == GMetaDOM::Node::ELEMENT_NODE);
	  Ptr<MathMLElement> arg = MathMLElement::getRenderingInterface(node);
	  assert(arg != 0);

	  mrow->Append(arg);

	  if (separators != NULL && separators->GetLength() > 0 && i < nChildren - 1)
	    {
	      unsigned offset = (i < separators->GetLength()) ? i : separators->GetLength() - 1;
	      const String* sep = allocString(*separators, offset, 1);
	      assert(sep != NULL);

	      Ptr<MathMLOperatorElement> separator = smart_cast<MathMLOperatorElement>(MathMLOperatorElement::create());
	      assert(separator != 0);

	      separator->SetSeparator();
	      separator->Append(sep);
	      mrow->Append(separator);
	    }
	}

      if (moreArguments) mainRow->Append(mrow);
#endif // HAVE_GMETADOM

      if (closeFence != NULL && closeFence->GetLength() > 0)
	{
	  fence = smart_cast<MathMLOperatorElement>(MathMLOperatorElement::create());
	  assert(fence != 0);
	  fence->Append(closeFence);
	  fence->SetFence();
	  mainRow->Append(fence);
	}

      mainRow->Normalize();
      SetChild(mainRow);

      normalized = true;
    }
}
