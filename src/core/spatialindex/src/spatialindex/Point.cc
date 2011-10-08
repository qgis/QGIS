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

#include <cstring>
#include <cmath>
#include <limits>

#include "../../include/SpatialIndex.h"

using namespace SpatialIndex;

Point::Point()
    : m_dimension( 0 ), m_pCoords( 0 )
{
}

Point::Point( const double* pCoords, uint32_t dimension )
    : m_dimension( dimension )
{
  // no need to initialize m_pCoords to 0 since if a bad_alloc is raised the destructor will not be called.

  m_pCoords = new double[m_dimension];
  memcpy( m_pCoords, pCoords, m_dimension * sizeof( double ) );
}

Point::Point( const Point& p )
    : m_dimension( p.m_dimension )
{
  // no need to initialize m_pCoords to 0 since if a bad_alloc is raised the destructor will not be called.

  m_pCoords = new double[m_dimension];
  memcpy( m_pCoords, p.m_pCoords, m_dimension * sizeof( double ) );
}

Point::~Point()
{
  delete[] m_pCoords;
}

Point& Point::operator=( const Point & p )
{
  if ( this != &p )
  {
    makeDimension( p.m_dimension );
    memcpy( m_pCoords, p.m_pCoords, m_dimension * sizeof( double ) );
  }

  return *this;
}

bool Point::operator==( const Point& p ) const
{
  if ( m_dimension != p.m_dimension )
    throw Tools::IllegalArgumentException(
      "Point::operator==: Points have different number of dimensions."
    );

  for ( uint32_t i = 0; i < m_dimension; ++i )
  {
    if (
      m_pCoords[i] < p.m_pCoords[i] - std::numeric_limits<double>::epsilon() ||
      m_pCoords[i] > p.m_pCoords[i] + std::numeric_limits<double>::epsilon() )  return false;
  }

  return true;
}

//
// IObject interface
//
Point* Point::clone()
{
  return new Point( *this );
}

//
// ISerializable interface
//
uint32_t Point::getByteArraySize()
{
  return ( sizeof( uint32_t ) + m_dimension * sizeof( double ) );
}

void Point::loadFromByteArray( const byte* ptr )
{
  uint32_t dimension;
  memcpy( &dimension, ptr, sizeof( uint32_t ) );
  ptr += sizeof( uint32_t );

  makeDimension( dimension );
  memcpy( m_pCoords, ptr, m_dimension * sizeof( double ) );
  //ptr += m_dimension * sizeof(double);
}

void Point::storeToByteArray( byte** data, uint32_t& len )
{
  len = getByteArraySize();
  *data = new byte[len];
  byte* ptr = *data;

  memcpy( ptr, &m_dimension, sizeof( uint32_t ) );
  ptr += sizeof( uint32_t );
  memcpy( ptr, m_pCoords, m_dimension * sizeof( double ) );
  //ptr += m_dimension * sizeof(double);
}

//
// IShape interface
//
bool Point::intersectsShape( const IShape& s ) const
{
  const Region* pr = dynamic_cast<const Region*>( &s );
  if ( pr != 0 )
  {
    return pr->containsPoint( *this );
  }

  throw Tools::IllegalStateException(
    "Point::intersectsShape: Not implemented yet!"
  );
}

bool Point::containsShape( const IShape& ) const
{
  return false;
}

bool Point::touchesShape( const IShape& s ) const
{
  const Point* ppt = dynamic_cast<const Point*>( &s );
  if ( ppt != 0 )
  {
    if ( *this == *ppt ) return true;
    return false;
  }

  const Region* pr = dynamic_cast<const Region*>( &s );
  if ( pr != 0 )
  {
    return pr->touchesPoint( *this );
  }

  throw Tools::IllegalStateException(
    "Point::touchesShape: Not implemented yet!"
  );
}

void Point::getCenter( Point& out ) const
{
  out = *this;
}

uint32_t Point::getDimension() const
{
  return m_dimension;
}

void Point::getMBR( Region& out ) const
{
  out = Region( m_pCoords, m_pCoords, m_dimension );
}

double Point::getArea() const
{
  return 0.0;
}

double Point::getMinimumDistance( const IShape& s ) const
{
  const Point* ppt = dynamic_cast<const Point*>( &s );
  if ( ppt != 0 )
  {
    return getMinimumDistance( *ppt );
  }

  const Region* pr = dynamic_cast<const Region*>( &s );
  if ( pr != 0 )
  {
    return pr->getMinimumDistance( *this );
  }

  throw Tools::IllegalStateException(
    "Point::getMinimumDistance: Not implemented yet!"
  );
}

double Point::getMinimumDistance( const Point& p ) const
{
  if ( m_dimension != p.m_dimension )
    throw Tools::IllegalArgumentException(
      "Point::getMinimumDistance: Shapes have different number of dimensions."
    );

  double ret = 0.0;

  for ( uint32_t cDim = 0; cDim < m_dimension; ++cDim )
  {
    ret += std::pow( m_pCoords[cDim] - p.m_pCoords[cDim], 2.0 );
  }

  return std::sqrt( ret );
}

double Point::getCoordinate( uint32_t index ) const
{
  if ( index >= m_dimension )
    throw Tools::IndexOutOfBoundsException( index );

  return m_pCoords[index];
}

void Point::makeInfinite( uint32_t dimension )
{
  makeDimension( dimension );
  for ( uint32_t cIndex = 0; cIndex < m_dimension; ++cIndex )
  {
    m_pCoords[cIndex] = std::numeric_limits<double>::max();
  }
}

void Point::makeDimension( uint32_t dimension )
{
  if ( m_dimension != dimension )
  {
    delete[] m_pCoords;

    // remember that this is not a constructor. The object will be destructed normally if
    // something goes wrong (bad_alloc), so we must take care not to leave the object at an intermediate state.
    m_pCoords = 0;

    m_dimension = dimension;
    m_pCoords = new double[m_dimension];
  }
}

std::ostream& SpatialIndex::operator<<( std::ostream& os, const Point& pt )
{
  for ( uint32_t cDim = 0; cDim < pt.m_dimension; ++cDim )
  {
    os << pt.m_pCoords[cDim] << " ";
  }

  return os;
}
