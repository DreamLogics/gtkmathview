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

#ifndef MathMLRowElement_hh
#define MathMLRowElement_hh

#if defined(HAVE_GMETADOM)
#include "gmetadom.hh"
#endif

#include "MathMLLinearContainerElement.hh"

class MathMLRowElement: public MathMLLinearContainerElement
{
protected:
  MathMLRowElement(void);
#if defined(HAVE_GMETADOM)
  MathMLRowElement(const GMetaDOM::Element&);
#endif
  virtual ~MathMLRowElement();

public:
  static MathMLElement* create(void) { return new MathMLRowElement(); }
#if defined(HAVE_GMETADOM)
  static MathMLElement* create(const GMetaDOM::Element& el) { return new MathMLRowElement(el); }
#endif

  // virtual void Normalize(void);
  virtual void Setup(class RenderingEnvironment*);
  virtual void DoLayout(LayoutId, class Layout&);
  virtual void DoStretchyLayout(void);

  virtual bool IsBreakable(void) const;
  virtual bool IsSpaceLike(void) const;
  virtual bool IsExpanding(void) const;

  OperatorFormId GetOperatorForm(MathMLElement*) const;
  virtual class MathMLOperatorElement* GetCoreOperator();

private:
  MathMLElement* lastElement;
};

#define TO_ROW(obj) (dynamic_cast<MathMLRowElement*>(obj))

#endif // MathMLRowElement_hh
