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
  MathMLDocument(const GMetaDOM::Element&);
  void Init(void);
#endif
  virtual ~MathMLDocument();

public:
  static Ptr<MathMLDocument> create(void)
  { return Ptr<MathMLDocument>(new MathMLDocument()); }
#if defined(HAVE_GMETADOM)
  static Ptr<MathMLDocument> create(const GMetaDOM::Document& doc)
  { return Ptr<MathMLDocument>(new MathMLDocument(doc)); }
  static Ptr<MathMLDocument> create(const GMetaDOM::Element& elem)
  { return Ptr<MathMLDocument>(new MathMLDocument(elem)); }
#endif

  virtual void Normalize(void);
  virtual bool IsDocument(void) const;

  Ptr<MathMLElement> GetRoot(void) const { return GetChild(); }

#if defined(HAVE_GMETADOM)
  const GMetaDOM::Document& GetDOMDocument(void) const { return DOMdoc; }
  const GMetaDOM::Element& GetDOMRoot(void) const { return DOMroot; }
protected:
  class DOMCharacterDataModifiedListener : public GMetaDOM::EventListener
  {
  public:
    virtual void handleEvent(const GMetaDOM::Event&);
  };

  class DOMNodeInsertedListener : public GMetaDOM::EventListener
  {
  public:
    virtual void handleEvent(const GMetaDOM::Event&);
  };

  class DOMNodeRemovedListener : public GMetaDOM::EventListener
  {
  public:
    virtual void handleEvent(const GMetaDOM::Event&);
  };

  class DOMAttrModifiedListener : public GMetaDOM::EventListener
  {
  public:
    virtual void handleEvent(const GMetaDOM::Event&);
  };

  DOMCharacterDataModifiedListener characterDataModifiedListener;
  DOMNodeInsertedListener nodeInsertedListener;
  DOMNodeRemovedListener nodeRemovedListener;
  DOMAttrModifiedListener attrModifiedListener;

  GMetaDOM::Document DOMdoc;  // can be 0
  GMetaDOM::Element  DOMroot; // can be 0
#endif
};

#endif // MathMLDocument_hh
