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

#include "stringAux.hh"
#include "Globals.hh"
#include "StringUnicode.hh"
#include "mathVariantAux.hh"
#include "ValueConversion.hh"
#include "MathMLAttribute.hh"
#include "MathMLStyleElement.hh"
#include "MathMLAttributeList.hh"
#include "RenderingEnvironment.hh"

MathMLStyleElement::MathMLStyleElement()
{
}

#if defined(HAVE_GMETADOM)
MathMLStyleElement::MathMLStyleElement(const GMetaDOM::Element& node)
  : MathMLNormalizingContainerElement(node)
{
}
#endif

MathMLStyleElement::~MathMLStyleElement()
{
}

const AttributeSignature*
MathMLStyleElement::GetAttributeSignature(AttributeId id) const
{
  static AttributeSignature sig[] = {
    { ATTR_SCRIPTLEVEL,            scriptLevelParser, NULL,                  NULL },
    { ATTR_DISPLAYSTYLE,           booleanParser,     NULL,                  NULL },
    { ATTR_SCRIPTSIZEMULTIPLIER,   numberParser,      NULL,                  NULL },
    { ATTR_SCRIPTMINSIZE,          numberUnitParser,  NULL,                  NULL },
    { ATTR_COLOR,                  colorParser,       NULL,                  NULL },
    { ATTR_BACKGROUND,             backgroundParser,  NULL,                  NULL },
    { ATTR_VERYVERYTHINMATHSPACE,  numberUnitParser,  NULL,                  NULL },
    { ATTR_VERYTHINMATHSPACE,      numberUnitParser,  NULL,                  NULL },
    { ATTR_THINMATHSPACE,          numberUnitParser,  NULL,                  NULL },
    { ATTR_MEDIUMMATHSPACE,        numberUnitParser,  NULL,                  NULL },
    { ATTR_THICKMATHSPACE,         numberUnitParser,  NULL,                  NULL },
    { ATTR_VERYTHICKMATHSPACE,     numberUnitParser,  NULL,                  NULL },
    { ATTR_VERYVERYTHICKMATHSPACE, numberUnitParser,  NULL,                  NULL },
    // inherited attributes (see below)
    { ATTR_FONTSIZE,               numberUnitParser,  NULL,                  NULL },
    { ATTR_FONTWEIGHT,             fontWeightParser,  NULL,                  NULL },
    { ATTR_FONTSTYLE,              fontStyleParser,   new StringC("normal"), NULL },
    { ATTR_FONTFAMILY,             stringParser,      NULL,                  NULL },
    { ATTR_MATHVARIANT,            mathVariantParser, NULL,                  NULL },
    { ATTR_MATHSIZE,               mathSizeParser,    NULL,                  NULL },
    { ATTR_MATHCOLOR,              colorParser,       NULL,                  NULL },
    { ATTR_MATHBACKGROUND,         colorParser,       NULL,                  NULL },
    //
    { ATTR_NOTVALID,               NULL,              NULL,                  NULL }
  };

  const AttributeSignature* signature = GetAttributeSignatureAux(id, sig);
  if (signature == NULL) signature = MathMLElement::GetAttributeSignature(id);

  return signature;
}

