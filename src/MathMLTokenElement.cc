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
#include <stdlib.h>
#ifdef HAVE_WCTYPE_H
#include <wctype.h>
#endif
#ifdef HAVE_WCHAR_H
#include <wchar.h>
#endif

#include "AFont.hh"
#include "Layout.hh"
#include "frameAux.hh"
#include "Iterator.hh"
#include "stringAux.hh"
#include "Globals.hh"
#include "traverseAux.hh"
#include "ShapeFactory.hh"
#include "StringFactory.hh"
#include "allocTextNode.hh"
#include "StringUnicode.hh"
#include "MathMLMarkNode.hh"
#include "MathMLCharNode.hh"
#include "MathMLTextNode.hh"
#include "mathVariantAux.hh"
#include "ValueConversion.hh"
#include "MathMLGlyphNode.hh"
#include "MathMLSpaceNode.hh"
#include "MathMLStringNode.hh"
#include "MathMLTokenElement.hh"
#include "RenderingEnvironment.hh"
#include "MathMLOperatorElement.hh"
#include "MathMLEmbellishedOperatorElement.hh"

MathMLTokenElement::MathMLTokenElement()
{
}

#if defined(HAVE_GMETADOM)
MathMLTokenElement::MathMLTokenElement(const GMetaDOM::Element& node)
  : MathMLElement(node)
{
}
#endif

MathMLTokenElement::~MathMLTokenElement()
{
  Free();
}

void
MathMLTokenElement::Free()
{
  while (content.GetSize() > 0)
    {
      MathMLTextNode* node = content.RemoveFirst();
      assert(node != NULL);
      node->Release();
    }
}

const AttributeSignature*
MathMLTokenElement::GetAttributeSignature(AttributeId id) const
{
  static AttributeSignature sig[] = {
    { ATTR_FONTSIZE,       numberUnitParser,  NULL, NULL },
    { ATTR_FONTWEIGHT,     fontWeightParser,  NULL, NULL },
    { ATTR_FONTSTYLE,      fontStyleParser,   NULL, NULL },
    { ATTR_FONTFAMILY,     stringParser,      NULL, NULL },
    { ATTR_COLOR,          colorParser,       NULL, NULL },
    { ATTR_MATHVARIANT,    mathVariantParser, NULL, NULL },
    { ATTR_MATHSIZE,       mathSizeParser,    NULL, NULL },
    { ATTR_MATHCOLOR,      colorParser,       NULL, NULL },
    { ATTR_MATHBACKGROUND, colorParser,       NULL, NULL },
    
    { ATTR_NOTVALID,       NULL,              NULL, NULL }
  };

  const AttributeSignature* signature = GetAttributeSignatureAux(id, sig);
  if (signature == NULL) signature = MathMLElement::GetAttributeSignature(id);

  return signature;
}

void
MathMLTokenElement::Append(const String* s)
{
  assert(s != NULL);

  if (s->GetLength() == 0) return;

  MathMLTextNode* last = NULL;
  if (content.GetSize() > 0 &&
      content.GetLast() != NULL &&
      content.GetLast()->IsText()) {
    last = TO_TEXT(content.GetLast());
    assert(last != NULL);
  }

  unsigned i = 0;
  bool lastBreak = true;
  unsigned sLength = s->GetLength();
  while (i < sLength) {
    MathMLTextNode* node = NULL;

    int spacing;
    BreakId bid;
    unsigned len = isNonMarkingChar(*s, i, &spacing, &bid);
    if (len > 0) {
      if (last != NULL && last->GetBreakability() < BREAK_YES) {
	last->AddSpacing(spacing);
	last->AddBreakability(bid);
      } else
	node = MathMLSpaceNode::create(spacing, bid);
      i += len;
      lastBreak = true;
    } else if (i + 1 < sLength && isCombining(s->GetChar(i + 1))) {
      node = allocCombinedCharNode(s->GetChar(i), s->GetChar(i + 1));
      i += 2;

      if (last != NULL && !lastBreak) last->SetBreakability(BREAK_NO);
      lastBreak = false;
#if 0
    } else if (iswalnum(s->GetChar(i))) {
      unsigned start = i;
      while (i < sLength && iswalnum(s->GetChar(i))) i++;
      assert(start < i);

      const String* sText = allocString(*s, start, i - start);
      node = allocTextNode(&sText);

      if (last != NULL && !lastBreak) last->SetBreakability(BREAK_NO);
      lastBreak = false;
#endif
    } else if (!isVariant(s->GetChar(i))) {
      node = allocCharNode(s->GetChar(i));
      i++;

      if (last != NULL && !lastBreak) last->SetBreakability(BREAK_NO);
      lastBreak = false;
    } else {
      Globals::logger(LOG_WARNING, "ignoring variant modifier char U+%04x", s->GetChar(i));
      i++;
    }
    
    if (node != NULL) {
      Append(node);
      last = node;
    }
  }
}

