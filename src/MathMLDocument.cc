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

#include "MathML.hh"
#include "Globals.hh"
#include "MathMLDocument.hh"
#include "RenderingEnvironment.hh"

MathMLDocument::MathMLDocument()
#if defined(HAVE_GMETADOM)
  : DOMdoc(0), DOMroot(0)
#endif
{
  Init();
}

#if defined(HAVE_GMETADOM)
MathMLDocument::MathMLDocument(const GMetaDOM::Document& doc)
  : MathMLBinContainerElement()
  , DOMdoc(doc), DOMroot(0)
{
  DOMroot = DOMdoc.get_documentElement();
  Init();
}

MathMLDocument::MathMLDocument(const GMetaDOM::Element& root)
  : MathMLBinContainerElement()
  , DOMdoc(0)
  , DOMroot(root)
{
  Init();
}

void
MathMLDocument::Init()
{
  if (DOMroot)
    {
      GMetaDOM::EventTarget et(DOMroot);
      assert(et);

      subtreeModifiedListener = new DOMSubtreeModifiedListener(this);
      attrModifiedListener = new DOMAttrModifiedListener(this);

      et.addEventListener("DOMNodeRemoved", *subtreeModifiedListener, false);
      et.addEventListener("DOMAttrModified", *attrModifiedListener, false);
    }
}
#endif

MathMLDocument::~MathMLDocument()
{
#if defined(HAVE_GMETADOM)
  if (DOMroot)
    {
      GMetaDOM::EventTarget et(DOMroot);
      assert(et);

      et.removeEventListener("DOMSubtreeModified", *subtreeModifiedListener, false);
      et.removeEventListener("DOMAttrModified", *attrModifiedListener, false);

      delete subtreeModifiedListener;
      delete attrModifiedListener;
      subtreeModifiedListener = 0;
      attrModifiedListener = 0;
    }
#endif
}

void
MathMLDocument::Normalize()
{
  if (DirtyStructure())
    {
#if defined(HAVE_GMETADOM)
      GMetaDOM::NodeList nodeList = GetDOMDocument().getElementsByTagNameNS(MATHML_NS_URI, "math");
      if (GMetaDOM::Node mathRoot = nodeList.item(0))
	{
	  Ptr<MathMLElement> elem = getFormattingNode(mathRoot);
	  assert(elem);
	  SetChild(elem);
	}	  
#endif // HAVE_GMETADOM

      if (child) child->Normalize(this);

      ResetDirtyStructure();
    }
}

void
MathMLDocument::Setup(RenderingEnvironment& env)
{
  if (DirtyAttribute() || DirtyAttributeP())
    {
      env.Push();
      env.SetDocument(this);
      MathMLBinContainerElement::Setup(env);
      env.Drop();
      ResetDirtyAttribute();
    }
}

void
MathMLDocument::SetDirtyAttribute()
{
  MathMLBinContainerElement::SetDirtyAttribute();
  // changing an attribute at this level amounts at
  // invalidating the whole tree. It is the notification
  // of a change in the context, rather than in a real attribute
  SetFlagDown(FDirtyAttribute);
}

#if defined(HAVE_GMETADOM)

void
MathMLDocument::DOMSubtreeModifiedListener::handleEvent(const GMetaDOM::Event& ev)
{
  const GMetaDOM::MutationEvent& me(ev);
  assert(me);
  assert(doc);
  if (Ptr<MathMLElement> elem = doc->findFormattingNode(me.get_relatedNode()))
    {
      printf("notifying event\n");
      elem->SetDirtyStructure();
    }
}

void
MathMLDocument::DOMAttrModifiedListener::handleEvent(const GMetaDOM::Event& ev)
{
  const GMetaDOM::MutationEvent& me(ev);
  assert(me);
  printf("an attribute changed\n");
}

#if 0
void
MathMLDocument::RegisterElement(const Ptr<MathMLElement>& elem)
{
  assert(elem);
  assert(elem->GetDOMElement());
  std::pair<std::hash_map< void*, Ptr<MathMLElement> >::iterator, bool> res =
    nodeMap.insert(elem->GetDOMElement().id());
  assert(!res.second);
  *(res.first) = elem;
}

