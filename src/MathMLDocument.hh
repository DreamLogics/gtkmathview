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

#ifndef MathMLDocument_hh
#define MathMLDocument_hh

#if defined(HAVE_GMETADOM)
#include "gmetadom.hh"
#endif

#include "MathMLBinContainerElement.hh"

class MathMLDocument: public MathMLBinContainerElement
{
protected:
  MathMLDocument(void);
#if defined(HAVE_GMETADOM)
  MathMLDocument(const GMetaDOM::Document&);
#endif
  virtual ~MathMLDocument();

public:
  static MathMLDocument* create(void) { return new MathMLDocument(); }
#if defined(HAVE_GMETADOM)
  static MathMLDocument* create(const GMetaDOM::Document& doc) { return new MathMLDocument(doc); }
#endif

  virtual bool IsDocument(void) const;

  MathMLElement* GetRoot(void) const { return GetChild(); }

#if defined(HAVE_GMETADOM)
  const GMetaDOM::Document& GetDOMDocument(void) const { return DOMdoc; }
protected:
  GMetaDOM::Document DOMdoc;
#endif
};

typedef MathMLDocument* MathMLDocumentPtr;

#define TO_DOCUMENT(object) ((MathMLDocumentPtr) (object))

#endif // MathMLDocument_hh
