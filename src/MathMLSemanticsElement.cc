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

#include "MathMLDummyElement.hh"
#include "MathMLOperatorElement.hh"
#include "MathMLSemanticsElement.hh"

MathMLSemanticsElement::MathMLSemanticsElement()
{
}

#if defined(HAVE_GMETADOM)
MathMLSemanticsElement::MathMLSemanticsElement(const GMetaDOM::Element& node)
  : MathMLLinearContainerElement(node)
{
}
#endif

MathMLSemanticsElement::~MathMLSemanticsElement()
{
}

void
MathMLSemanticsElement::Normalize()
{
  while (content.GetSize() > 1)
    content.RemoveLast();

  if (content.GetSize() == 0)
    {
      Ptr<MathMLElement> mdummy = MathMLDummyElement::create();
      assert(mdummy != 0);
      mdummy->SetParent(this);
      content.Append(mdummy);
    }

  assert(content.GetSize() == 1);
  assert(content.GetFirst() != 0);
  content.GetFirst()->Normalize();
}

bool
MathMLSemanticsElement::IsBreakable() const
{
  assert(content.GetSize() == 1);
  assert(content.GetFirst() != 0);
  return content.GetFirst()->IsBreakable();
}

bool
MathMLSemanticsElement::IsExpanding() const
{
  assert(content.GetSize() == 1);
  assert(content.GetFirst() != 0);
  return content.GetFirst()->IsExpanding();
}

Ptr<class MathMLOperatorElement>
MathMLSemanticsElement::GetCoreOperator()
{
  assert(content.GetSize() == 1);
  assert(content.GetFirst() != 0);
  return content.GetFirst()->GetCoreOperator();
}