void
MathMLTokenElement::Append(MathMLTextNode* node)
{
  assert(node != NULL);
  node->SetParent(this);
  content.Append(node);
}

void
MathMLTokenElement::Normalize()
{
  if (HasDirtyStructure() || HasChildWithDirtyStructure())
    {
      assert(GetDOMElement() != 0);

      String* sContent = NULL;
      for (GMetaDOM::Node p = GetDOMElement().get_firstChild(); 
	   p != 0;
	   p = p.get_nextSibling()) 
	{
	  switch (p.get_nodeType()) {
	  case GMetaDOM::Node::TEXT_NODE:
	    {
	      // ok, we have a chunk of text
	      GMetaDOM::DOMString content = p.get_nodeValue();
	      String* s = allocString(content);
	      assert(s != NULL);
	      
	      // white-spaces are always collapsed...
	      s->CollapseSpaces();
	      
	      // ...but spaces at the at the beginning (end) are deleted only if this
	      // is the very first (last) chunk in the token.
	      if (p.get_previousSibling() == 0) s->TrimSpacesLeft();
	      if (p.get_nextSibling() == 0) s->TrimSpacesRight();

	      Append(s);
	      delete s;
	    }
	  break;

#if 0
	  // to be rewritten or deleted
	  case GMetaDOM::Node::ENTITY_REFERENCE_NODE:
	    for (GMetaDOM::Node p = node.get_firstChild(); p != 0; p = p.get_nextSibling())
	      MathMLizeTokenContent(p, parent);
	    break;
#endif 0

	  case GMetaDOM::Node::ELEMENT_NODE:
	    {	    
	      if (p.get_namespaceURI() == MATHML_NS_URI)
		{
		  if (p.get_nodeName() == "mglyph")
		    {
		      MathMLTextNode* text = SubstituteMGlyphElement(p);
		      if (text != NULL) Append(text);
		    }
		  else if (p.get_nodeName() == "malignmark")
		    {
		      MathMLTextNode* text = SubstituteAlignMarkElement(p);
		      if (text != NULL) Append(text);
		    }
		  else
		    {
		      char* s_name = p.get_nodeName().toC();
		      Globals::logger(LOG_WARNING, "element `%s' inside token (ignored)\n", s_name);
		      delete [] s_name;
		    }
		} else
		  {
		    char* s_name = p.get_nodeName().toC();
		    Globals::logger(LOG_WARNING, "element `%s' inside token (ignored)\n", s_name);
		    delete [] s_name;
		  }
	    }
	  break;
	  
	  default:
	    break;
	  }
	}

      ResetDirtyStructure();
    }
}