void
MathMLStyleElement::Setup(RenderingEnvironment* env)
{
  assert(env != NULL);

  MathMLAttributeList attributes;

#if defined(HAVE_MINIDOM)
  for (mDOMAttrRef attribute = mdom_node_get_first_attribute(GetDOMElement());
       attribute != NULL;
       attribute = mdom_attr_get_next_sibling(attribute)) {
    AttributeId id = AttributeIdOfName(C_CONST_STRING(mdom_attr_get_name(attribute)));
    if (id != ATTR_NOTVALID) {
      mDOMStringRef value = mdom_attr_get_value(attribute);
      String* sValue = allocString(value);
      attributes.Append(new MathMLAttribute(id, sValue));
      mdom_string_free(value);
    }
  }
#elif defined(HAVE_GMETADOM)
  GMetaDOM::NamedNodeMap nnm = GetDOMElement().get_attributes();

  for (unsigned i = 0; i < nnm.get_length(); i++) {
    GMetaDOM::Node attribute = nnm.item(i);

    char* s_name = attribute.get_nodeName().toC();
    AttributeId id = AttributeIdOfName(s_name);
    delete [] s_name;

    if (id != ATTR_NOTVALID) {
      GMetaDOM::DOMString value = attribute.get_nodeValue();
      String* sValue = allocString(value);
      attributes.Append(new MathMLAttribute(id, sValue));
    }
  }
#endif // HAVE_GMETADOM

  env->Push(&attributes);

  const Value* value = NULL;

  value = GetAttributeValue(ATTR_DISPLAYSTYLE, NULL, false);
  if (value != NULL) env->SetDisplayStyle(value->ToBoolean());
  delete value;

  value = GetAttributeValue(ATTR_SCRIPTSIZEMULTIPLIER, NULL, false);
  if (value != NULL) env->SetScriptSizeMultiplier(value->ToNumber());
  delete value;

  value = GetAttributeValue(ATTR_SCRIPTMINSIZE, NULL, false);
  if (value != NULL) env->SetScriptMinSize(value->ToNumberUnit());
  delete value;

  value = GetAttributeValue(ATTR_SCRIPTLEVEL, NULL, false);
  if (value != NULL) {
    const Value* p = value->Get(0);
    assert(p != NULL);

    if (p->IsEmpty()) {
      p = value->Get(1);
      assert(p != NULL);
      
      int scriptLevel = p->ToInteger();
      if (scriptLevel < 0) scriptLevel = 0;
      env->SetScriptLevel(scriptLevel);
    } else {
      int sign = 1;
      if (p->IsKeyword(KW_PLUS)) sign = 1;
      else sign = -1;
      p = value->Get(1);
      assert(p != NULL);
      
      int scriptLevel = p->ToInteger();
      if (scriptLevel < 0) scriptLevel = 0;
      env->AddScriptLevel(sign * scriptLevel);
    }
  }
  delete value;

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
    }
  }
  delete value;

  RGBValue oldBackground = env->GetBackgroundColor();
  value = GetAttributeValue(ATTR_MATHBACKGROUND, NULL, false);
  if (value != NULL) {
    if (IsSet(ATTR_BACKGROUND))
      Globals::logger(LOG_WARNING, "attribute `mathbackground' overrides deprecated attribute `background'");
    if (!value->IsKeyword(KW_TRANSPARENT)) env->SetBackgroundColor(ToRGB(value));
  } else {
    value = GetAttributeValue(ATTR_BACKGROUND, NULL, false);
    if (value != NULL) {
      Globals::logger(LOG_WARNING, "attribute `background' is deprecated in MathML 2");
      if (!value->IsKeyword(KW_TRANSPARENT)) env->SetBackgroundColor(ToRGB(value));
    }
  }
  delete value;
  background = env->GetBackgroundColor();
  differentBackground = background != oldBackground;

  value = GetAttributeValue(ATTR_VERYVERYTHINMATHSPACE, NULL, false);
  if (value != NULL) env->SetMathSpace(MATH_SPACE_VERYVERYTHIN, value->ToNumberUnit());
  delete value;

  value = GetAttributeValue(ATTR_VERYTHINMATHSPACE, NULL, false);
  if (value != NULL) env->SetMathSpace(MATH_SPACE_VERYTHIN, value->ToNumberUnit());
  delete value;

  value = GetAttributeValue(ATTR_THINMATHSPACE, NULL, false);
  if (value != NULL) env->SetMathSpace(MATH_SPACE_THIN, value->ToNumberUnit());
  delete value;

  value = GetAttributeValue(ATTR_MEDIUMMATHSPACE, NULL, false);
  if (value != NULL) env->SetMathSpace(MATH_SPACE_MEDIUM, value->ToNumberUnit());
  delete value;

  value = GetAttributeValue(ATTR_THICKMATHSPACE, NULL, false);
  if (value != NULL) env->SetMathSpace(MATH_SPACE_THICK, value->ToNumberUnit());
  delete value;

  value = GetAttributeValue(ATTR_VERYTHINMATHSPACE, NULL, false);
  if (value != NULL) env->SetMathSpace(MATH_SPACE_VERYTHICK, value->ToNumberUnit());
  delete value;

  value = GetAttributeValue(ATTR_VERYVERYTHICKMATHSPACE, NULL, false);
  if (value != NULL) env->SetMathSpace(MATH_SPACE_VERYVERYTHICK, value->ToNumberUnit());
  delete value;

  // the following attributes, thought not directly supported by <mstyle>
  // must be parsed here since they are always inherited by other elements

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
      Globals::logger(LOG_WARNING, "the attribute `fontfamily` is deprecated in MathML 2");
      env->SetFontFamily(value->ToString());
    }
    delete value;

    value = GetAttributeValue(ATTR_FONTWEIGHT, NULL, false);
    if (value != NULL) {
      Globals::logger(LOG_WARNING, "the attribute `fontweight` is deprecated in MathML 2");
      env->SetFontWeight(ToFontWeightId(value));
    }
    delete value;

    value = GetAttributeValue(ATTR_FONTSTYLE, NULL, false);
    if (value != NULL) {
      Globals::logger(LOG_WARNING, "the attribute `fontstyle` is deprecated in MathML 2");
      env->SetFontStyle(ToFontStyleId(value));
    }
    delete value;
  }

  MathMLNormalizingContainerElement::Setup(env);

  env->Drop();

  while (attributes.GetSize() > 0) {
    MathMLAttribute* attr = attributes.RemoveFirst();
    delete attr;
  }
}

void
MathMLStyleElement::Render(const DrawingArea& area)
{
  if (!HasDirtyChildren()) return;
 
  if (IsDirty()) {
    if (differentBackground && !IsSelected()) {
      if (bGC[0] == NULL) {
	GraphicsContextValues values;
	values.foreground = values.background = background;
	bGC[0] = area.GetGC(values, GC_MASK_FOREGROUND | GC_MASK_BACKGROUND);
      }

      area.Clear(bGC[0], GetRectangle());
    }
  }

  RenderBackground(area);

  assert(child != 0);
  child->Render(area);

  ResetDirty();
}

bool
MathMLStyleElement::IsSpaceLike() const
{
  assert(child != 0);
  return child->IsSpaceLike();
}


