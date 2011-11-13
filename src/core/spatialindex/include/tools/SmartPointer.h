// Spatial Index Library
//
// Copyright (C) 2004  Navel Ltd.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  Email:
//    mhadji@gmail.com

#pragma once

namespace Tools
{
  template <class X> class SmartPointer
  {
    public:
      explicit SmartPointer( X* p = 0 ) throw() : m_pointer( p ) { m_prev = m_next = this; }
      ~SmartPointer() { release(); }
      SmartPointer( const SmartPointer& p ) throw() { acquire( p ); }
      SmartPointer& operator=( const SmartPointer& p )
      {
        if ( this != &p )
        {
          release();
          acquire( p );
        }
        return *this;
      }

      X& operator*() const throw() { return *m_pointer; }
      X* operator->() const throw() { return m_pointer; }
      X* get() const throw() { return m_pointer; }
      bool unique() const throw() { return m_prev ? m_prev == this : true; }

    private:
      X* m_pointer;
      mutable const SmartPointer* m_prev;
      mutable const SmartPointer* m_next;

      void acquire( const SmartPointer& p ) throw()
      {
        m_pointer = p.m_pointer;
        m_next = p.m_next;
        m_next->m_prev = this;
        m_prev = &p;
#ifndef mutable
        p.m_next = this;
#else
        ( const_cast<linked_ptr<X>*>( &p ) )->m_next = this;
#endif
      }

      void release()
      {
        if ( unique() ) delete m_pointer;
        else
        {
          m_prev->m_next = m_next;
          m_next->m_prev = m_prev;
          m_prev = m_next = 0;
        }
        m_pointer = 0;
      }
  };
}