void
MathMLTokenElement::Setup(RenderingEnvironment* env)
{
  assert(env != NULL);

  env->Push();

  if (IsA() == TAG_MTEXT || IsA() == TAG_MN) {
    env->SetFontMode(FONT_MODE_TEXT);
  }

  const Value* value = NULL;

  value = GetAttributeValue(ATTR_MATHSIZE, NULL, false);
  if (value != NULL) {
    if (IsSet(ATTR_FONTSIZE))
      Globals::logger(LOG_WARNING, "attribute `mathsize' overrides deprecated attribute `fontsize'");
    
    if (value->IsKeyword(KW_SMALL)) env->AddScriptLevel(1);
    else if (value->IsKeyword(KW_BIG)) env->AddScriptLevel(-1);
    else if (value->IsKeyword(KW_NORMAL)) ; // noop
    else env->SetFontSize(value->ToNumberUnit());
  } else {
    value = GetAttributeValue(ATTR_FONTSIZE, NULL, false);
    if (value != NULL) {
      Globals::logger(LOG_WARNING, "the attribute `fontsize' is deprecated in MathML 2");
      env->SetFontSize(value->ToNumberUnit());
    }
  }
  delete value;
  
  value = GetAttributeValue(ATTR_MATHVARIANT, NULL, false);
  if (value != NULL) {
    assert(value->IsKeyword());

    const MathVariantAttributes& attr = attributesOfVariant(value->ToKeyword());
    assert(attr.kw != KW_NOTVALID);
    env->SetFontFamily(attr.family);
    env->SetFontWeight(attr.weight);
    env->SetFontStyle(attr.style);

    if (IsSet(ATTR_FONTFAMILY) || IsSet(ATTR_FONTWEIGHT) || IsSet(ATTR_FONTSTYLE))
      Globals::logger(LOG_WARNING, "attribute `mathvariant' overrides deprecated font-related attributes");

    delete value;
  } else {
    value = GetAttributeValue(ATTR_FONTFAMILY, NULL, false);
    if (value != NULL) {
      Globals::logger(LOG_WARNING, "the attribute `fontfamily' is deprecated in MathML 2");
      env->SetFontFamily(value->ToString());
    }
    delete value;

    value = GetAttributeValue(ATTR_FONTWEIGHT, NULL, false);
    if (value != NULL) {
      Globals::logger(LOG_WARNING, "the attribute `fontweight' is deprecated in MathML 2");
      env->SetFontWeight(ToFontWeightId(value));
    }
    delete value;

    value = GetAttributeValue(ATTR_FONTSTYLE, NULL, false);
    if (value != NULL) {
      Globals::logger(LOG_WARNING, "the attribute `fontstyle' is deprecated in MathML 2");
      env->SetFontStyle(ToFontStyleId(value));
    } else if (IsA() == TAG_MI) {
      if (GetLogicalContentLength() == 1) {
	MathMLTextNode* node = content.GetFirst();
	assert(node != NULL);

	if (node->IsChar()) {
	  MathMLCharNode* cNode = TO_CHAR(node);
	  assert(cNode != NULL);

	  if (!isUpperCaseGreek(cNode->GetChar())) env->SetFontStyle(FONT_STYLE_ITALIC);
	  else env->SetFontStyle(FONT_STYLE_NORMAL);
	} else
	  env->SetFontStyle(FONT_STYLE_NORMAL);
      } else {
	env->SetFontStyle(FONT_STYLE_NORMAL);
	env->SetFontMode(FONT_MODE_TEXT);
      }
    }
    delete value;
  }

  value = GetAttributeValue(ATTR_MATHCOLOR, NULL, false);
  if (value != NULL) {
    if (IsSet(ATTR_COLOR))
      Globals::logger(LOG_WARNING, "attribute `mathcolor' overrides deprecated attribute `color'");
    env->SetColor(ToRGB(value));
  } else {
    value = GetAttributeValue(ATTR_COLOR, NULL, false);
    if (value != NULL) {
      Globals::logger(LOG_WARNING, "attribute `color' is deprecated in MathML 2");
      env->SetColor(ToRGB(value));
    } else
      if (HasLink()) env->SetColor(Globals::configuration.GetLinkForeground());
  }
  delete value;

  value = GetAttributeValue(ATTR_MATHBACKGROUND, NULL, false);
  if (value != NULL) env->SetBackgroundColor(ToRGB(value));
  else if (HasLink()) env->SetBackgroundColor(Globals::configuration.GetLinkBackground());
  delete value;

  color      = env->GetColor();
  background = env->GetBackgroundColor();
  sppm       = env->GetScaledPointsPerEm();

  for (Iterator<MathMLTextNode*> p(content); p.More(); p.Next()) {
    assert(p() != NULL);
    p()->Setup(env);
  }

  env->Drop();
}

void
MathMLTokenElement::DoLayout(LayoutId id, Layout& layout)
{
  Iterator<MathMLTextNode*> i(content);
  while (i.More()) {
    assert(i() != NULL);
    MathMLTextNode* text = i();
    assert(text != NULL);

    scaled spacing = (sppm * text->GetSpacing()) / 18;
    BreakId breakability = text->GetBreakability();
    
    // the breakability after the token will be set by the
    // surrounding context
    if (i.IsLast()) breakability = BREAK_NO;
    else if (breakability == BREAK_AUTO) breakability = BREAK_GOOD;
    
    // ok, we do the actual layout of the chunk only if we are
    // doing minimum layout. In all the other cases the layout is exactly the same
    // as the minimum layout, that we have previously done,
    // so we save some work.
    if (id == LAYOUT_MIN) text->DoLayout();

    // if we do not insert MathMLSpaceNodes in the layout, they will not be
    // positioned correctly, since positioning is done thru the layout.
    // In such way, If a space node is the first inside a token, it will produce
    // a zero-origin rectangle which is obviously incorrect
    if (text->IsSpace()) layout.SetLastBreakability(breakability);
    layout.Append(text, spacing, breakability);
#if 0
    if (!text->IsSpace())
      layout.Append(text, spacing, breakability);
    else {
      layout.SetLastBreakability(breakability);
      layout.Append(spacing, breakability);
    }
#endif

    i.Next();
  }

  AddItalicCorrection(layout);

  ResetDirtyLayout(id);
}

