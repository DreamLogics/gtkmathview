// Copyright (C) 2000-2007, Luca Padovani <padovani@sti.uniurb.it>.
//
// This file is part of GtkMathView, a flexible, high-quality rendering
// engine for MathML documents.
// 
// GtkMathView is free software; you can redistribute it and/or modify it
// either under the terms of the GNU Lesser General Public License version
// 3 as published by the Free Software Foundation (the "LGPL") or, at your
// option, under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation (the "GPL").  If you do not
// alter this notice, a recipient may use your version of this file under
// either the GPL or the LGPL.
//
// GtkMathView is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the LGPL or
// the GPL for more details.
// 
// You should have received a copy of the LGPL and of the GPL along with
// this program in the files COPYING-LGPL-3 and COPYING-GPL-2; if not, see
// <http://www.gnu.org/licenses/>.

#include <config.h>

#include <sstream>
#include <iomanip>
#include <iostream>
#include "AbstractLogger.hh"
#include "libxml2_Model.hh"
#include "libxml2_MathView.hh"
#include "SVG_libxml2_StreamRenderingContext.hh"

SVG_libxml2_StreamRenderingContext::SVG_libxml2_StreamRenderingContext(const SmartPtr<AbstractLogger>& logger,
								       std::ostream& output,
								       const SmartPtr<libxml2_MathView>& _view)
  : SVG_StreamRenderingContext(logger, output), view(_view)
{ }

SVG_libxml2_StreamRenderingContext::~SVG_libxml2_StreamRenderingContext()
{ }

String
SVG_libxml2_StreamRenderingContext::toSVGLength(const scaled& s) const
{
  std::ostringstream buffer;
  buffer << std::fixed << std::setprecision(2) << (s.toFloat() * 72.27) / 90;
  return buffer.str();
}

String
SVG_libxml2_StreamRenderingContext::getId(const SmartPtr<Element>& elem) const
{
  assert(elem);
  if (xmlElement* el = view->modelElementOfElement(elem))
    {
      if (xmlChar* id = xmlGetProp((xmlNode*) el, libxml2_Model::toModelString("id")))
	{
	  String res = libxml2_Model::fromModelString(id);
	  xmlFree(id);
	  return res;
	}
    }
  return "";
}
