// Copyright (C) 2000-2002, Luca Padovani <luca.padovani@cs.unibo.it>.
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
// http://www.cs.unibo.it/helm/mml-widget, or send a mail to
// <luca.padovani@cs.unibo.it>

#ifndef Ptr_hh
#define Ptr_hh

template <class P>
class Ptr
{
public:
  Ptr(P* p = 0) : ptr(p) { if (ptr != 0) ptr->ref(); }
  Ptr(const Ptr& p) : ptr(p.ptr) { if (ptr != 0) ptr->ref(); }
  ~Ptr() { if (ptr != 0) ptr->unref(); }

  P* operator->() const { assert(ptr != 0); return ptr; }
  Ptr& operator=(const Ptr& p)
  { 
    if (p.ptr != 0) p.ptr->ref();
    if (ptr != 0) ptr->unref();
    ptr = p.ptr;
    return *this;
  }

  friend bool operator==(const Ptr& p, const Ptr& q) { return p.ptr == q.ptr; }
  friend bool operator!=(const Ptr& p, const Ptr& q) { return p.ptr != q.ptr; }
  template <class Q> friend Ptr<Q> smart_cast(const Ptr& p) { return Ptr<Q>(dynamic_cast<Q*>(p.ptr)); }  
  
  // NOTE: due to the following conversion operator there can be many
  // ambiguities when comparing different smart pointers
  template <class Q> operator Ptr<Q>() const { return Ptr<Q>(ptr); }

  void* get(void) const
  {
    if (ptr != 0) ptr->ref();
    return static_cast<void*>(ptr);
  }

  void set(void* p)
  {
    if (ptr != 0) ptr->unref();
    ptr = static_cast<P*>(p);
  }

private:
  P* ptr;
};

#endif // Ptr_hh
