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
#include "MathMLCombinedCharNode.hh"
#include "MathMLTextNode.hh"
#include "mathVariantAux.hh"
#include "ValueConversion.hh"
#include "MathMLGlyphNode.hh"
#include "MathMLSpaceNode.hh"
#include "MathMLStringNode.hh"
#include "MathMLTextElement.hh"
#include "MathMLTokenElement.hh"
#include "MathMLNumberElement.hh"
#include "RenderingEnvironment.hh"
#include "MathMLOperatorElement.hh"
#include "MathMLIdentifierElement.hh"
#include "MathMLEmbellishedOperatorElement.hh"
#include "FormattingContext.hh"

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

  Ptr<MathMLTextNode> last = 0;
  if (content.GetSize() > 0 &&
      content.GetLast() != 0 &&
      content.GetLast()->IsText())
    {
      last = smart_cast<MathMLTextNode>(content.GetLast());
      assert(last != 0);
    }

  unsigned i = 0;
  unsigned sLength = s->GetLength();
  while (i < sLength)
    {
      Ptr<MathMLTextNode> node = 0;

      int spacing;
      unsigned len = isNonMarkingChar(*s, i, &spacing);
      if (len > 0)
	{
	  node = MathMLSpaceNode::create(spacing);
	  i += len;
	} 
      else if (i + 1 < sLength && isCombining(s->GetChar(i + 1)))
	{
	  node = allocCombinedCharNode(s->GetChar(i), s->GetChar(i + 1));
	  i += 2;
#if 0
	}
      else if (iswalnum(s->GetChar(i)))
	{
	  unsigned start = i;
	  while (i < sLength && iswalnum(s->GetChar(i))) i++;
	  assert(start < i);

	  const String* sText = allocString(*s, start, i - start);
	  node = allocTextNode(&sText);
#endif
	}
      else if (!isVariant(s->GetChar(i)))
	{
	  node = allocCharNode(s->GetChar(i));
	  i++;
	}
      else
	{
	  Globals::logger(LOG_WARNING, "ignoring variant modifier char U+%04x", s->GetChar(i));
	  i++;
	}
    
      if (node != 0)
	{
	  Append(node);
	  last = node;
	}
    }
}

void
MathMLTokenElement::Append(const Ptr<MathMLTextNode>& node)
{
  assert(node != 0);
  node->SetParent(this);
  content.Append(node);
}

