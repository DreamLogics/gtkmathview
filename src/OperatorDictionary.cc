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

#include "keyword.hh"
#include "stringAux.hh"
#include "Globals.hh"
#include "MathMLAttribute.hh"
#include "MathMLParseFile.hh"
#include "OperatorDictionary.hh"
#include "MathMLAttributeList.hh"

#if defined(HAVE_MINIDOM)

void
getAttribute(mDOMNodeRef node, const char* attr, MathMLAttributeList* aList)
{
  assert(aList != NULL);

  mDOMStringRef attrVal = mdom_node_get_attribute(node, DOM_CONST_STRING(attr));
  if (attrVal == NULL) return;

  MathMLAttribute* attribute =
    new MathMLAttribute(AttributeIdOfName(attr), allocString(attrVal));

  aList->Append(attribute);
  mdom_string_free(attrVal);
}

#elif defined(HAVE_GMETADOM)

void
getAttribute(const GMetaDOM::Element& node, const char* attr, MathMLAttributeList* aList)
{
  assert(aList != NULL);

  GMetaDOM::GdomeString attrVal = node.getAttribute(attr);
  if (attrVal.empty()) return;

  MathMLAttribute* attribute =
    new MathMLAttribute(AttributeIdOfName(attr), allocString(attrVal));

  aList->Append(attribute);
}

#endif // HAVE_GMETADOM

//

OperatorDictionary::OperatorDictionary()
{
}

OperatorDictionary::~OperatorDictionary()
{
  Unload();
}

#if defined(HAVE_MINIDOM)

bool
OperatorDictionary::Load(const char* fileName)
{
  mDOMDocRef doc = MathMLParseFile(fileName, true);
  if (doc == NULL) return false;

  mDOMNodeRef root = mdom_doc_get_root_node(doc);
  if (root == NULL) {
    mdom_doc_free(doc);
    Globals::logger(LOG_WARNING, "operator dictionary `%s': parse error", fileName);
    return false;
  }

  if (!mdom_string_eq(mdom_node_get_name(root), DOM_CONST_STRING("dictionary"))) {
    mdom_doc_free(doc);
    Globals::logger(LOG_WARNING, "operator dictionary `%s': could not find root element", fileName);
    return false;
  }

  for (mDOMNodeRef op = mdom_node_get_first_child(root);
       op != NULL;
       op = mdom_node_get_next_sibling(op)) {
    if (!mdom_node_is_blank(op)
	&& mdom_string_eq(mdom_node_get_name(op), DOM_CONST_STRING("operator"))) {
      mDOMStringRef opName = mdom_node_get_attribute(op, DOM_CONST_STRING("name"));
      if (opName != NULL) {
	const String* opString = allocString(opName);
	MathMLAttributeList* def = new MathMLAttributeList;

	getAttribute(op, "form", def);
	getAttribute(op, "fence", def);
	getAttribute(op, "separator", def);
	getAttribute(op, "lspace", def);
	getAttribute(op, "rspace", def);
#ifdef ENABLE_EXTENSIONS
	getAttribute(op, "tspace", def);
	getAttribute(op, "bspace", def);
#endif // ENABLE_EXTENSIONS
	getAttribute(op, "stretchy", def);
	getAttribute(op, "direction", def);
	getAttribute(op, "symmetric", def);
	getAttribute(op, "maxsize", def);
	getAttribute(op, "minsize", def);
	getAttribute(op, "largeop", def);
	getAttribute(op, "movablelimits", def);
	getAttribute(op, "accent", def);

	const MathMLAttributeList* defaults = AlreadyDefined(*def);
	if (defaults == NULL) defaults = def;
	else delete def;

	OperatorDictionaryItem* item = new OperatorDictionaryItem;
	item->name     = opString;
	item->defaults = defaults;

	items.AddFirst(item);
      } else {
	Globals::logger(LOG_WARNING, "operator dictionary `%s': could not find operator name", fileName);
      }
    } else if (!mdom_node_is_blank(op)) {
      Globals::logger(LOG_WARNING, "operator dictionary `%s': unknown element `%s'", fileName,
			 C_CONST_STRING(mdom_node_get_name(op)));
    }
  }

  mdom_doc_free(doc);

  return true;
}

#elif defined(HAVE_GMETADOM)

