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

#include "Iterator.hh"
#include "Globals.hh"
#include "StringUnicode.hh"
#include "ValueConversion.hh"
#include "MathMLDummyElement.hh"
#include "MathMLTableElement.hh"
#include "MathMLTableRowElement.hh"
#include "MathMLTableCellElement.hh"
#include "MathMLLabeledTableRowElement.hh"

MathMLTableRowElement::MathMLTableRowElement()
{
  rowIndex = 0;
}

#if defined(HAVE_GMETADOM)
MathMLTableRowElement::MathMLTableRowElement(const GMetaDOM::Element& node)
  : MathMLLinearContainerElement(node)
{
  rowIndex = 0;
}
#endif

MathMLTableRowElement::~MathMLTableRowElement()
{
}

const AttributeSignature*
MathMLTableRowElement::GetAttributeSignature(AttributeId id) const
{
  static AttributeSignature sig[] = {
    //{ ATTR_ROWALIGN,        rowAlignListParser,       NULL },
    { ATTR_ROWALIGN,        rowAlignParser,           NULL, NULL },
    { ATTR_COLUMNALIGN,     columnAlignListParser,    NULL, NULL },
    { ATTR_GROUPALIGN,      groupAlignListListParser, NULL, NULL },

    { ATTR_NOTVALID,        NULL,                     NULL, NULL }
  };

  const AttributeSignature* signature = GetAttributeSignatureAux(id, sig);
  if (signature == NULL) signature = MathMLElement::GetAttributeSignature(id);

  return signature;
}

void
MathMLTableRowElement::Normalize()
{
  if (HasDirtyStructure() || HasChildWithDirtyStructure())
    {
      MathMLLinearContainerElement::Normalize();

      for (unsigned i = 0; i < content.GetSize(); i++)
	{
	  Ptr<MathMLElement> elem = content.RemoveFirst();
	  assert(elem);
	  
	  // if this is a labeled row, then the first child is always the label
	  // because of normalization (see above)
	  if (!is_a<MathMLTableCellElement>(elem) &&
	      (is_a<MathMLLabeledTableRowElement>(Ptr<MathMLElement>(this)) || i > 0))
	    {
	      Ptr<MathMLTableCellElement> inferredTableCell =
		smart_cast<MathMLTableCellElement>(MathMLTableCellElement::create());
	      assert(inferredTableCell);
	      inferredTableCell->SetParent(this);
	      inferredTableCell->SetChild(elem);
	      elem = inferredTableCell;
	    }
	  elem->Normalize();

	  content.Append(elem);
	}

      ResetDirtyStructure();
    }
}

void
MathMLTableRowElement::SetupRowIndex(unsigned i)
{
  rowIndex = i;
}

void
MathMLTableRowElement::SetupCellSpanning(RenderingEnvironment* env)
{
  for (Iterator< Ptr<MathMLElement> > p(content); p.More(); p.Next())
    {
      assert(p());

      if (IsA() == TAG_MTR || p() != content.GetFirst())
	{
	  assert(p()->IsA() == TAG_MTD);

	  Ptr<MathMLTableCellElement> mtd = smart_cast<MathMLTableCellElement>(p());
	  assert(mtd);

	  mtd->SetupCellSpanning(env);
	}
    }
}

void
MathMLTableRowElement::Setup(RenderingEnvironment* env)
{
  assert(GetParent());
  Ptr<MathMLTableElement> mtable = smart_cast<MathMLTableElement>(GetParent());
  assert(mtable);

  const Value* value;

  value = GetAttributeValue(ATTR_COLUMNALIGN, NULL, false);
  if (value != NULL) mtable->SetupColumnAlignAux(value, rowIndex, 1, IsA() == TAG_MLABELEDTR);

  value = GetAttributeValue(ATTR_ROWALIGN, NULL, false);
  if (value != NULL) mtable->SetupRowAlignAux(value, rowIndex, IsA() == TAG_MLABELEDTR);

  value = GetAttributeValue(ATTR_GROUPALIGN, NULL, false);
  if (value != NULL) mtable->SetupGroupAlignAux(value, rowIndex, 1);

  MathMLLinearContainerElement::Setup(env);
}

void
MathMLTableRowElement::SetDirty(const Rectangle* rect)
{
  // this function is needed because a table row does not have a
  // valid shape, its sole purpose is to call recursively SetDirty on
  // its children
  for (Iterator< Ptr<MathMLElement> > elem(content); elem.More(); elem.Next())
    {
      assert(elem());
      elem()->SetDirty(rect);
    }
}

bool
MathMLTableRowElement::IsInside(scaled x, scaled y) const
{
  // same arguments as for the SetDirty method above
  for (Iterator< Ptr<MathMLElement> > elem(content); elem.More(); elem.Next())
    {
      assert(elem());
      if (elem()->IsInside(x, y)) return true;
    }

  return false;
}

Ptr<MathMLElement>
MathMLTableRowElement::GetLabel(void) const
{
  return 0;
}

