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

#include <algorithm>
#include <functional>

#include <assert.h>
#include <stddef.h>

#include "Adaptors.hh"
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
  : MathMLContainerElement(node)
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
  assert(size <= subScript.size());
  if (size != subScript.size())
    {
      assert(subScript.size() == superScript.size());
      for (unsigned i = size; i < subScript.size(); i++)
	{
	  SetSubScript(i, 0);
	  SetSuperScript(i, 0);
	}
      subScript.resize(size);
      superScript.resize(size);
      SetDirtyLayout();
    }
}

void
MathMLMultiScriptsElement::SetPreScriptsSize(unsigned size)
{
  assert(size <= preSubScript.size());
  if (size != preSubScript.size())
    {
      assert(preSubScript.size() == preSuperScript.size());
      for (unsigned i = size; i < preSubScript.size(); i++)
	{
	  SetPreSubScript(i, 0);
	  SetPreSuperScript(i, 0);
	}
      preSubScript.resize(size);
      preSuperScript.resize(size);
      SetDirtyLayout();
    }
}

Ptr<MathMLElement>
MathMLMultiScriptsElement::GetSubScript(unsigned i) const
{
  assert(i < subScript.size());
  return subScript[i];
}

Ptr<MathMLElement>
MathMLMultiScriptsElement::GetSuperScript(unsigned i) const
{
  assert(i < superScript.size());
  return superScript[i];
}

void
MathMLMultiScriptsElement::SetSubScript(unsigned i, const Ptr<MathMLElement>& elem)
{
  assert(i <= subScript.size());
  if (i == subScript.size())
    {
      assert(subScript.size() == superScript.size());
      subScript.push_back(elem);
      superScript.push_back(0);
    }
  else if (elem != subScript[i])
    {
      if (subScript[i]) subScript[i]->SetParent(0);
      elem->SetParent(this);
      subScript[i] = elem;
      SetDirtyLayout();
    }
}

void
MathMLMultiScriptsElement::SetSuperScript(unsigned i, const Ptr<MathMLElement>& elem)
{
  assert(i <= superScript.size());
  if (i == superScript.size())
    {
      assert(superScript.size() == subScript.size());
      subScript.push_back(0);
      superScript.push_back(elem);
    }
  else if (elem != superScript[i])
    {
      if (superScript[i]) superScript[i]->SetParent(0);
      elem->SetParent(this);
      superScript[i] = elem;
      SetDirtyLayout();
    }
}

Ptr<MathMLElement>
MathMLMultiScriptsElement::GetPreSubScript(unsigned i) const
{
  assert(i < preSubScript.size());
  return preSubScript[i];
}

Ptr<MathMLElement>
MathMLMultiScriptsElement::GetPreSuperScript(unsigned i) const
{
  assert(i < preSuperScript.size());
  return preSuperScript[i];
}

void
MathMLMultiScriptsElement::SetPreSubScript(unsigned i, const Ptr<MathMLElement>& elem)
{
  assert(i <= preSubScript.size());
  if (i == preSubScript.size())
    {
      assert(preSubScript.size() == preSuperScript.size());
      preSubScript.push_back(elem);
      preSuperScript.push_back(0);
    }
  else if (elem != preSubScript[i])
    {
      if (preSubScript[i]) preSubScript[i]->SetParent(0);
      elem->SetParent(this);
      preSubScript[i] = elem;
      SetDirtyLayout();
    }
}

void
MathMLMultiScriptsElement::SetPreSuperScript(unsigned i, const Ptr<MathMLElement>& elem)
{
  assert(i <= preSuperScript.size());
  if (i == preSuperScript.size())
    {
      assert(preSubScript.size() == preSuperScript.size());
      preSubScript.push_back(0);
      preSuperScript.push_back(elem);
    }
  else if (elem != preSuperScript[i])
    {
      if (preSuperScript[i]) preSuperScript[i]->SetParent(0);
      elem->SetParent(this);
      preSuperScript[i] = elem;
      SetDirtyLayout();
    }
}

