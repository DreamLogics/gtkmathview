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

#include <algorithm>

#include <assert.h>
#include <stddef.h>

#include "Adaptors.hh"
#include "ChildList.hh"
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
  if (DirtyStructure())
    {
#if defined(HAVE_GMETADOM)
      if (GetDOMElement())
	{
	  ChildList children(GetDOMElement(), MATHML_NS_URI, "mtd");
	  unsigned n = children.get_length();
	  content.reserve(n);
	  for (unsigned i = 0; i < n; i++)
	    {
	      GMetaDOM::Node node = children.item(i);
	      Ptr<MathMLElement> elem = MathMLElement::getRenderingInterface(node);
	      assert(elem);
	      SetChild(i, elem);
	    }
	  SetSize(n); // this should be noop
	}
#endif
      
      std::for_each(content.begin(), content.end(), NormalizeAdaptor());

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
  for (std::vector< Ptr<MathMLElement> >::iterator p = content.begin();
       p != content.end();
       p++)
    {
      assert(is_a<MathMLTableCellElement>(*p));
      Ptr<MathMLTableCellElement> mtd = smart_cast<MathMLTableCellElement>(*p);
      assert(mtd);
      mtd->SetupCellSpanning(env);
    }
}

void
MathMLTableRowElement::Setup(RenderingEnvironment* env)
{
  if (DirtyAttribute())
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
    }

  MathMLLinearContainerElement::Setup(env);

  ResetDirtyAttribute();
}

#if 0
void
MathMLTableRowElement::SetDirty(const Rectangle* rect)
{
  // this method is needed because a table row does not have a
  // valid shape, its sole purpose is to call recursively SetDirty on
  // its children
  std::for_each(content.begin(), content.end(), std::bind2nd(SetDirtyAdaptor(), rect));
}
#endif

bool
MathMLTableRowElement::IsInside(scaled x, scaled y) const
{
  // same arguments as for the SetDirty method above
  for (std::vector< Ptr<MathMLElement> >::const_iterator elem = content.begin();
       elem != content.end();
       elem++)
    {
      assert(*elem);
      if ((*elem)->IsInside(x, y)) return true;
    }

  return false;
}

Ptr<MathMLElement>
MathMLTableRowElement::GetLabel(void) const
{
  return 0;
}

