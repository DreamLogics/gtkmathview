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
#include "ChildList.hh"
#include "MathMLDummyElement.hh"
#include "RenderingEnvironment.hh"
#include "MathMLOperatorElement.hh"
#include "MathMLMultiScriptsElement.hh"
#include "FormattingContext.hh"

MathMLMultiScriptsElement::MathMLMultiScriptsElement()
{
}

#if defined(HAVE_GMETADOM)
MathMLMultiScriptsElement::MathMLMultiScriptsElement(const GMetaDOM::Element& node)
  : MathMLLinearContainerElement(node)
{
}
#endif

MathMLMultiScriptsElement::~MathMLMultiScriptsElement()
{
}

void
MathMLMultiScriptsElement::SetBase(const Ptr<MathMLElement>& elem)
{
  if (elem != base)
    {
      if (elem) elem->SetParent(this);
      if (base) base->SetParent(0);
      base = elem;
      SetDirtyLayout();
    }
}

void
MathMLMultiScriptsElement::SetScriptsSize(unsigned size)
{
  assert(size <= scripts.size());
  if (size != scripts.size())
    {
      for (unsigned i = size; i < scripts.size(); i++)
	{
	  SetSubScript(i, 0);
	  SetSuperScript(i, 0);
	}
      scripts.resize(size);
      SetDirtyLayout();
    }
}

void
MathMLMultiScriptsElement::SetPreScriptsSize(unsigned size)
{
  assert(size <= preScripts.size());
  if (size != preScripts.size())
    {
      for (unsigned i = size; i < preScripts.size(); i++)
	{
	  SetPreSubScript(i, 0);
	  SetPreSuperScript(i, 0);
	}
      preScripts.resize(size);
      SetDirtyLayout();
    }
}

void
MathMLMultiScriptsElement::SetSubScript(unsigned i, const Ptr<MathMLElement>& elem)
{
  assert(i <= script.size());
  if (i == script.size()) AppendScripts(elem, 0);
  else if (elem != script[i].first)
    {
      if (script[i].first) script[i].first->SetParent(0);
      elem->SetParent(this);
      script[i].first = elem;
      SetDirtyLayout();
    }
}

void
MathMLMultiScriptsElement::SetSuperScript(unsigned i, const Ptr<MathMLElement>& elem)
{
  assert(i <= script.size());
  if (i == script.size()) AppendScripts(0, elem);
  else if (elem != script[i].second)
    {
      if (script[i].second) script[i].second->SetParent(0);
      elem->SetParent(this);
      script[i].second = elem;
      SetDirtyLayout();
    }
}

void
MathMLMultiScriptsElement::AppendScripts(const Ptr<MathMLElement>& sub, const Ptr<MathMLElement>& sup)
{
  script.push_back(std::pair< Ptr<MathMLElement>,Ptr<MathMLElement> >(sub, sup));
}

void
MathMLMultiScriptsElement::SetPreSubScript(unsigned i, const Ptr<MathMLElement>& elem)
{
  assert(i <= preScript.size());
  if (i == preScript.size()) AppendPreScripts(elem, 0);
  else if (elem != preScript[i].first)
    {
      if (preScript[i].first) preScript[i].first->SetParent(0);
      elem->SetParent(this);
      preScript[i].first = elem;
      SetDirtyLayout();
    }
}

void
MathMLMultiScriptsElement::SetPreSuperScript(unsigned i, const Ptr<MathMLElement>& elem)
{
  assert(i <= preScript.size());
  if (i == preScript.size()) AppendPreScripts(0, elem);
  else if (elem != preScript[i].second)
    {
      if (preScript[i].second) preScript[i].second->SetParent(0);
      elem->SetParent(this);
      preScript[i].second = elem;
      SetDirtyLayout();
    }
}

void
MathMLMultiScriptsElement::AppendPreScripts(const Ptr<MathMLElement>& sub, const Ptr<MathMLElement>& sup)
{
  preScript.push_back(std::pair< Ptr<MathMLElement>,Ptr<MathMLElement> >(sub, sup));
}

void
MathMLMultiScriptsElement::Normalize()
{
  if (HasDirtyStructure() || HasChildWithDirtyStructure())
    {
#if defined(HAVE_GMETADOM)
      ChildList children(GetDOMElement(), MATHML_NS_URI, "*");
      for (unsigned i = 0; i < children.get_length(); i++)
	{
	  GMetaDOM::Node node = children.item(i);
	  assert(node.get_nodeType() == GMetaDOM::Node::ELEMENT_NODE);

	  if (i == 0 &&
	      (node.get_nodeName() == "none" || node.get_nodeName() == "mprescripts"))
	    {
	      Ptr<MathMLElement> mdummy = MathMLDummyElement::create();
	      assert(mdummy);
	      SetChild(0, mdummy);
	    }
	  else
	    {
	      Ptr<MathMLElement> elem = MathMLElement::getRenderingInterface(node);
	      // it might be that we get a NULL. In that case it would probably make
	      // sense to create a dummy element, because we filtered MathML
	      // elements only
	      assert(elem);
	      SetChild(i, elem);
	    }
	}

      // the following is to be sure that no spurious elements remain at the
      // end of the container
      SetSize(children.get_length());
#endif // HAVE_GMETADOM
    }

  if (content.GetSize() == 0 ||
      (content.GetFirst() && content.GetFirst()->IsA() == TAG_NONE) ||
      (content.GetFirst() && content.GetFirst()->IsA() == TAG_MPRESCRIPTS))
    {
    }

  base = content.GetFirst();
  assert(base);

  unsigned i = 0;
  bool     preScripts = false;

  for (Iterator< Ptr<MathMLElement> > elem(content); elem.More(); elem.Next(), i++)
    {
      assert(elem());
      if (elem()->IsA() == TAG_MPRESCRIPTS) 
	{
	  preScripts = true;
	  break;
	}
    }

  if (preScripts)
    {
      nPre  = content.GetSize() - i - 1;
      nPost = content.GetSize() - nPre - 2;
    }
  else
    {
      nPre  = 0;
      nPost = content.GetSize() - 1;
    }
}