void
MathMLTokenElement::Freeze()
{
  if (HasLayout()) MathMLElement::Freeze();
  else {
    if (shape != NULL) delete shape;
    ShapeFactory shapeFactory;
    for (Iterator<MathMLTextNode*> i(content); i.More(); i.Next()) {
      assert(i() != NULL);

      Rectangle* rect = new Rectangle;
      getFrameBoundingBox(i(), LAYOUT_AUTO).ToRectangle(i()->GetX(), i()->GetY(), *rect);
      shapeFactory.Add(rect);
      if (i()->IsLast()) shapeFactory.SetNewRow();
    }
    shape = shapeFactory.GetShape();
  }
}

void
MathMLTokenElement::Render(const DrawingArea& area)
{
  if (!HasDirtyChildren()) return;

  RenderBackground(area);

  if (fGC[IsSelected()] == NULL) {
    GraphicsContextValues values;

    values.foreground = IsSelected() ? area.GetSelectionForeground() : color;
    values.background = IsSelected() ? area.GetSelectionBackground() : background;
    fGC[IsSelected()] = area.GetGC(values, GC_MASK_FOREGROUND | GC_MASK_BACKGROUND);
  }

  for (Iterator<MathMLTextNode*> i(content); i.More(); i.Next()) {
    assert(i() != NULL);
    i()->Render(area);
  }

  //area.DrawRectangle(fGC[0], *shape);

  ResetDirty();
}

void
MathMLTokenElement::GetLinearBoundingBox(BoundingBox& b) const
{
  b.Null();
  for (Iterator<MathMLTextNode*> i(content); i.More(); i.Next()) {
    assert(i() != NULL);
    const BoundingBox& textBox = i()->GetBoundingBox();
    b.Append(textBox);
  }
}

bool
MathMLTokenElement::IsToken() const
{
  return true;
}

bool
MathMLTokenElement::IsBreakable() const
{
  return true;
}

BreakId
MathMLTokenElement::GetBreakability() const
{
  // we have to skip some empty elements (malignmark) and get
  // the breakability of the last text node

  Iterator<MathMLTextNode*> text(content);
  text.ResetLast();
  while (text.More()) {
    assert(text() != NULL);
    if (!text()->IsMark()) break;
    text.Prev();
  }

  return text.More() ? text()->GetBreakability() : BREAK_AUTO;
}

void
MathMLTokenElement::SetDirty(const Rectangle* rect)
{
  assert(IsShaped());

  dirtyBackground =
    (GetParent() != NULL && (GetParent()->IsSelected() != IsSelected())) ? 1 : 0;

  if (IsDirty()) return;
  if (rect != NULL && !shape->Overlaps(*rect)) return;

  dirty = 1;
  SetDirtyChildren();

  for (Iterator<MathMLTextNode*> text(content); text.More(); text.Next()) {
    assert(text() != NULL);
    text()->SetDirty(rect);
  }
}

scaled
MathMLTokenElement::GetLeftEdge() const
{
  scaled edge = 0;

  for (Iterator<MathMLTextNode*> text(content); text.More(); text.Next()) {
    assert(text() !=  NULL);
    if (text.IsFirst()) edge = text()->GetLeftEdge();
    else edge = scaledMin(edge, text()->GetLeftEdge());
  }

  return edge;
}

scaled
MathMLTokenElement::GetRightEdge() const
{
  scaled edge = 0;

  for (Iterator<MathMLTextNode*> text(content); text.More(); text.Next()) {
    assert(text() !=  NULL);
    if (text.IsFirst()) edge = text()->GetRightEdge();
    else edge = scaledMax(edge, text()->GetRightEdge());
  }

  return edge;
}

scaled
MathMLTokenElement::GetDecimalPointEdge() const
{
  for (Iterator<MathMLTextNode*> text(content); text.More(); text.Next()) {
    assert(text() != NULL);
    if (text()->HasDecimalPoint()) return text()->GetDecimalPointEdge();
  }

  return GetRightEdge();
}

