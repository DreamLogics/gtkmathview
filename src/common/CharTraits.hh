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

#include <string>

namespace std {

  template<>
  struct char_traits<Char16>
  {
    typedef Char16 char_type;

    typedef unsigned long int_type;

    typedef streampos pos_type;
    typedef streamoff off_type;
    typedef mbstate_t state_type;
      
    static void 
    assign(char_type& __c1, const char_type& __c2)
    { __c1 = __c2; }

    static bool 
    eq(const char_type& __c1, const char_type& __c2)
    { return __c1 == __c2; }

    static bool 
    lt(const char_type& __c1, const char_type& __c2)
    { return __c1 < __c2; }

    static int 
    compare(const char_type* __s1, const char_type* __s2, size_t __n)
    { 
      for (size_t __i = 0; __i < __n; ++__i)
	if (!eq(__s1[__i], __s2[__i]))
	  return lt(__s1[__i], __s2[__i]) ? -1 : 1;
      return 0; 
    }

    static size_t
    length(const char_type* __s)
    { 
      const char_type* __p = __s; 
      while (*__p) ++__p; 
      return (__p - __s); 
    }

    static const char_type* 
    find(const char_type* __s, size_t __n, const char_type& __a)
    { 
      for (const char_type* __p = __s; size_t(__p - __s) < __n; ++__p)
	if (*__p == __a) return __p;
      return 0;
    }

    static char_type* 
    move(char_type* __s1, const char_type* __s2, size_t __n)
    { return (char_type*) memmove(__s1, __s2, __n * sizeof(char_type)); }

    static char_type* 
    copy(char_type* __s1, const char_type* __s2, size_t __n)
    { return (char_type*) memcpy(__s1, __s2, __n * sizeof(char_type)); }

    static char_type* 
    assign(char_type* __s, size_t __n, char_type __a)
    { 
      for (char_type* __p = __s; __p < __s + __n; ++__p) 
	assign(*__p, __a);
      return __s; 
    }

    static char_type 
    to_char_type(const int_type& __c)
    {
      char_type __r = { __c };
      return __r;
    }

    static int_type 
    to_int_type(const char_type& __c) 
    { return int_type(__c); }

    static bool 
    eq_int_type(const int_type& __c1, const int_type& __c2)
    { return __c1 == __c2; }

    static int_type 
    eof() { return static_cast<int_type>(-1); }

    static int_type 
    not_eof(const int_type& __c)
    { return eq_int_type(__c, eof()) ? int_type(0) : __c; }
  };

}

