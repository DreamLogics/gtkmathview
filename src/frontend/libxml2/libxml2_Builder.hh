// Copyright (C) 2000-2004, Luca Padovani <luca.padovani@cs.unibo.it>.
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
// http://helm.cs.unibo.it/mml-widget/, or send a mail to
// <lpadovan@cs.unibo.it>

#ifndef __libxml2_Builder_hh__
#define __libxml2_Builder_hh__

#include "libxml2_Model.hh"
#include "TemplateLinker.hh"
#include "Builder.hh"
#include "String.hh"
#include "Element.hh"

class libxml2_Builder : public Builder
{
protected:
  libxml2_Builder(void) : root(0) { }
  virtual ~libxml2_Builder();

public:
  static SmartPtr<libxml2_Builder> create(void);

  void setRootModelElement(xmlElement*);
  xmlElement* getRootModelElement(void) const { return root; }

  SmartPtr<Element> findElement(xmlElement* p) const { return linker.assoc(p); }
  xmlElement* findSelfOrAncestorModelElement(const SmartPtr<Element>&) const;
  SmartPtr<Element> findSelfOrAncestorElement(xmlElement*) const;

  bool notifyStructureChanged(xmlElement*);
  bool notifyAttributeChanged(xmlElement*, const xmlChar*);

protected:
  // methods for accessing the linker
  SmartPtr<Element> linkerAssoc(xmlElement* el) const { return linker.assoc(el); }
  void linkerAdd(xmlElement* el, Element* elem) const { linker.add(el, elem); }
  void linkerRemove(Element* elem) const { linker.remove(elem); }

private:
  mutable TemplateLinker<libxml2_Model> linker;
  xmlElement* root;
};

#endif // __libxml2_Builder_hh__