bool
MathMLTokenElement::IsNonMarking() const
{
  for (Iterator<MathMLTextNode*> text(content); text.More(); text.Next()) {
    assert(text() != NULL);
    if (!text()->IsSpace()) return false;
  }

  return true;
}

bool
MathMLTokenElement::IsLast() const
{
  if (last != 0) return true;
  if (content.GetSize() > 0) {
    assert(content.GetLast() != NULL);
    return content.GetLast()->IsLast();
  } else
    return false;
}

const MathMLCharNode*
MathMLTokenElement::GetCharNode() const
{
  if (content.GetSize() != 1) return NULL;

  MathMLTextNode* node = content.GetFirst();
  assert(node != NULL);
  if (!node->IsChar() || node->IsCombinedChar()) return NULL;

  return TO_CHAR(node);
}

void
MathMLTokenElement::AddItalicCorrection(Layout& layout)
{
  if (IsA() != TAG_MI && IsA() != TAG_MN && IsA() != TAG_MTEXT) return;
  
  if (content.GetSize() == 0) return;

  MathMLTextNode* lastNode = content.GetLast();
  assert(lastNode != NULL);

  MathMLElement* next = findRightSibling(this);
  if (next == NULL || next->IsA() != TAG_MO) return;

  MathMLOperatorElement* op = next->GetCoreOperator();
  if (op == NULL) return;
  bool isFence = op->IsFence();
  op->Release();
  if (!isFence) return;

  const BoundingBox& box = lastNode->GetBoundingBox();
  Globals::logger(LOG_DEBUG, "adding italic correction: %d %d", sp2ipx(box.rBearing), sp2ipx(box.width));
  if (box.rBearing > box.width) layout.Append(box.rBearing - box.width);
}

MathMLTextNode*
MathMLTokenElement::SubstituteMGlyphElement(const GMetaDOM::Element& node)
{
  assert(node != 0);

  GMetaDOM::DOMString alt        = node.getAttribute("alt");
  GMetaDOM::DOMString fontFamily = node.getAttribute("fontfamily");
  GMetaDOM::DOMString index      = node.getAttribute("index");

  if (alt.isEmpty() || fontFamily.isEmpty() || index.isEmpty()) {
    Globals::logger(LOG_WARNING, "malformed `mglyph' element (some required attribute is missing)\n");
    return MathMLCharNode::create('?');
  }

  char* s_index = index.toC();
  char* endPtr;
  unsigned nch = strtoul(s_index, &endPtr, 10);
  delete [] s_index;

  if (endPtr == NULL || *endPtr != '\0') {
    Globals::logger(LOG_WARNING, "malformed `mglyph' element (parsing error in `index' attribute)\n");
    nch = '?';
  }

  char* s_alt = alt.toC();
  char* s_fontFamily = fontFamily.toC();
  MathMLGlyphNode* glyph = MathMLGlyphNode::create(s_alt, s_fontFamily, nch);
  delete [] s_alt;
  delete [] s_fontFamily;

  return glyph;
}

MathMLTextNode*
MathMLTokenElement::SubstituteAlignMarkElement(const GMetaDOM::Element& node)
{
  assert(node != 0);

  GMetaDOM::DOMString edge = node.getAttribute("edge");

  MarkAlignType align = MARK_ALIGN_NOTVALID;

  if (!edge.isEmpty()) {
    if      (edge == "left") align = MARK_ALIGN_LEFT;
    else if (edge == "right") align = MARK_ALIGN_RIGHT;
    else {
      char* s_edge = edge.toC();
      Globals::logger(LOG_WARNING,
		      "malformed `malignmark' element, attribute `edge' has invalid value `%s' (ignored)",
		      s_edge);
      delete [] s_edge;
    }
  }

  return MathMLMarkNode::create(align);
}

String*
MathMLTokenElement::GetRawContent() const
{
  StringFactory c;

  for (Iterator<MathMLTextNode*> i(content); i.More(); i.Next())
    {
      assert(i() != NULL);
      String* s = i()->GetRawContent();
      if (s != NULL)
	{
	  c.Append(s);
	  delete s;
	}
    }

  return c.Pack();
}

unsigned
MathMLTokenElement::GetLogicalContentLength() const
{
  unsigned len = 0;

  for (Iterator<MathMLTextNode*> i(content); i.More(); i.Next())
    {
      assert(i() != NULL);
      len += i()->GetLogicalContentLength();
    }

  return len;
}
