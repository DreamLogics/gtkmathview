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
#include <stdio.h>

#include "MathML.hh"
#include "Layout.hh"
#include "stringAux.hh"
#include "Globals.hh"
#include "traverseAux.hh"
#include "DrawingArea.hh"
#include "MathMLElement.hh"
#include "MathMLDocument.hh"
#include "ValueConversion.hh"
#include "MathMLStyleElement.hh"
#include "MathMLAttributeList.hh"
#include "RenderingEnvironment.hh"
#include "FormattingContext.hh"

#ifdef DEBUG
int MathMLElement::counter = 0;
#endif // DEBUG

MathMLElement::MathMLElement()
#if defined(HAVE_GMETADOM)
  : node(0)
#endif
{
  Init();
}

// MathMLElement: this is the base class for every MathML presentation element.
// It implements the basic skeleton of every such element, moreover it handles
// the attributes and provides some facility functions to access and parse
// attributes.
#if defined(HAVE_GMETADOM)
MathMLElement::MathMLElement(const GMetaDOM::Element& n)
  : node(n)
{
  if (node)
    {
      Ptr<MathMLElement> elem = ::getRenderingInterface(node);
      if (!elem) ::setRenderingInterface(node, this);
    }

  Init();
}
#endif

void
MathMLElement::Init()
{
  dirtyStructure = childWithDirtyStructure = 1;
  dirtyAttribute = childWithDirtyAttribute = 1;

  fGC[0] = fGC[1] = NULL;
  bGC[0] = bGC[1] = NULL;
}

MathMLElement::~MathMLElement()
{
#if defined(HAVE_GMETADOM)
  if (node)
    {
      Ptr<MathMLElement> elem = ::getRenderingInterface(node);
      if (elem == this) ::setRenderingInterface(node, 0);
    }
#endif

  ReleaseGCs();
}

#if defined(HAVE_GMETADOM)
Ptr<MathMLElement>
MathMLElement::getRenderingInterface(const GMetaDOM::Element& el)
{
  assert(el);

  Ptr<MathMLElement> elem = ::getRenderingInterface(el);
  if (elem) return elem;

  std::string s_tag;
  if (!el.get_namespaceURI().null())
    s_tag = el.get_nodeName();
  else
    s_tag = el.get_localName();

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

  return tab[i].create(el);
}
#endif // HAVE_GMETADOM

// GetAttributeSignatureAux: this is an auxiliary function used to retrieve
// the signature of the attribute with id ID given an array of attribute
// signatures.
const AttributeSignature*
MathMLElement::GetAttributeSignatureAux(AttributeId id,
					AttributeSignature sig[]) const
{
  for (unsigned i = 0; sig[i].GetAttributeId() != ATTR_NOTVALID; i++)
    if (sig[i].GetAttributeId() == id) return &sig[i];

  return NULL;
}

// GetAttributeSignature: return the attribute signature of ID.
// Here are the attributes common to all (presentation) elements of MathML
const AttributeSignature*
MathMLElement::GetAttributeSignature(AttributeId id) const
{
  static AttributeSignature sig[] = {
    { ATTR_ID,         NULL, NULL, NULL },
    { ATTR_CLASS,      NULL, NULL, NULL },
    { ATTR_OTHER,      NULL, NULL, NULL },

    { ATTR_NOTVALID,   NULL, NULL, NULL }
  };

  return GetAttributeSignatureAux(id, sig);
}

const String*
MathMLElement::GetDefaultAttribute(AttributeId id) const
{
  const AttributeSignature* aSignature = GetAttributeSignature(id);
  assert(aSignature != NULL);
  return aSignature->GetDefaultValue();
}

const Value*
MathMLElement::GetDefaultAttributeValue(AttributeId id) const
{
  const AttributeSignature* aSignature = GetAttributeSignature(id);
  assert(aSignature != NULL);
  return aSignature->GetDefaultParsedValue();
}

