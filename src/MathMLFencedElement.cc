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
  : MathMLLinearContainerElement(node)
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
  if (signature == NULL) signature = MathMLLinearContainerElement::GetAttributeSignature(id);

  return signature;
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

  if (!normalized) {
    NormalizeFencedElement();
    normalized = true;
  }

  MathMLLinearContainerElement::Setup(env);
}

void
MathMLFencedElement::Normalize()
{
  //MathMLContainerElement::Normalize();
}

void
MathMLFencedElement::NormalizeFencedElement()
{
  Ptr<MathMLRowElement> mainRow = smart_cast<MathMLRowElement>(MathMLRowElement::create());
  assert(mainRow != 0);

  Ptr<MathMLRowElement> mrow = 0;
  Ptr<MathMLOperatorElement> fence = 0;

  if (openFence != 0 && openFence->GetLength() > 0) {
    fence = smart_cast<MathMLOperatorElement>(MathMLOperatorElement::create());
    assert(fence != 0);

    fence->Append(openFence);
    fence->SetFence();
    fence->SetParent(mainRow);
    mainRow->Append(fence);
  }

  bool moreArguments = content.GetSize() > 1;

  if (moreArguments) mrow = smart_cast<MathMLRowElement>(MathMLRowElement::create());
  else mrow = mainRow;
  assert(mrow != 0);

  unsigned i = 0;
  while (content.GetSize() > 0) {
    Ptr<MathMLElement> arg = content.RemoveFirst();
    assert(arg != 0);

    arg->SetParent(mrow);
    mrow->Append(arg);

    if (separators != NULL
	&& separators->GetLength() > 0
	&& content.GetSize() > 0) {
      unsigned offset = (i < separators->GetLength()) ? i : separators->GetLength() - 1;
      const String* sep = allocString(*separators, offset, 1);
      assert(sep != NULL);

      Ptr<MathMLOperatorElement> separator = smart_cast<MathMLOperatorElement>(MathMLOperatorElement::create());
      assert(separator != 0);

      separator->SetSeparator();
      separator->Append(sep);
      separator->SetParent(mrow);
      mrow->Append(separator);
    }

    i++;
  }

  if (moreArguments) {
    mrow->SetParent(mainRow);
    mainRow->Append(mrow);
  }

  if (closeFence != NULL && closeFence->GetLength() > 0) {
    fence = smart_cast<MathMLOperatorElement>(MathMLOperatorElement::create());
    assert(fence != 0);

    fence->Append(closeFence);
    fence->SetFence();
    fence->SetParent(mainRow);
    mainRow->Append(fence);
  }

  mainRow->Normalize();

  assert(content.GetSize() == 0);
  mainRow->SetParent(this);
  Append(mainRow);
}

bool
MathMLFencedElement::IsBreakable() const
{
  return true;
}
