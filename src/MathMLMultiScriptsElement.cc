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

#include "Iterator.hh"
#include "MathMLDummyElement.hh"
#include "RenderingEnvironment.hh"
#include "MathMLMultiScriptsElement.hh"

#if defined(HAVE_MINIDOM)
MathMLMultiScriptsElement::MathMLMultiScriptsElement(mDOMNodeRef node)
#elif defined(HAVE_GMETADOM)
MathMLMultiScriptsElement::MathMLMultiScriptsElement(const GMetaDOM::Element& node)
#endif
  : MathMLContainerElement(node, TAG_MMULTISCRIPTS)
{
}

MathMLMultiScriptsElement::~MathMLMultiScriptsElement()
{
}

void
MathMLMultiScriptsElement::Normalize()
{
  if (content.GetSize() == 0 ||
      (content.GetFirst() != NULL && content.GetFirst()->IsA() == TAG_NONE) ||
      (content.GetFirst() != NULL && content.GetFirst()->IsA() == TAG_MPRESCRIPTS)) {
    MathMLElement* mdummy = new MathMLDummyElement();
    mdummy->SetParent(this);
    content.AddFirst(mdummy);
  }

  base = content.GetFirst();
  assert(base != NULL);

  unsigned i = 0;
  bool     preScripts = false;

  for (Iterator<MathMLElement*> elem(content); elem.More(); elem.Next(), i++) {
    assert(elem() != NULL);
    if (elem()->IsA() == TAG_MPRESCRIPTS) {
      preScripts = true;
      continue;
    }

    if (elem()->IsA() != TAG_MPRESCRIPTS && elem()->IsA() != TAG_NONE)
      elem()->Normalize();
  }

  if (preScripts) {
    nPre  = content.GetSize() - i - 1;
    nPost = content.GetSize() - nPre - 2;
  } else {
    nPre  = 0;
    nPost = content.GetSize() - 1;
  }
}

void
MathMLMultiScriptsElement::Setup(RenderingEnvironment* env)
{
  assert(content.GetSize() > 0);
  assert(content.GetFirst() != NULL);

  ScriptSetup(env);

  content.GetFirst()->Setup(env);
  
  env->Push();
  env->AddScriptLevel(1);
  env->SetDisplayStyle(false);

  Iterator<MathMLElement*> elem(content);
  elem.Next();
  while (elem.More()) {
    assert(elem() != NULL);
    elem()->Setup(env);
    elem.Next();
  }

  env->Drop();
}

void
MathMLMultiScriptsElement::DoBoxedLayout(LayoutId id, BreakId, scaled availWidth)
{
  if (!HasDirtyLayout(id, availWidth)) return;

  assert(base != NULL);

  unsigned n = 1 + nPre / 2 + nPost / 2;
  assert(n > 0);

  base->DoBoxedLayout(id, BREAK_NO, availWidth / n);

  BoundingBox subScriptBox;
  BoundingBox superScriptBox;

  subScriptBox.Null();
  superScriptBox.Null();

  scaled totalWidth = 0;
  scaled subScriptWidth = 0;
  bool preScript = false;
  unsigned i = 0;
  Iterator<MathMLElement*> elem(content);
  elem.Next();

  while (elem.More()) {
    assert(elem() != NULL);

    elem()->DoBoxedLayout(id, BREAK_NO, availWidth / n);

    if (elem()->IsA() == TAG_MPRESCRIPTS) {
      preScript = true;
      i = 0;
    } else {
      if (i % 2 == 0) {
	const BoundingBox& scriptBox = elem()->GetBoundingBox();
	subScriptBox.Append(scriptBox);
	subScriptWidth = scriptBox.width;
      } else {
	const BoundingBox& scriptBox = elem()->GetBoundingBox();
	superScriptBox.Append(scriptBox);
	totalWidth += scaledMax(subScriptWidth, scriptBox.width);
      }
      
      i++;
    }

    elem.Next();
  }

  const BoundingBox& baseBox = base->GetBoundingBox();
  DoScriptLayout(baseBox, subScriptBox, superScriptBox,
		 subShiftX, subShiftY, superShiftX, superShiftY);

  box = baseBox;
  box.width = scaledMax(subShiftX, superShiftX) + totalWidth;

  if (!subScriptBox.IsNull()) {
    box.ascent  = scaledMax(box.ascent, subScriptBox.ascent - subShiftY);
    box.tAscent  = scaledMax(box.tAscent, subScriptBox.tAscent - subShiftY);
    box.descent = scaledMax(box.descent, subScriptBox.descent + subShiftY);
    box.tDescent = scaledMax(box.tDescent, subScriptBox.tDescent + subShiftY);
  }

  if (!superScriptBox.IsNull()) {
    box.ascent  = scaledMax(box.ascent, superScriptBox.ascent + superShiftY);
    box.tAscent  = scaledMax(box.tAscent, superScriptBox.tAscent + superShiftY);
    box.descent = scaledMax(box.descent, superScriptBox.descent - superShiftY);
    box.tDescent = scaledMax(box.tDescent, superScriptBox.tDescent - superShiftY);
  }

  ConfirmLayout(id);

  ResetDirtyLayout(id, availWidth);
}

