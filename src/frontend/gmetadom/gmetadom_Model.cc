// Copyright (C) 2000-2004, Luca Padovani <luca.padovani@cs.unibo.it>.
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
// http://helm.cs.unibo.it/mml-widget/, or send a mail to
// <lpadovan@cs.unibo.it>

#include <config.h>

#include <cassert>

#include "Clock.hh"
#include "Globals.hh"
#include "gmetadom_Model.hh"
#include "MathMLEntitiesTable.hh"

DOM::Element
gmetadom_Model::parseXML(const String& path, bool subst)
{
  DOM::Element root;

  Clock perf;
  perf.Start();
  if (!subst)
    {
      if (DOM::Document res = DOM::DOMImplementation().createDocumentFromURI(path.c_str()))
	root = res.get_documentElement();
    } 
  else
    {
      GdomeDOMImplementation* di = gdome_di_mkref();
      assert(di != NULL);
      GdomeException exc = 0;
      GdomeDocument* doc = gdome_di_createDocFromURIWithEntitiesTable(di,
								      path.c_str(),
								      getMathMLEntities(),
								      GDOME_LOAD_PARSING | GDOME_LOAD_SUBSTITUTE_ENTITIES,
								      &exc);
      if (exc != 0)
	{
	  gdome_di_unref(di, &exc);
	  gdome_doc_unref(doc, &exc);
	  return DOM::Document(0);
	}

      if (doc == 0)
	{
	  // FIXME: this should be signalled as an exception, I think
	  gdome_di_unref(di, &exc);
	  return DOM::Document(0);
	}

      DOM::Document res(doc);
      gdome_di_unref(di, &exc);
      assert(exc == 0);
      gdome_doc_unref(doc, &exc);
      assert(exc == 0);
    
      root = res.get_documentElement();
    }
  perf.Stop();
  Globals::logger(LOG_INFO, "parsing time: %dms", perf());

  return root;
}

String
gmetadom_Model::getElementValue(const DOM::Element& elem)
{
  DOM::GdomeString res = "";
  
  for (DOM::Node p = elem.get_firstChild(); p; p = p.get_nextSibling())
    {
      switch (p.get_nodeType()) {
      case DOM::Node::CDATA_SECTION_NODE:
      case DOM::Node::TEXT_NODE:
	res = res + p.get_nodeValue();
	break;
      default:
	break;
      }
    }
  
  return res;
}

String
gmetadom_Model::getNodeName(const DOM::Node& node)
{
  assert(node);
  if (!node.get_namespaceURI().null()) return node.get_localName();
  else return node.get_nodeName();
}