void
MathMLMultiScriptsElement::Normalize()
{
  if (HasDirtyStructure() || HasChildWithDirtyStructure())
    {
#if defined(HAVE_GMETADOM)
      ChildList children(GetDOMElement(), MATHML_NS_URI, "*");
      unsigned i = 0;
      unsigned nScripts = 0;
      unsigned nPreScripts = 0;
      unsigned n = children.get_length();
      bool preScripts = false;

      while (i < n)
	{
	  GMetaDOM::Node node = children.item(i);

	  if (i == 0)
	    {
	      Ptr<MathMLElement> elem;
	      if (node.get_nodeName() != "none" && node.get_nodeName() != "mprescripts")
		{
		  elem = MathMLElement::getRenderingInterface(node);
		  i++;
		}
	      if (elem) SetBase(elem);
	      else if (!is_a<MathMLDummyElement>(base)) SetBase(MathMLDummyElement::create());
	    }
	  else if (node.get_nodeName() == "mprescripts")
	    {
	      // FIXME: issue warning if preScripts is already set
	      preScripts = true;
	      i++;
	    }
	  else if (!preScripts)
	    {
	      Ptr<MathMLElement> sub;
	      Ptr<MathMLElement> sup;

	      if (node.get_nodeName() != "none")
		sub = MathMLElement::getRenderingInterface(node);
	      i++;
	      
	      if (i < n)
		{
		  node = children.item(i);
		  if (node.get_nodeName() != "none" && node.get_nodeName() != "mprescripts")
		    sup = MathMLElement::getRenderingInterface(node);
		  if (node.get_nodeName() != "mprescripts") i++;
		}

	      if (sub || sup)
		{
		  SetSubScript(nScripts, sub);
		  SetSuperScript(nScripts, sup);
		  nScripts++;
		}
	    }
	  else
	    {
	      Ptr<MathMLElement> sub;
	      Ptr<MathMLElement> sup;

	      if (node.get_nodeName() != "none")
		sub = MathMLElement::getRenderingInterface(node);
	      i++;
	      
	      if (i < n)
		{
		  node = children.item(i);
		  if (node.get_nodeName() != "none" && node.get_nodeName() != "mprescripts")
		    sup = MathMLElement::getRenderingInterface(node);
		  if (node.get_nodeName() != "mprescripts") i++;
		}

	      if (sub || sup)
		{
		  SetPreSubScript(nScripts, sub);
		  SetPreSuperScript(nScripts, sup);
		  nScripts++;
		}
	    }
	}
      
      if (n == 0 && !is_a<MathMLDummyElement>(base)) base = MathMLDummyElement::create();
      SetScriptsSize(nScripts);
      SetPreScriptsSize(nPreScripts);
#endif // HAVE_GMETADOM

      if (base) base->Normalize();
      std::for_each(subScript.begin(), subScript.end(), NormalizeAdaptor());
      std::for_each(superScript.begin(), superScript.end(), NormalizeAdaptor());
      std::for_each(preSubScript.begin(), preSubScript.end(), NormalizeAdaptor());
      std::for_each(preSuperScript.begin(), preSuperScript.end(), NormalizeAdaptor());
    }
}

void
MathMLMultiScriptsElement::Setup(RenderingEnvironment* env)
{
  assert(base);
  base->Setup(env);

  env->Push();
  env->AddScriptLevel(1);
  env->SetDisplayStyle(false);

  std::for_each(subScript.begin(), subScript.end(), std::bind2nd(SetupAdaptor(), env));
  std::for_each(superScript.begin(), superScript.end(), std::bind2nd(SetupAdaptor(), env));
  std::for_each(preSubScript.begin(), preSubScript.end(), std::bind2nd(SetupAdaptor(), env));
  std::for_each(preSuperScript.begin(), preSuperScript.end(), std::bind2nd(SetupAdaptor(), env));

  ScriptSetup(env);

  env->Drop();
}

