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

#ifndef __Area_hh__
#define __Area_hh__

#include "DOM.hh"
#include "AreaId.hh"
#include "BoundingBox.hh"
#include "Object.hh"
#include "SmartPtr.hh"

typedef SmartPtr<const class Area> AreaRef;

class Area : public Object
{
protected:
  Area(void) { };
  virtual ~Area() { };

public:
  virtual BoundingBox  box(void) const = 0;
  virtual void         render(class RenderingContext&, const scaled& x, const scaled& y) const = 0;
  virtual AreaRef      fit(const scaled&, const scaled&, const scaled&) const = 0;
  virtual AreaRef      replace(const class ReplacementContext&) const = 0;
  virtual scaled       leftEdge(void) const = 0;
  virtual scaled       rightEdge(void) const = 0;
          AreaRef      node(const AreaId& id) const { return node(id.path.begin(), id.path.end()); }
          scaled       origin(const AreaId& id) const { return origin(id.path.begin(), id.path.end()); }
          scaled       leftSide(const AreaId& id) const { return leftSide(id.path.begin(), id.path.end()); }
          scaled       rightSide(const AreaId& id) const { return rightSide(id.path.begin(), id.path.end()); }

  // creates an id object of the first occurrence of the argument
  // during a depth-first visit of the area tree
  // throws InvalidArea if the argument is not found
  AreaId idOf(const AreaRef&) const;

  virtual SmartPtr<Area> clone(void) const = 0;
  virtual AreaRef      flatten(void) const { return this; }
  virtual bool         idOf(const AreaRef&, AreaIdFactory&) const = 0;
  virtual AreaRef      node(const AreaId::const_iterator, const class AreaId::const_iterator) const = 0;  
  virtual scaled       origin(const AreaId::const_iterator, const class AreaId::const_iterator) const = 0;
  virtual scaled       leftSide(const AreaId::const_iterator, const class AreaId::const_iterator) const = 0;
  virtual scaled       rightSide(const AreaId::const_iterator, const class AreaId::const_iterator) const = 0;
  virtual void         strength(int&, int&, int&) const = 0;
  virtual DOM::Element dump(const DOM::Document&) const = 0;

  class InvalidId { };
  class NotAllowed { };
  class NotFound { };
};

#endif // __Area_hh__