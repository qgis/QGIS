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

namespace SpatialIndex
{
  class SIDX_DLL Point : public Tools::IObject, public virtual IShape
  {
    public:
      Point();
      Point( const double* pCoords, uint32_t dimension );
      Point( const Point& p );
      virtual ~Point();

      virtual Point& operator=( const Point& p );
      virtual bool operator==( const Point& p ) const;

      //
      // IObject interface
      //
      virtual Point* clone();

      //
      // ISerializable interface
      //
      virtual uint32_t getByteArraySize();
      virtual void loadFromByteArray( const byte* data );
      virtual void storeToByteArray( byte** data, uint32_t& length );

      //
      // IShape interface
      //
      virtual bool intersectsShape( const IShape& in ) const;
      virtual bool containsShape( const IShape& in ) const;
      virtual bool touchesShape( const IShape& in ) const;
      virtual void getCenter( Point& out ) const;
      virtual uint32_t getDimension() const;
      virtual void getMBR( Region& out ) const;
      virtual double getArea() const;
      virtual double getMinimumDistance( const IShape& in ) const;

      virtual double getMinimumDistance( const Point& p ) const;

      virtual double getCoordinate( uint32_t index ) const;

      virtual void makeInfinite( uint32_t dimension );
      virtual void makeDimension( uint32_t dimension );

    public:
      uint32_t m_dimension;
      double* m_pCoords;

      friend class Region;
      friend SIDX_DLL std::ostream& operator<<( std::ostream& os, const Point& pt );
  }; // Point

  SIDX_DLL std::ostream& operator<<( std::ostream& os, const Point& pt );
}
