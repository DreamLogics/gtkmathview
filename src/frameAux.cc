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

#include "frameAux.hh"
#include "Iterator.hh"
#include "MathMLFrame.hh"
#include "MathMLTextNode.hh"
#include "MathMLTokenElement.hh"
#include "MathMLContainerElement.hh"

const BoundingBox&
getFrameBoundingBox(const Ptr<MathMLFrame>& frame)
{
  assert(frame != Ptr<MathMLFrame>(0));
  return frame->GetBoundingBox();
}

Ptr<MathMLFrame>
getFrameLeftSibling(const Ptr<MathMLFrame>& frame)
{
  assert(frame != 0);
  assert(frame->GetParent() != 0);

  if (is_a<MathMLTokenElement>(frame->GetParent()))
    {
      Ptr<MathMLTokenElement> token = smart_cast<MathMLTokenElement>(frame->GetParent());
      assert(token != 0);

      Ptr<MathMLFrame> left = 0;
      for (Iterator< Ptr<MathMLTextNode> > p(token->GetContent()); p.More(); p.Next())
	{
	  if (Ptr<MathMLFrame>(p()) == frame) return left;
	  left = p();
	}
    }
#if 0
  // to be reimplemented when things stabilize again
  else if (frame->GetParent()->IsA() == TAG_MROW)
    {
      MathMLRowElement* row = TO_ROW(frame->GetParent());
      assert(row != NULL);

      MathMLElement* left = NULL;
      for (Iterator< Ptr<MathMLElement> > p(container->content); p.More(); p.Next()) {
	if (p() == frame) return left;
	left = p();
      }
    }
#endif

  return 0;
}

Ptr<MathMLFrame>
getFrameRightSibling(const Ptr<MathMLFrame>& frame)
{
  assert(frame != 0);
  assert(frame->GetParent() != 0);

  if (is_a<MathMLTokenElement>(frame->GetParent()))
    {
      Ptr<MathMLTokenElement> token = smart_cast<MathMLTokenElement>(frame->GetParent());
      assert(token != 0);

      for (Iterator< Ptr<MathMLTextNode> > p(token->GetContent()); p.More(); p.Next())
	if (Ptr<MathMLFrame>(p()) == frame)
	  {
	    p.Next();
	    if (p.More()) return p();
	  }
    } 
#if 0
  // to be reimplemented when things stabilize again
  else if (frame->GetParent()->IsContainer())
    {
      MathMLContainerElement* container = TO_CONTAINER(frame->GetParent());
      assert(container != NULL);

      for (Iterator<MathMLElement*> p(container->content); p.More(); p.Next())
	if (p() == frame) {
	  p.Next();
	  if (p.More()) return p();
	}
    }
#endif

  return 0;
}
