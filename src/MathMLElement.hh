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

#ifndef MathMLElement_hh
#define MathMLElement_hh

#if defined(HAVE_GMETADOM)
#include "gmetadom.hh"
#endif

#include "keyword.hh"
#include "MathMLFrame.hh"
#include "BoundingBox.hh"
#include "DrawingArea.hh"
#include "AttributeSignature.hh"

// MathMLElement: base class for every MathML Element
class MathMLElement: public MathMLFrame
{
protected:
  MathMLElement(void);
#if defined(HAVE_GMETADOM)
  MathMLElement(const GMetaDOM::Element&);
#endif
  virtual ~MathMLElement();
private:
  void Init(void);

public:
  virtual const AttributeSignature* GetAttributeSignature(AttributeId) const;
  virtual void Normalize(void) = 0;
  virtual void Setup(class RenderingEnvironment*); // setup attributes
  virtual void DoLayout(LayoutId, scaled = 0);
  virtual void DoStretchyLayout(void);
  virtual void RenderBackground(const DrawingArea&);
  virtual void Render(const DrawingArea&);
  virtual void ReleaseGCs(void);
  virtual void SetDirty(const Rectangle* = NULL);
  virtual Ptr<MathMLElement> Inside(scaled, scaled);
  virtual bool IsElement(void) const;

  const class GraphicsContext* GetForegroundGC(void) const { return fGC[IsSelected()]; }
  const class GraphicsContext* GetBackgroundGC(void) const { return bGC[IsSelected()]; }

  // attributes
  const String* GetDefaultAttribute(AttributeId) const;
  const Value*  GetDefaultAttributeValue(AttributeId) const;
  const String* GetAttribute(AttributeId,
			     const RenderingEnvironment* = NULL,
			     bool = true) const;
  const Value*  GetAttributeValue(AttributeId,
				  const RenderingEnvironment* = NULL,
				  bool = true) const;
  const Value*  ParseAttribute(AttributeId, const String*) const;
  static const Value* Resolve(const Value*,
			      const RenderingEnvironment*,
			      int = -1, int = -1);
  bool IsSet(AttributeId) const;

  // some queries
  TagId        	 IsA(void) const;
#if defined(HAVE_GMETADOM)
  const GMetaDOM::Element& GetDOMElement(void) const { return node; }
  static Ptr<MathMLElement> getRenderingInterface(const GMetaDOM::Element&);
#endif
  Rectangle      GetRectangle(void) const;
  virtual bool 	 IsSpaceLike(void) const;
  virtual bool 	 IsExpanding(void) const;
  virtual bool 	 IsInside(scaled, scaled) const;
  bool           HasLink(void) const;
  RGBValue     	 GetBackgroundColor(void) const { return background; }
  unsigned     	 GetDepth(void) const;
  virtual scaled GetLeftEdge(void) const;
  virtual scaled GetRightEdge(void) const;
  virtual Ptr<class MathMLOperatorElement> GetCoreOperator(void);

  bool HasDirtyLayout(void) const { return MathMLFrame::HasDirtyLayout(); }
  void ResetDirtyLayout(void) { MathMLFrame::ResetDirtyLayout(); }
  bool HasDirtyLayout(LayoutId, scaled) const;
  void ResetDirtyLayout(LayoutId id) { if (id == LAYOUT_AUTO) MathMLFrame::ResetDirtyLayout(); }

protected:
  const AttributeSignature* GetAttributeSignatureAux(AttributeId,
						     AttributeSignature[]) const;

public:
  virtual void SetDirtyStructure(void);
  void         ResetDirtyStructure(void) { dirtyStructure = childWithDirtyStructure = 0; }
  bool         HasDirtyStructure(void) const { return dirtyStructure != 0; }
  bool         HasChildWithDirtyStructure(void) const { return childWithDirtyStructure != 0; }

  virtual void SetDirtyAttribute(void);
  void         ResetDirtyAttribute(void) { dirtyAttribute = childWithDirtyAttribute = 0; }
  bool         HasDirtyAttribute(void) const { return dirtyAttribute != 0; }
  bool         HasChildWithDirtyAttribute(void) const { return childWithDirtyAttribute != 0; }

protected:
  unsigned    dirtyStructure : 1;
  unsigned    childWithDirtyStructure : 1;
  unsigned    dirtyAttribute : 1;
  unsigned    childWithDirtyAttribute : 1;

  const class GraphicsContext* fGC[2];
  const class GraphicsContext* bGC[2];

  RGBValue background; // background color

private:
#if defined(HAVE_GMETADOM)
  GMetaDOM::Element node; // reference to the DOM node
#endif
};

#endif // MathMLElement_hh
