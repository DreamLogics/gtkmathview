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
#include <stddef.h>

#include "Iterator.hh"
#include "Globals.hh"
#include "StringUnicode.hh"
#include "ValueConversion.hh"
#include "MathMLDummyElement.hh"
#include "MathMLTableElement.hh"
#include "MathMLTableRowElement.hh"
#include "MathMLTableCellElement.hh"
#include "MathMLLabeledTableRowElement.hh"

MathMLLabeledTableRowElement::MathMLLabeledTableRowElement()
{
}

#if defined(HAVE_GMETADOM)
MathMLLabeledTableRowElement::MathMLLabeledTableRowElement(const GMetaDOM::Element& node)
  : MathMLTableRowElement(node)
{
}
#endif

MathMLLabeledTableRowElement::~MathMLLabeledTableRowElement()
{
}

void
MathMLLabeledTableRowElement::Normalize()
{
  if (HasDirtyStructure() || HasChildWithDirtyStructure())
    {
      MathMLTableRowElement::Normalize();

      if (content.GetSize() == 0 ||
	  (content.GetSize() > 0 &&
	   content.GetFirst() != 0 &&
	   is_a<MathMLTableCellElement>(content.GetFirst())))
	{
	  Ptr<MathMLElement> mdummy = MathMLDummyElement::create();
	  assert(mdummy != 0);
	  mdummy->SetParent(this);
	  content.AddFirst(mdummy);
	  Globals::logger(LOG_WARNING, "`mlabeledtr' element without label (dummy label added)");
	}

      ResetDirtyStructure();
    }
}

Ptr<MathMLElement>
MathMLLabeledTableRowElement::GetLabel(void) const
{
  assert(content.GetSize() > 0);
  assert(content.GetFirst() != 0);
  assert(!is_a<MathMLTableCellElement>(content.GetFirst()));

  return content.GetFirst();
}