const String*
MathMLElement::GetAttribute(AttributeId id,
			    const RenderingEnvironment* env,
			    bool searchDefault) const
{
  const String* sValue = NULL;

  // if this element is not connected with a DOM element
  // then it cannot have attributes. This may happen for
  // elements inferred with normalization
#if defined(HAVE_MINIDOM)
  if (node != NULL) {
    mDOMStringRef value = mdom_node_get_attribute(node, DOM_CONST_STRING(NameOfAttributeId(id)));
    if (value != NULL) {
      sValue = allocString(value);
      mdom_string_free(value);
    }
  }
#elif defined(HAVE_GMETADOM)
  if (node) {
    GMetaDOM::GdomeString value = node.getAttribute(NameOfAttributeId(id));
    if (!value.empty()) sValue = allocString(value);
  }
#endif // HAVE_GMETADOM

  if (sValue == NULL && env != NULL) {
    const MathMLAttribute* attr = env->GetAttribute(id);
    if (attr != NULL) sValue = attr->GetValue();
  }

  if (sValue == NULL && searchDefault) sValue = GetDefaultAttribute(id);

  return sValue;
}

const Value*
MathMLElement::GetAttributeValue(AttributeId id,
				 const RenderingEnvironment* env,
				 bool searchDefault) const
{
  const Value* value = NULL;

  const AttributeSignature* aSignature = GetAttributeSignature(id);
  assert(aSignature != NULL);

  const String* sValue = NULL;

#if defined(HAVE_MINIDOM)
  if (node != NULL) {
    mDOMStringRef value = mdom_node_get_attribute(node,
						  DOM_CONST_STRING(NameOfAttributeId(id)));
    if (value != NULL) {
      sValue = allocString(value);
      mdom_string_free(value);
    }
  }
#elif defined(HAVE_GMETADOM)
  if (node) {
    GMetaDOM::GdomeString value = node.getAttribute(NameOfAttributeId(id));
    if (!value.empty()) sValue = allocString(value);
  }
#endif // HAVE_GMETADOM

  if (sValue != NULL) {
    AttributeParser parser = aSignature->GetParser();
    assert(parser != NULL);

    StringTokenizer st(*sValue);
    value = parser(st);

    if (value == NULL) {
      Globals::logger(LOG_WARNING, "in element `%s' parsing error in attribute `%s'",
			 NameOfTagId(IsA()), NameOfAttributeId(id));
    }

    delete sValue;
    sValue = NULL;
  } else if (env != NULL) {
    const MathMLAttribute* attr = env->GetAttribute(id);    
    if (attr != NULL) value = attr->GetParsedValue(aSignature);
  }

  if (value == NULL && searchDefault) value = GetDefaultAttributeValue(id);

  return value;
}

const Value*
MathMLElement::Resolve(const Value* value,
		       const RenderingEnvironment* env,
		       int i, int j)
{
  assert(value != NULL);
  assert(env != NULL);

  const Value* realValue = value->Get(i, j);
  assert(realValue != NULL);

  if      (realValue->IsKeyword(KW_VERYVERYTHINMATHSPACE))
    realValue = new Value(env->GetMathSpace(MATH_SPACE_VERYVERYTHIN));
  else if (realValue->IsKeyword(KW_VERYTHINMATHSPACE))
    realValue = new Value(env->GetMathSpace(MATH_SPACE_VERYTHIN));
  else if (realValue->IsKeyword(KW_THINMATHSPACE))
    realValue = new Value(env->GetMathSpace(MATH_SPACE_THIN));
  else if (realValue->IsKeyword(KW_MEDIUMMATHSPACE))
    realValue = new Value(env->GetMathSpace(MATH_SPACE_MEDIUM));
  else if (realValue->IsKeyword(KW_THICKMATHSPACE))
    realValue = new Value(env->GetMathSpace(MATH_SPACE_THICK));
  else if (realValue->IsKeyword(KW_VERYTHICKMATHSPACE))
    realValue = new Value(env->GetMathSpace(MATH_SPACE_VERYTHICK));
  else if (realValue->IsKeyword(KW_VERYVERYTHICKMATHSPACE))
    realValue = new Value(env->GetMathSpace(MATH_SPACE_VERYVERYTHICK));
  else
    // the following cloning is necessary because values returned by
    // the resolving function must always be deleted (never cached)
    realValue = new Value(*realValue);

  return realValue;
}

bool
MathMLElement::IsSet(AttributeId id) const
{
#if defined(HAVE_MINIDOM)
  if (node == NULL) return false;

  mDOMStringRef value = mdom_node_get_attribute(node, DOM_CONST_STRING(NameOfAttributeId(id)));

  if (value != NULL) {
    mdom_string_free(value);
    return true;
  }

  return false;
#elif defined(HAVE_GMETADOM)
  if (!node) return false;
  return node.hasAttribute(NameOfAttributeId(id));
#endif // HAVE_GMETADOM
}