void
MathMLMultiScriptsElement::DoLayout(const class FormattingContext& ctxt)
{
  if (HasDirtyLayout())
    {
      assert(base);
      base->DoLayout(ctxt);

      BoundingBox subScriptBox;
      BoundingBox superScriptBox;
      scaled scriptsWidth = 0;
      scaled preScriptsWidth = 0;

      std::vector< Ptr<MathMLElement> >::const_iterator pSub;
      std::vector< Ptr<MathMLElement> >::const_iterator pSup;

      for (pSub = subScript.begin(), pSup = superScript.begin();
	   pSub != subScript.end();
	   pSub++, pSup++)
	{
	  assert(pSup != superScript.end());
	  BoundingBox subBox;
	  BoundingBox supBox;

	  if (*pSub)
	    {
	      (*pSub)->DoLayout(ctxt);
	      subBox = (*pSub)->GetBoundingBox();
	    }
	  if (*pSup)
	    {
	      (*pSup)->DoLayout(ctxt);
	      supBox = (*pSup)->GetBoundingBox();
	    }

	  subScriptBox.Append(subBox);
	  superScriptBox.Append(supBox);
	  scriptsWidth = scaledMax(scriptsWidth, scaledMax(subBox.width, supBox.width));
	}

      for (pSub = preSubScript.begin(), pSup = preSuperScript.begin();
	   pSub != preSubScript.end();
	   pSub++, pSup++)
	{
	  assert(pSup != preSuperScript.end());
	  BoundingBox subBox;
	  BoundingBox supBox;

	  if (*pSub)
	    {
	      (*pSub)->DoLayout(ctxt);
	      subBox = (*pSub)->GetBoundingBox();
	    }
	  if (*pSup)
	    {
	      (*pSup)->DoLayout(ctxt);
	      supBox = (*pSup)->GetBoundingBox();
	    }

	  subScriptBox.Append(subBox);
	  superScriptBox.Append(supBox);
	  preScriptsWidth = scaledMax(preScriptsWidth, scaledMax(subBox.width, supBox.width));
	}

      DoScriptLayout(base->GetBoundingBox(), subScriptBox, superScriptBox,
		     subShiftX, subShiftY, superShiftX, superShiftY);
  
      box = base->GetBoundingBox();
      box.Append(preScriptsWidth + scriptsWidth);

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
}

void
MathMLMultiScriptsElement::SetPosition(scaled x, scaled y)
{
  position.x = x;
  position.y = y;

  std::vector< Ptr<MathMLElement> >::reverse_iterator preSub;
  std::vector< Ptr<MathMLElement> >::reverse_iterator preSup;

  for (preSub = preSubScript.rbegin(), preSup = preSuperScript.rbegin();
       preSub != preSubScript.rend();
       preSub++, preSup++)
    {
      assert(preSup != preSuperScript.rend());
      scaled scriptW = 0;
      if (*preSub) 
	{
	  (*preSub)->SetPosition(x, y + subShiftY);
	  scriptW = (*preSub)->GetBoundingBox().GetWidth();
	}
      if (*preSup)
	{
	  (*preSup)->SetPosition(x, y - superShiftY);
	  scriptW = scaledMax(scriptW, (*preSup)->GetBoundingBox().GetWidth());
	}

      x += scriptW;
    }

  assert(base);
  base->SetPosition(x, y);

  std::vector< Ptr<MathMLElement> >::iterator pSub;
  std::vector< Ptr<MathMLElement> >::iterator pSup;

  for (pSub = subScript.begin(), pSup = superScript.begin();
       pSub != subScript.end();
       pSub++, pSup++)
    {
      assert(pSup != superScript.end());
      scaled scriptW = 0;
      if (*pSub) 
	{
	  (*pSub)->SetPosition(x + subShiftX, y + subShiftY);
	  scriptW = (*pSub)->GetBoundingBox().GetWidth();
	}
      if (*pSup)
	{
	  (*pSup)->SetPosition(x + superShiftX, y - superShiftY);
	  scriptW = scaledMax(scriptW, (*pSup)->GetBoundingBox().GetWidth());
	}

      x += scriptW;
    }
}

void
MathMLMultiScriptsElement::Render(const DrawingArea& area)
{
  if (HasDirtyChildren())
    {
      RenderBackground(area);
      assert(base);
      base->Render(area);
      std::for_each(subScript.begin(), subScript.end(), std::bind2nd(RenderAdaptor(), &area));
      std::for_each(superScript.begin(), superScript.end(), std::bind2nd(RenderAdaptor(), &area));
      std::for_each(preSubScript.begin(), preSubScript.end(), std::bind2nd(RenderAdaptor(), &area));
      std::for_each(preSuperScript.begin(), preSuperScript.end(), std::bind2nd(RenderAdaptor(), &area));
      ResetDirty();
    }
}

void
MathMLMultiScriptsElement::ReleaseGCs()
{
  MathMLElement::ReleaseGCs();
  assert(base);
  base->ReleaseGCs();
  std::for_each(subScript.begin(), subScript.end(), ReleaseGCsAdaptor());
  std::for_each(superScript.begin(), superScript.end(), ReleaseGCsAdaptor());
  std::for_each(preSubScript.begin(), preSubScript.end(), ReleaseGCsAdaptor());
  std::for_each(preSuperScript.begin(), preSuperScript.end(), ReleaseGCsAdaptor());
}

void
MathMLMultiScriptsElement::Replace(const Ptr<MathMLElement>& oldElem, const Ptr<MathMLElement>& newElem)
{
  assert(0);
}

scaled
MathMLMultiScriptsElement::GetLeftEdge() const
{
  if (preSubScript.size() > 0)
    {
      assert(preSuperScript.size() == preSubScript.size());
      Ptr<MathMLElement> sub = GetPreSubScript(preSubScript.size() - 1);
      Ptr<MathMLElement> sup = GetPreSuperScript(preSuperScript.size() - 1);
      if (sub && sup)
	return scaledMin(sub->GetLeftEdge(), sup->GetLeftEdge());
      else if (sub)
	return sub->GetLeftEdge();
      else if (sup)
	return sup->GetLeftEdge();
      else
	assert(0);
    }
  else
    {
      assert(base);
      return base->GetLeftEdge();
    }
}

scaled
MathMLMultiScriptsElement::GetRightEdge() const
{
  if (subScript.size() > 0)
    {
      assert(subScript.size() == superScript.size());
      Ptr<MathMLElement> sub = GetSubScript(subScript.size() - 1);
      Ptr<MathMLElement> sup = GetSuperScript(superScript.size() - 1);
      if (sub && sup)
	return scaledMin(sub->GetLeftEdge(), sup->GetLeftEdge());
      else if (sub)
	return sub->GetLeftEdge();
      else if (sup)
	return sup->GetLeftEdge();
      else
	assert(0);      
    }
  else
    {
      assert(base);
      return base->GetRightEdge();
    }
}

Ptr<class MathMLOperatorElement>
MathMLMultiScriptsElement::GetCoreOperator()
{
  assert(base);
  return base->GetCoreOperator();
}
