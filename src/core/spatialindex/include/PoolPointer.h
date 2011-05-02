// Tools Library
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

#ifndef __tools_pool_pointer_h
#define __tools_pool_pointer_h

namespace Tools
{
  template <class X> class PointerPool;

  template <class X> class PoolPointer
  {
    public:
      explicit PoolPointer( X* p = 0 ) : m_pointer( p ), m_pPool( 0 ) { m_prev = m_next = this; }
      explicit PoolPointer( X* p, PointerPool<X>* pPool ) throw() : m_pointer( p ), m_pPool( pPool ) { m_prev = m_next = this; }
      ~PoolPointer() { release(); }
      PoolPointer( const PoolPointer& p ) throw() { acquire( p ); }
      PoolPointer& operator=( const PoolPointer& p )
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
      void relinquish() throw()
      {
        m_pPool = 0;
        m_pointer = 0;
        release();
      }

    private:
      X* m_pointer;
      mutable const PoolPointer* m_prev;
      mutable const PoolPointer* m_next;
      PointerPool<X>* m_pPool;

      void acquire( const PoolPointer& p ) throw()
      {
        m_pPool = p.m_pPool;
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
        if ( unique() )
        {
          if ( m_pPool != 0 ) m_pPool->release( m_pointer );
          else delete m_pointer;
        }
        else
        {
          m_prev->m_next = m_next;
          m_next->m_prev = m_prev;
          m_prev = m_next = 0;
        }
        m_pointer = 0;
        m_pPool = 0;
      }
  };
}

#endif /* __tools_pool_pointer_h */