bool
OperatorDictionary::Load(const char* fileName)
{
  try {
    GMetaDOM::Document doc = MathMLParseFile(fileName, true);

    GMetaDOM::Element root = doc.get_documentElement();
    if (!root) {
      Globals::logger(LOG_WARNING, "operator dictionary `%s': parse error", fileName);
      return false;
    }

    if (root.get_nodeName() != "dictionary") {
      Globals::logger(LOG_WARNING, "operator dictionary `%s': could not find root element", fileName);
      return false;
    }

    for (GMetaDOM::Node op = root.get_firstChild(); op; op = op.get_nextSibling()) {
      if (op.get_nodeType() == GMetaDOM::Node::ELEMENT_NODE && op.get_nodeName() == "operator") {
	GMetaDOM::Element elem = op;
	GMetaDOM::GdomeString opName = elem.getAttribute("name");

	if (!opName.empty()) {
	  const String* opString = allocString(opName);
	  MathMLAttributeList* def = new MathMLAttributeList;

	  getAttribute(op, "form", def);
	  getAttribute(op, "fence", def);
	  getAttribute(op, "separator", def);
	  getAttribute(op, "lspace", def);
	  getAttribute(op, "rspace", def);
#ifdef ENABLE_EXTENSIONS
	  getAttribute(op, "tspace", def);
	  getAttribute(op, "bspace", def);
#endif // ENABLE_EXTENSIONS
	  getAttribute(op, "stretchy", def);
	  getAttribute(op, "direction", def);
	  getAttribute(op, "symmetric", def);
	  getAttribute(op, "maxsize", def);
	  getAttribute(op, "minsize", def);
	  getAttribute(op, "largeop", def);
	  getAttribute(op, "movablelimits", def);
	  getAttribute(op, "accent", def);

	  const MathMLAttributeList* defaults = AlreadyDefined(*def);
	  if (defaults == NULL) defaults = def;
	  else delete def;

	  Dictionary::const_iterator p = items.find(opString);
	  if (p == items.end())
	    {
	      FormDefaults& formDefaults = items[opString];
	      if (elem.getAttribute("form") == "prefix")
		{
		  assert(formDefaults.prefix == 0);
		  formDefaults.prefix = defaults;
		}
	      else if (elem.getAttribute("form") == "infix")
		{
		  assert(formDefaults.infix == 0);
		  formDefaults.infix = defaults;
		}
	      else if (elem.getAttribute("form") == "postfix")
		{
		  assert(formDefaults.postfix == 0);
		  formDefaults.postfix = defaults;
		}
	      else
		Globals::logger(LOG_WARNING, 
				"invalid `form' attribute for entry `%s' in operator dictionary (ignored)",
				opString->ToStaticC());
	    }
	} else {
	  Globals::logger(LOG_WARNING, "operator dictionary `%s': could not find operator name", fileName);
	}
      } else if (!GMetaDOM::nodeIsBlank(op)) {
	std::string s_name = op.get_nodeName();
	Globals::logger(LOG_WARNING, "operator dictionary `%s': unknown element `%s'", fileName, s_name.c_str());
      }
    }

    return true;
  } catch (GMetaDOM::DOMException) {
    return false;
  }
}

#endif // HAVE_GMETADOM

void
OperatorDictionary::Unload()
{
  for (AttributeListContainer::iterator p = defaults.begin();
       p != defaults.end();
       p++)
    {
      assert(*p);
      delete *p;
    }
}

const MathMLAttributeList*
OperatorDictionary::AlreadyDefined(const MathMLAttributeList& def) const
{
  for (AttributeListContainer::const_iterator p = defaults.begin();
       p != defaults.end();
       p++)
    {
      assert(*p);
      if ((*p)->Equal(def)) return *p;
    }

  return 0;
}

void
OperatorDictionary::Search(const String* opName,
			   const MathMLAttributeList** prefix,
			   const MathMLAttributeList** infix,
			   const MathMLAttributeList** postfix) const
{
  assert(opName != 0);
  assert(prefix != 0 && infix != 0 && postfix != 0);

  *prefix = *infix = *postfix = 0;

  Dictionary::const_iterator p = items.find(opName);
  if (p != items.end())
    {
      assert((*p).first != 0);
      *prefix = (*p).second.prefix;
      *infix = (*p).second.infix;
      *postfix = (*p).second.postfix;
    }
}
