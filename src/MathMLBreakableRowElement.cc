// Copyright (C) 2000-2003, Luca Padovani <luca.padovani@cs.unibo.it>.
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
// http://helm.cs.unibo.it/mml-widget, or send a mail to
// <luca.padovani@cs.unibo.it>

#include <config.h>

#include "RenderingEnvironment.hh"
#include "MathMLBreakableRowElement.hh"

void
MathMLBreakableRowElement::Setup(RenderingEnvironment& env)
{
  if (DirtyAttribute() || DirtyAttributeP())
    {
      if (!layout && !GetLayout()) layout = VerticalLayout::create();
      if (layout) layout->SetSpacing(env.GetScaledPointsPerEm(), env.GetScaledPointsPerEx() / 2);
      MathMLRowElement::Setup(env);
      ResetDirtyAttribute();
    }  
}

void
MathMLBreakableRowElement::DoLayout(const FormattingContext& ctxt)
{
  if (DirtyLayout(ctxt))
    {
      Ptr<VerticalLayout> l = GetLayout();
      assert(l);

      l->RemoveAll();
      DoBreakableLayout(ctxt, l);

      box = l->GetBoundingBox();
      DoEmbellishmentLayout(this, box);
    }
}

void
MathMLBreakableRowElement::DoBreakableLayout(const FormattingContext& ctxt, const Ptr<VerticalLayout>& l)
{
  assert(l);

  for (std::vector< Ptr<MathMLElement> >::iterator elem = content.begin();
       elem != content.end();
       elem++)
    {
      if (Ptr<MathMLBreakableRowElement> brow = smart_cast<MathMLBreakableRowElement>(*elem))
	brow->DoBreakableLayout(ctxt, l);
      else
	{
	  (*elem)->DoLayout(ctxt);
	  l->Add(*elem);
	}
    }

  DoStretchyLayout();
  ResetDirtyLayout(ctxt);
}

void
MathMLBreakableRowElement::SetPosition(scaled x, scaled y)
{
  assert(layout);
  position.x = x;
  position.y = y;
  SetEmbellishmentPosition(this, x, y);
  layout->SetPosition(x, y);
}

Ptr<VerticalLayout>
MathMLBreakableRowElement::GetLayout() const
{
  if (layout)
    return layout;
  else if (Ptr<MathMLBreakableRowElement> parent = smart_cast<MathMLBreakableRowElement>(GetParent()))
    return parent->GetLayout();
  else
    return 0;
}

scaled
MathMLBreakableRowElement::GetExitBaseline() const
{
  assert(layout);
  return layout->GetExitBaseline();
}
