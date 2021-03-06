// Copyright (C) 2000-2007, Luca Padovani <padovani@sti.uniurb.it>.
//
// This file is part of GtkMathView, a flexible, high-quality rendering
// engine for MathML documents.
// 
// GtkMathView is free software; you can redistribute it and/or modify it
// either under the terms of the GNU Lesser General Public License version
// 3 as published by the Free Software Foundation (the "LGPL") or, at your
// option, under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation (the "GPL").  If you do not
// alter this notice, a recipient may use your version of this file under
// either the GPL or the LGPL.
//
// GtkMathView is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the LGPL or
// the GPL for more details.
// 
// You should have received a copy of the LGPL and of the GPL along with
// this program in the files COPYING-LGPL-3 and COPYING-GPL-2; if not, see
// <http://www.gnu.org/licenses/>.

#ifndef __custom_reader_Builder_hh__
#define __custom_reader_Builder_hh__

#include "customXmlReader.hh"
#include "TemplateReaderBuilder.hh"
#include "TemplateLinker.hh"
#include "custom_reader_Model.hh"
#include "String.hh"
#include "Element.hh"

class custom_reader_Builder : public TemplateReaderBuilder<customXmlReader>
{
protected:
  custom_reader_Builder(void) { }
  virtual ~custom_reader_Builder() { }

public:
  static SmartPtr<custom_reader_Builder> create(void);

  SmartPtr<Element> findElement(void* p) const { return linker.assoc(p); }
  void* findSelfOrAncestorModelElement(const SmartPtr<Element>&) const;
  SmartPtr<Element> findSelfOrAncestorElement(void* p) const { return findElement(p); }

  bool notifyStructureChanged(void*);
  bool notifyAttributeChanged(void*, const char*);

protected:
  SmartPtr<Element>
  linkerAssoc(const SmartPtr<customXmlReader>& reader) const
  {
    if (void* id = reader->getNodeId())
      return linker.assoc(id);
    else
      return 0;
  }

  void
  linkerAdd(const SmartPtr<customXmlReader>& reader, Element* elem) const
  { if (void* id = reader->getNodeId()) linker.add(id, elem); }

  void linkerRemove(Element* elem) const { linker.remove(elem); }

private:
  mutable TemplateLinker<custom_reader_Model, void*> linker;
};

#endif // __custom_reader_Builder_hh__
