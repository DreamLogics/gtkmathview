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

#ifndef MathMLOperatorElement_hh
#define MathMLOperatorElement_hh

#if defined(HAVE_GMETADOM)
#include "gmetadom.hh"
#endif

#include "CharMap.hh"
#include "MathMLEmbellishment.hh"
#include "MathMLTokenElement.hh"

class MathMLOperatorElement
  : public MathMLTokenElement, public MathMLEmbellishment
{
protected:
  MathMLOperatorElement(void);
#if defined(HAVE_GMETADOM)
  MathMLOperatorElement(const DOM::Element&);
#endif
  virtual ~MathMLOperatorElement();

private:
  void Init(void);

public:
  static Ptr<MathMLElement> create(void)
  { return Ptr<MathMLElement>(new MathMLOperatorElement()); }
#if defined(HAVE_GMETADOM)
  static Ptr<MathMLElement> create(const DOM::Element& el)
  { return Ptr<MathMLElement>(new MathMLOperatorElement(el)); }
#endif

  virtual const AttributeSignature* GetAttributeSignature(AttributeId) const;
  //virtual void Normalize(const Ptr<class MathMLDocument>&);
  virtual void Setup(class RenderingEnvironment&);
  virtual void DoLayout(const class FormattingContext&);
  virtual void SetPosition(scaled, scaled);

  bool         IsStretchy(void) const { return stretchy != 0; }
  StretchId    GetStretch(void) const;
  bool         IsAccent(void) const { return accent != 0; }
  bool         ForcedFence(void) const { return forcedFence != 0; }
  bool         IsFence(void) const { return fence != 0; }
  bool         ForcedSeparator(void) const { return forcedSeparator != 0; }
  bool         IsSeparator(void) const { return separator != 0; }
  bool         ForcedSymmetric(void) const { return forcedSymmetric != 0; }
  bool         IsSymmetric(void) const { return symmetric != 0; }
  bool         HasInfiniteMaxSize(void) const { return infiniteMaxSize != 0; }
  bool         HasMovableLimits(void) const { return movableLimits != 0; }
  void         SetFence(void) { forcedFence = fence = 1; }
  void         SetSeparator(void) { forcedSeparator = separator = 1; }
  void         ResetSymmetric(void) { forcedSymmetric = 1; symmetric = 0; }
  BoundingBox  GetMinBoundingBox(void) const { return minBox; }
  scaled       GetLeftPadding(void) const { return lSpace; }
  scaled       GetRightPadding(void) const { return rSpace; }
#ifdef ENABLE_EXTENSIONS
  scaled       GetTopPadding(void) const { return tSpace; }
  scaled       GetBottomPadding(void) const { return bSpace; }
#endif // ENABLE_EXTENSIONS

  void         HorizontalStretchTo(scaled, bool = false);
  void         VerticalStretchTo(scaled, scaled, bool = false);

  virtual Ptr<MathMLOperatorElement> GetCoreOperator(void);
  //Ptr<class MathMLEmbellishedOperatorElement> GetEmbellishment(void) const;

private:
  OperatorFormId InferOperatorForm(void);
  const Value* GetOperatorAttributeValue(AttributeId,
					 const class RenderingEnvironment&) const;
  void ParseLimitValue(const Value*, const class RenderingEnvironment&, float&, scaled&);

  //Ptr<class MathMLEmbellishedOperatorElement> eOp; // ptr to the root of the embellished operator

  OperatorFormId form;
  const class MathMLAttributeList* defaults;

  scaled axis;

  unsigned forcedFence : 1;
  unsigned fence : 1;
  unsigned forcedSeparator : 1;
  unsigned separator : 1;
  unsigned forcedSymmetric : 1;
  unsigned symmetric : 1;
  unsigned stretchy : 1;
  unsigned infiniteMaxSize : 1; // if != 0 => no max size contraint
  unsigned movableLimits : 1;
  unsigned accent : 1;
  scaled lSpace;
  scaled rSpace;
#ifdef ENABLE_EXTENSIONS
  scaled tSpace;
  scaled bSpace;
#endif // ENABLE_EXTENSIONS
  float  maxMultiplier;   // if <0 => absolute max size contraint
  scaled maxSize;
  float  minMultiplier;   // if <0 => absolute min size constraint
  scaled minSize;

  BoundingBox minBox;
};

#endif // MathMLOperatorElement_hh