void
MathMLMultiScriptsElement::Setup(RenderingEnvironment* env)
{
  assert(content.GetSize() > 0);
  assert(content.GetFirst());

  content.GetFirst()->Setup(env);
  
  env->Push();
  env->AddScriptLevel(1);
  env->SetDisplayStyle(false);

  Iterator< Ptr<MathMLElement> > elem(content);
  elem.Next();
  while (elem.More())
    {
      if (elem()) elem()->Setup(env);
      elem.Next();
    }

  ScriptSetup(env);

  env->Drop();
}

void
MathMLMultiScriptsElement::DoLayout(const class FormattingContext& ctxt)
{
  if (!HasDirtyLayout()) return;

  assert(base);

  unsigned n = 1 + nPre / 2 + nPost / 2;
  assert(n > 0);

  base->DoLayout(ctxt);

  BoundingBox subScriptBox;
  BoundingBox superScriptBox;

  subScriptBox.Null();
  superScriptBox.Null();

  scaled totalWidth = 0;
  scaled subScriptWidth = 0;
  bool preScript = false;
  unsigned i = 0;
  Iterator< Ptr<MathMLElement> > elem(content);
  elem.Next();

  while (elem.More())
    {
      assert(elem());

      elem()->DoLayout(ctxt);

      if (!preScript && elem()->IsA() == TAG_MPRESCRIPTS)
	{
	  preScript = true;
	  i = 0;
	}
      else
	{
	  if (i % 2 == 0)
	    {
	      const BoundingBox& scriptBox = elem()->GetBoundingBox();
	      subScriptBox.Append(scriptBox);
	      subScriptWidth = scriptBox.width;
	    } 
	  else
	    {
	      const BoundingBox& scriptBox = elem()->GetBoundingBox();
	      superScriptBox.Append(scriptBox);
	      totalWidth += scaledMax(subScriptWidth, scriptBox.width);
	    }

	  i++;
	}

      elem.Next();
    }

  DoScriptLayout(base->GetBoundingBox(), subScriptBox, superScriptBox,
		 subShiftX, subShiftY, superShiftX, superShiftY);

  box = base->GetBoundingBox();

  if (!subScriptBox.IsNull())
    {
      box.ascent  = scaledMax(box.ascent, subScriptBox.ascent - subShiftY);
      box.descent = scaledMax(box.descent, subScriptBox.descent + subShiftY);
    }

  if (!superScriptBox.IsNull())
    {
      box.ascent  = scaledMax(box.ascent, superScriptBox.ascent + superShiftY);
      box.descent = scaledMax(box.descent, superScriptBox.descent - superShiftY);
    }

  ResetDirtyLayout(ctxt.GetLayoutType());
}

void
MathMLMultiScriptsElement::SetPosition(scaled x, scaled y)
{
  position.x = x;
  position.y = y;

  Iterator< Ptr<MathMLElement> > elem(content);
  elem.More();

  scaled subScriptWidth = 0;
  bool preScript = false;
  unsigned i = 0;

  if (nPre > 0)
    {
      while (elem.More())
	{
	  assert(elem());

	  if (preScript)
	    {
	      if (i % 2 == 0)
		{
		  const BoundingBox& scriptBox = elem()->GetBoundingBox();
		  subScriptWidth = scriptBox.width;
		  elem()->SetPosition(x, y + subShiftY);
		} 
	      else
		{
		  const BoundingBox& scriptBox = elem()->GetBoundingBox();
		  elem()->SetPosition(x, y - superShiftY);
		  x += scaledMax(subScriptWidth, scriptBox.width);
		}

	      i++;
	    } 
	  else if (elem()->IsA() == TAG_MPRESCRIPTS)
	    {
	      preScript = true;
	      i = 0;
	    }

	  elem.Next();
	}
    }

  base->SetPosition(x, y);

  if (nPost > 0)
    {
      x += scaledMax(subShiftX, superShiftX);

      elem.ResetFirst();
      elem.Next();

      subScriptWidth = 0;
      preScript = false;
      i = 0;
    
      while (elem.More() && !preScript)
	{
	  assert(elem());

	  if (elem()->IsA() == TAG_MPRESCRIPTS) preScript = true;
	  else
	    {
	      if (i % 2 == 0)
		{
		  const BoundingBox& scriptBox = elem()->GetBoundingBox();
		  subScriptWidth = scriptBox.width;
		  elem()->SetPosition(x, y + subShiftY);
		} 
	      else
		{
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

Ptr<class MathMLOperatorElement>
MathMLMultiScriptsElement::GetCoreOperator()
{
  assert(base);
  return base->GetCoreOperator();
}