void
MathMLElement::Setup(RenderingEnvironment* env)
{
  if (HasDirtyAttribute() || HasChildWithDirtyAttribute())
    {
      background = env->GetBackgroundColor();
      ResetDirtyAttribute();
    }
}

void
MathMLElement::DoLayout(const FormattingContext& ctxt)
{
  if (HasDirtyLayout(ctxt)) ResetDirtyLayout(ctxt);
}

void
MathMLElement::DoStretchyLayout()
{
}

void
MathMLElement::RenderBackground(const DrawingArea& area)
{
  if (bGC[IsSelected()] == NULL)
    {
      GraphicsContextValues values;
      values.background = values.foreground = IsSelected() ? area.GetSelectionBackground() : background;
      bGC[IsSelected()] = area.GetGC(values, GC_MASK_FOREGROUND | GC_MASK_BACKGROUND);
    }

  if (HasDirtyBackground())
    area.Clear(bGC[IsSelected()], GetX(), GetY(), GetBoundingBox());
}

void
MathMLElement::Render(const DrawingArea& area)
{
  if (!IsDirty()) return;
  RenderBackground(area);
  ResetDirty();
}

void
MathMLElement::SetDirty(const Rectangle* rect)
{
  dirtyBackground =
    (GetParent() && (GetParent()->IsSelected() != IsSelected())) ? 1 : 0;
#if 0
  if (GetParent() && (GetParent()->IsSelected() != IsSelected()))
    dirtyBackground = 1;
#endif

  if (IsDirty()) return;
  if (rect != NULL && !GetRectangle().Overlaps(*rect)) return;

  dirty = 1;
  SetDirtyChildren();
}

bool
MathMLElement::IsSpaceLike() const
{
  return false;
}

bool
MathMLElement::IsInside(scaled x, scaled y) const
{
  return GetRectangle().IsInside(x, y);
}

Ptr<MathMLElement>
MathMLElement::Inside(scaled x, scaled y)
{
  return IsInside(x, y) ? this : 0;
}

unsigned
MathMLElement::GetDepth() const
{
  unsigned depth = 0;
  Ptr<const MathMLElement> p = this;
  
  while (p)
    {
      depth++;
      p = p->GetParent();
    }

  return depth;
}

scaled
MathMLElement::GetLeftEdge() const
{
  return GetX();
}

scaled
MathMLElement::GetRightEdge() const
{
  return GetX();
}

void
MathMLElement::ReleaseGCs()
{
  fGC[0] = fGC[1] = NULL;
  bGC[0] = bGC[1] = NULL;
}

bool
MathMLElement::HasLink() const
{
#if defined(HAVE_MINIDOM)
  mDOMNodeRef p = GetDOMElement();

  while (p != NULL && !mdom_node_has_attribute(p, DOM_CONST_STRING("href")))
    p = mdom_node_get_parent(p);

  return p != NULL;
#elif defined(HAVE_GMETADOM)
  GMetaDOM::Element p = GetDOMElement();

  while (p && !p.hasAttribute("href")) {
    GMetaDOM::Node parent = p.get_parentNode();
    p = parent;
  }

  return p;
#endif // HAVE_GMETADOM
}

Ptr<MathMLOperatorElement>
MathMLElement::GetCoreOperator()
{
  // it's not clear whether this should be an abstract method, since
  // many elements share this trivial implementation
  return 0;
}

TagId
MathMLElement::IsA() const
{
  if (!node) return TAG_NOTVALID;

  std::string s_tag = node.get_nodeName();
  TagId res = TagIdOfName(s_tag.c_str());

  return res;
}

void
MathMLElement::SetDirtyStructure()
{
  dirtyStructure = 1;
  
  Ptr<MathMLElement> parent = GetParent();
  while (parent)
    {
      parent->childWithDirtyStructure = 1;
      parent = parent->GetParent();
    }
}

void
MathMLElement::SetDirtyAttribute()
{
  dirtyAttribute = 1;

  Ptr<MathMLElement> parent = GetParent();
  while (parent)
    {
      parent->childWithDirtyAttribute = 1;
      parent = parent->GetParent();
    }
}
