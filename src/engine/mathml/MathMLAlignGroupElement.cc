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

#include <cassert>

#include "MathMLMarkNode.hh"
#include "MathMLTokenElement.hh"
#include "MathMLAlignMarkElement.hh"
#include "MathMLAlignGroupElement.hh"

MathMLAlignGroupElement::MathMLAlignGroupElement(const SmartPtr<class MathMLNamespaceContext>& context)
  : MathMLElement(context)
{ }

MathMLAlignGroupElement::~MathMLAlignGroupElement()
{ }

void
MathMLAlignGroupElement::SetDecimalPoint(const SmartPtr<class MathMLTokenElement>& token)
{
  assert(token);
  assert(!decimalPoint);
  decimalPoint = token;
}

void
MathMLAlignGroupElement::SetAlignmentMark(const SmartPtr<class MathMLMarkNode>& mark)
{
  assert(mark);
  assert(!alignMarkNode);
  alignMarkNode = mark;
}

void
MathMLAlignGroupElement::SetAlignmentMark(const SmartPtr<class MathMLAlignMarkElement>& mark)
{
  assert(mark);
  assert(!alignMarkElement);
  alignMarkElement = mark;
}

bool
MathMLAlignGroupElement::IsSpaceLike() const
{ return true; }

void
MathMLAlignGroupElement::SetWidth(const scaled& w)
{ width = w; }