void
MathMLMultiScriptsElement::SetPosition(scaled x, scaled y)
{
  position.x = x;
  position.y = y;

  Iterator<MathMLElement*> elem(content);
  elem.More();

  scaled subScriptWidth = 0;
  bool preScript = false;
  if (nPre > 0) {
    while (elem.More()) {
      assert(elem() != NULL);

      if (preScript) {
	MathMLElement* subScript = elem();
	elem.Next();

	MathMLElement* superScript = 0;	
	if (elem.More()) superScript = elem();

	scaled subScriptWidth = subScript ? subScript->GetBoundingBox().width : 0;
	scaled superScriptWidth = superScript ? superScript->GetBoundingBox().width : 0;
	scaled scriptWidth = scaledMax(subScriptWidth, superScriptWidth);

	if (subScript != 0)
	  subScript->SetPosition(x + scriptWidth - subScriptWidth, y + subShiftY);
	if (superScript != 0)
	  superScript->SetPosition(x + scriptWidth - superScriptWidth, y - superShiftY);

// 	if (i % 2 == 0) {
// 	  const BoundingBox& scriptBox = elem()->GetBoundingBox();
// 	  subScriptWidth = scriptBox.width;
// 	  elem()->SetPosition(x, y + subShiftY);
// 	} else {
// 	  const BoundingBox& scriptBox = elem()->GetBoundingBox();
// 	  elem()->SetPosition(x, y - superShiftY);
// 	  x += scaledMax(subScriptWidth, scriptBox.width);
// 	}

	x += scriptWidth;
      } else if (elem()->IsA() == TAG_MPRESCRIPTS) {
	preScript = true;
      }

      elem.Next();
    }
  }

  unsigned i = 0;

  base->SetPosition(x, y);

  if (nPost > 0) {
    x += scaledMax(subShiftX, superShiftX);

    elem.ResetFirst();
    elem.Next();

    subScriptWidth = 0;
    preScript = false;
    i = 0;
    
    while (elem.More() && !preScript) {
      assert(elem() != NULL);

      if (elem()->IsA() == TAG_MPRESCRIPTS) preScript = true;
      else {
	if (i % 2 == 0) {
	  const BoundingBox& scriptBox = elem()->GetBoundingBox();
	  subScriptWidth = scriptBox.width;
	  elem()->SetPosition(x, y + subShiftY);
	} else {
	  const BoundingBox& scriptBox = elem()->GetBoundingBox();
	  elem()->SetPosition(x, y - superShiftY);
	  x += scaledMax(subScriptWidth, scriptBox.width);
	}

	i++;
      }

      elem.Next();
    }
  }
}

