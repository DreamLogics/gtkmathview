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

#include "MathEngine.hh"
#include "StringUnicode.hh"
#include "RenderingEnvironment.hh"
#include "MathMLEncloseElement.hh"
#include "MathMLRadicalElement.hh"

#if defined(HAVE_MINIDOM)
MathMLEncloseElement::MathMLEncloseElement(mDOMNodeRef node)
#elif defined(HAVE_GMETADOM)
MathMLEncloseElement::MathMLEncloseElement(const GMetaDOM::Element& node)
#endif
  : MathMLNormalizingContainerElement(node, TAG_MENCLOSE)
{
  normalized = false;
  notation = 0;
}

MathMLEncloseElement::~MathMLEncloseElement()
{
  if (notation)
    {
      delete notation;
      notation = 0;
    }
}

const AttributeSignature*
MathMLEncloseElement::GetAttributeSignature(AttributeId id) const
{
  static AttributeSignature sig[] = {
    { ATTR_NOTATION, stringParser, new StringC("longdiv"), NULL },
    { ATTR_NOTVALID, NULL,         NULL,                   NULL }
  };

  const AttributeSignature* signature = GetAttributeSignatureAux(id, sig);
  if (signature == NULL) signature = MathMLElement::GetAttributeSignature(id);

  return signature;
}

void
MathMLEncloseElement::NormalizeRadicalElement()
{
  assert(content.GetSize() == 1);
  assert(content.GetFirst() != NULL);

  MathMLElement* child = content.RemoveFirst();

  MathMLContainerElement* sqrt = new MathMLRadicalElement(NULL, TAG_MSQRT);
  sqrt->content.Append(child);
  child->SetParent(sqrt);
  sqrt->SetParent(this);
  sqrt->Normalize();

  content.Append(sqrt);
}

void
MathMLEncloseElement::Setup(RenderingEnvironment* env)
{
  assert(env != NULL);

  if (notation) delete notation;
  const Value* value = GetAttributeValue(ATTR_NOTATION, env);
  assert(value != NULL);
  notation = value->ToString()->Clone();
  delete value;

  spacing = env->ToScaledPoints(env->GetMathSpace(MATH_SPACE_MEDIUM));
  lineThickness = env->GetRuleThickness();
  color = env->GetColor();

  if (!normalized) {
    if (notation->Equal("radical")) NormalizeRadicalElement();
    normalized = true;
  }

  MathMLContainerElement::Setup(env);
}

void
MathMLEncloseElement::DoBoxedLayout(LayoutId id, BreakId, scaled availWidth)
{
  if (!HasDirtyLayout(id, availWidth)) return;

  assert(content.GetSize() == 1);
  assert(content.GetFirst() != NULL);

  MathMLNormalizingContainerElement::DoBoxedLayout(id, BREAK_NO, availWidth);
  box = content.GetFirst()->GetBoundingBox();

  if (notation->Equal("actuarial") || notation->Equal("longdiv")) {
    box = content.GetFirst()->GetBoundingBox();
    box.ascent += spacing + lineThickness;
    box.width += spacing + lineThickness;
  }

  ConfirmLayout(id);

  ResetDirtyLayout(id, availWidth);
}

void
MathMLEncloseElement::SetPosition(scaled x, scaled y)
{
  assert(content.GetSize() == 1);
  assert(content.GetFirst() != NULL);

  position.x = x;
  position.y = y;

  if (notation->Equal("longdiv"))
    content.GetFirst()->SetPosition(x + spacing + lineThickness, y);
  else
    content.GetFirst()->SetPosition(x, y);
}

void
MathMLEncloseElement::Render(const DrawingArea& area)
{
  if (!HasDirtyChildren()) return;

  assert(content.GetSize() == 1);
  assert(content.GetFirst() != NULL);

  MathMLNormalizingContainerElement::Render(area);

  if (fGC[IsSelected()] == NULL) {
    GraphicsContextValues values;
    values.foreground = IsSelected() ? area.GetSelectionForeground() : color;
    values.lineWidth = lineThickness;
    fGC[IsSelected()] = area.GetGC(values, GC_MASK_FOREGROUND | GC_MASK_LINE_WIDTH);
  }

  if (notation->Equal("longdiv")) {
    area.MoveTo(GetX() + lineThickness / 2, GetY() + box.descent);
    area.DrawLineTo(fGC[IsSelected()], GetX() + lineThickness / 2, GetY() - box.ascent + lineThickness / 2);
    area.DrawLineTo(fGC[IsSelected()], GetX() + box.width, GetY() - box.ascent + lineThickness / 2);
  } else if (notation->Equal("actuarial")) {
    area.MoveTo(GetX(), GetY() - box.ascent + lineThickness / 2);
    area.DrawLineTo(fGC[IsSelected()], GetX() + box.width - lineThickness / 2, GetY() - box.ascent + lineThickness / 2);
    area.DrawLineTo(fGC[IsSelected()], GetX() + box.width - lineThickness / 2, GetY() + box.descent);
  } else if (notation->Equal("horizontalstrike")) {
    area.MoveTo(GetX(), GetY() - box.ascent / 2);
    area.DrawLineTo(fGC[IsSelected()], GetX() + box.width, GetY() - box.ascent / 2);
  } else if (notation->Equal("verticalstrike")) {
    area.MoveTo(GetX() + box.width / 2, GetY() - box.ascent);
    area.DrawLineTo(fGC[IsSelected()], GetX() + box.width / 2, GetY() + box.descent);
  } else if (notation->Equal("updiagonalstrike")) {
    area.MoveTo(GetX(), GetY() + box.descent);
    area.DrawLineTo(fGC[IsSelected()], GetX() + box.width, GetY() - box.ascent);
  } else if (notation->Equal("downdiagonalstrike")) {
    area.MoveTo(GetX(), GetY() - box.ascent);
    area.DrawLineTo(fGC[IsSelected()], GetX() + box.width, GetY() + box.descent);
  } else
    MathEngine::logger(LOG_WARNING, "notation `%s' not supported for menclose element (ignored)", notation->ToStaticC());

  ResetDirty();
}