void
MathMLTokenElement::Normalize()
{
  if ((HasDirtyStructure() || HasChildWithDirtyStructure()) && GetDOMElement() != 0)
    {
      String* sContent = NULL;
      for (GMetaDOM::Node p = GetDOMElement().get_firstChild(); 
	   p != 0;
	   p = p.get_nextSibling()) 
	{
	  switch (p.get_nodeType())
	    {
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
			Ptr<MathMLTextNode> text = SubstituteMGlyphElement(p);
			if (text != 0) Append(text);
		      }
		    else if (p.get_nodeName() == "malignmark")
		      {
			Ptr<MathMLTextNode> text = SubstituteAlignMarkElement(p);
			if (text != 0) Append(text);
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

  if (!is_a<MathMLIdentifierElement>(Ptr<MathMLElement>(this)) &&
      !is_a<MathMLOperatorElement>(Ptr<MathMLElement>(this)))
    env->SetFontMode(FONT_MODE_TEXT);

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
    } else if (is_a<MathMLIdentifierElement>(Ptr<MathMLElement>(this))) {
      if (GetLogicalContentLength() == 1) {
	Ptr<MathMLTextNode> node = content.GetFirst();
	assert(node != 0);

	if (is_a<MathMLCharNode>(node)) {
	  Ptr<MathMLCharNode> cNode = smart_cast<MathMLCharNode>(node);
	  assert(cNode != 0);

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

  for (Iterator< Ptr<MathMLTextNode> > p(content); p.More(); p.Next())
    {
      assert(p() != 0);
      p()->Setup(env);
    }

  env->Drop();
}

void
MathMLTokenElement::DoLayout(const class FormattingContext& ctxt)
{
  if (!HasDirtyLayout()) return;

  box.Null();
  for (Iterator< Ptr<MathMLTextNode> > text(content); text.More(); text.Next())
    {
      assert(text() != 0);

      // as the minimum layout, that we have previously done,
      // so we save some work.
      if (ctxt.GetLayoutType() == LAYOUT_MIN) text()->DoLayout(ctxt);

      // if we do not insert MathMLSpaceNodes in the layout, they will not be
      // positioned correctly, since positioning is done thru the layout.
      // In such way, If a space node is the first inside a token, it will produce
      // a zero-origin rectangle which is obviously incorrect
      box.Append(text()->GetBoundingBox());
      box.Append((sppm * text()->GetSpacing()) / 18);
    }

  AddItalicCorrection();

  ResetDirtyLayout(ctxt.GetLayoutType());
}

void
MathMLTokenElement::SetPosition(scaled x, scaled y)
{
  MathMLElement::SetPosition(x, y);
  for (Iterator< Ptr<MathMLTextNode> > text(content); text.More(); text.Next())
    {
      assert(text() != 0);
      text()->SetPosition(x, y);
      x += text()->GetBoundingBox().width;
      x += (sppm * text()->GetSpacing()) / 18;
    }
}

void
MathMLTokenElement::Render(const DrawingArea& area)
{
  if (!HasDirtyChildren()) return;

  RenderBackground(area);

  if (fGC[IsSelected()] == NULL)
    {
      GraphicsContextValues values;

      values.foreground = IsSelected() ? area.GetSelectionForeground() : color;
      values.background = IsSelected() ? area.GetSelectionBackground() : background;
      fGC[IsSelected()] = area.GetGC(values, GC_MASK_FOREGROUND | GC_MASK_BACKGROUND);
    }

  for (Iterator< Ptr<MathMLTextNode> > text(content); text.More(); text.Next())
    {
      assert(text() != 0);
      text()->Render(area);
    }

  //area.DrawRectangle(fGC[0], *shape);

  ResetDirty();
}

void
MathMLTokenElement::SetDirty(const Rectangle* rect)
{
  dirtyBackground =
    (GetParent() != 0 && 
     ((GetParent()->IsSelected() != IsSelected()) ||
      (GetParent()->GetBackgroundColor() != GetBackgroundColor()))) ? 1 : 0;

  if (IsDirty()) return;
  if (rect != NULL && !GetRectangle().Overlaps(*rect)) return;

  dirty = 1;
  SetDirtyChildren();

  for (Iterator< Ptr<MathMLTextNode> > text(content); text.More(); text.Next())
    {
      assert(text() != 0);
      text()->SetDirty(rect);
    }
}

scaled
MathMLTokenElement::GetLeftEdge() const
{
  scaled edge = 0;

  for (Iterator< Ptr<MathMLTextNode> > text(content); text.More(); text.Next())
    {
      assert(text() !=  0);
      if (text.IsFirst()) edge = text()->GetLeftEdge();
      else edge = scaledMin(edge, text()->GetLeftEdge());
    }

  return edge;
}

scaled
MathMLTokenElement::GetRightEdge() const
{
  scaled edge = 0;

  for (Iterator< Ptr<MathMLTextNode> > text(content); text.More(); text.Next())
    {
      assert(text() !=  0);
      if (text.IsFirst()) edge = text()->GetRightEdge();
      else edge = scaledMax(edge, text()->GetRightEdge());
    }

  return edge;
}

scaled
MathMLTokenElement::GetDecimalPointEdge() const
{
  for (Iterator< Ptr<MathMLTextNode> > text(content); text.More(); text.Next())
    {
      assert(text() != 0);
      if (text()->HasDecimalPoint()) return text()->GetDecimalPointEdge();
    }

  return GetRightEdge();
}

bool
MathMLTokenElement::IsNonMarking() const
{
  for (Iterator< Ptr<MathMLTextNode> > text(content); text.More(); text.Next())
    {
      assert(text() != 0);
      if (!is_a<MathMLSpaceNode>(text())) return false;
    }

  return true;
}

Ptr<MathMLCharNode>
MathMLTokenElement::GetCharNode() const
{
  if (content.GetSize() != 1) return 0;

  Ptr<MathMLTextNode> node = content.GetFirst();
  assert(node != 0);
  if (!is_a<MathMLCharNode>(node) || is_a<MathMLCombinedCharNode>(node)) return 0;

  return smart_cast<MathMLCharNode>(node);
}

void
MathMLTokenElement::AddItalicCorrection()
{
  if (!is_a<MathMLIdentifierElement>(Ptr<MathMLElement>(this)) &&
      !is_a<MathMLNumberElement>(Ptr<MathMLElement>(this)) &&
      !is_a<MathMLTextElement>(Ptr<MathMLElement>(this))) return;
  
  if (content.GetSize() == 0) return;

  Ptr<MathMLTextNode> lastNode = content.GetLast();
  assert(lastNode != 0);

  Ptr<MathMLElement> next = findRightSibling(this);
  if (next == 0 ||
      !is_a<MathMLOperatorElement>(next) ||
      !is_a<MathMLEmbellishedOperatorElement>(next)) return;

  Ptr<MathMLOperatorElement> op = next->GetCoreOperator();
  if (op == 0) return;
  bool isFence = op->IsFence();
  if (!isFence) return;

  const BoundingBox& lastBox = lastNode->GetBoundingBox();
  Globals::logger(LOG_DEBUG, "adding italic correction: %d %d", sp2ipx(box.rBearing), sp2ipx(box.width));
  if (lastBox.rBearing > lastBox.width) box.Append(lastBox.rBearing - lastBox.width);
}

Ptr<MathMLTextNode>
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
  Ptr<MathMLGlyphNode> glyph = MathMLGlyphNode::create(s_alt, s_fontFamily, nch);
  delete [] s_alt;
  delete [] s_fontFamily;

  return glyph;
}

Ptr<MathMLTextNode>
MathMLTokenElement::SubstituteAlignMarkElement(const GMetaDOM::Element& node)
{
  assert(node != 0);

  GMetaDOM::DOMString edge = node.getAttribute("edge");

  MarkAlignType align = MARK_ALIGN_NOTVALID;

  if (!edge.isEmpty())
    {
      if      (edge == "left") align = MARK_ALIGN_LEFT;
      else if (edge == "right") align = MARK_ALIGN_RIGHT;
      else
	{
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

  for (Iterator< Ptr<MathMLTextNode> > i(content); i.More(); i.Next())
    {
      assert(i() != 0);
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

  for (Iterator< Ptr<MathMLTextNode> > i(content); i.More(); i.Next())
    {
      assert(i() != 0);
      len += i()->GetLogicalContentLength();
    }

  return len;
}