void
MathMLDocument::UnregisterElement(const Ptr<MathMLElement>& elem)
{
  assert(elem);
  assert(elem->GetDOMElement());
  unsigned res = nodeMap.erase(elem->GetDOMElement().id());
  assert(res == 1);
}
#endif

Ptr<MathMLElement>
MathMLDocument::getFormattingNodeNoCreate(const GMetaDOM::Element& el) const
{
  assert(el);

  DOMNodeMap::iterator p = nodeMap.find(el);
  if (p != nodeMap.end()) return (*p).second;
  else return 0;
}

Ptr<MathMLElement>
MathMLDocument::findFormattingNode(const GMetaDOM::Element& el) const
{
  for (GMetaDOM::Element p = el; p; p = p.get_parentNode())
    if (Ptr<MathMLElement> fNode = getFormattingNodeNoCreate(p))
      return fNode;
  
  return 0;
}

Ptr<MathMLElement>
MathMLDocument::getFormattingNode(const GMetaDOM::Element& el) const
{
  assert(el);

  DOMNodeMap::iterator p = nodeMap.find(el);
  if (p != nodeMap.end()) return (*p).second;

  std::string s_tag = nodeLocalName(el);

  TagId tag = TagIdOfName(s_tag.c_str());

  if (tag == TAG_NOTVALID)
    {
      Globals::logger(LOG_WARNING, "skipping unrecognized element");
      return 0;
    }

  static struct
  {
    TagId tag;
    Ptr<MathMLElement> (*create)(const GMetaDOM::Element&);
  } tab[] = {
    { TAG_MATH,          &MathMLmathElement::create },
    { TAG_MI,            &MathMLIdentifierElement::create },
    { TAG_MN,            &MathMLTokenElement::create },
    { TAG_MO,            &MathMLOperatorElement::create },
    { TAG_MTEXT,         &MathMLTextElement::create },
    { TAG_MSPACE,        &MathMLSpaceElement::create },
    { TAG_MS,            &MathMLStringLitElement::create },
    { TAG_MROW,          &MathMLRowElement::create },
    { TAG_MFRAC,         &MathMLFractionElement::create },
    { TAG_MSQRT,         &MathMLRadicalElement::create },
    { TAG_MROOT,         &MathMLRadicalElement::create },
    { TAG_MSTYLE,        &MathMLStyleElement::create },
    { TAG_MERROR,        &MathMLErrorElement::create },
    { TAG_MPADDED,       &MathMLPaddedElement::create },
    { TAG_MPHANTOM,      &MathMLPhantomElement::create },
    { TAG_MFENCED,       &MathMLFencedElement::create },
    { TAG_MSUB,          &MathMLScriptElement::create },
    { TAG_MSUP,          &MathMLScriptElement::create },
    { TAG_MSUBSUP,       &MathMLScriptElement::create },
    { TAG_MUNDER,        &MathMLUnderOverElement::create },
    { TAG_MOVER,         &MathMLUnderOverElement::create },
    { TAG_MUNDEROVER,    &MathMLUnderOverElement::create },
    { TAG_MMULTISCRIPTS, &MathMLMultiScriptsElement::create },
    { TAG_MTABLE,        &MathMLTableElement::create },
    { TAG_MTR,           &MathMLTableRowElement::create },
    { TAG_MLABELEDTR,    &MathMLTableRowElement::create },
    { TAG_MTD,           &MathMLTableCellElement::create },
    { TAG_MALIGNGROUP,   &MathMLAlignGroupElement::create },
    { TAG_MALIGNMARK,    &MathMLAlignMarkElement::create },
    { TAG_MACTION,       &MathMLActionElement::create },
    { TAG_MENCLOSE,      &MathMLEncloseElement::create },
    { TAG_SEMANTICS,     &MathMLSemanticsElement::create },

    { TAG_NOTVALID,      0 }
  };

  unsigned i;
  for (i = 0; tab[i].tag != TAG_NOTVALID && tab[i].tag != tag; i++) ;
  assert(tab[i].create != 0);

  Ptr<MathMLElement> res = tab[i].create(el);
  assert(res);

  nodeMap[el] = res;

  return res;
}

#endif // HAVE_GMETADOM
