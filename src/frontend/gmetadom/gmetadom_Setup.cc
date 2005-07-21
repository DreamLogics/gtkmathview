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
// http://cs.unibo.it/~lpadovan/mml-widget/, or send a mail to
// <luca.padovani@cs.unibo.it>

#include <config.h>

#include "AbstractLogger.hh"
#include "gmetadom_Setup.hh"
#include "gmetadom_Model.hh"
#include "TemplateSetup.hh"

bool
gmetadom_Setup::loadOperatorDictionary(const AbstractLogger& logger, MathMLOperatorDictionary& dictionary, const String& path)
{
  try
    {
      return TemplateSetup<gmetadom_Model>::load<MathMLOperatorDictionary, true>(logger, dictionary,
										 "operator dictionary",
										 "dictionary", path);
    }
  catch (DOM::DOMException e)
    {
      String msg = e.msg;
      logger.out(LOG_DEBUG, "caught exception: %d `%s'", e.code, msg.c_str());
      return false;
    }
}

bool
gmetadom_Setup::loadConfiguration(const AbstractLogger& logger, Configuration& conf, const String& path)
{
  try
    {
      return TemplateSetup<gmetadom_Model>::load<Configuration, false>(logger, conf,
								       "configuration", 
								       "math-engine-configuration", path);
    }
  catch (DOM::DOMException e)
    {
      String msg = e.msg;
      logger.out(LOG_DEBUG, "caught exception: %d `%s'", e.code, msg.c_str());
      return false;
    }
}
