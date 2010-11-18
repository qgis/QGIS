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
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  Email:
//    mhadji@gmail.com

#include <cstring>
#include <limits>
#include <Tools.h>

Tools::Geometry::Region::Region()
    : m_dimension( 0 ), m_pLow( 0 ), m_pHigh( 0 )
{
}

Tools::Geometry::Region::Region( const double* pLow, const double* pHigh, unsigned long dimension )
{
  initialize( pLow, pHigh, dimension );
}

Tools::Geometry::Region::Region( const Point& low, const Point& high )
{
  if ( low.m_dimension != high.m_dimension )
    throw IllegalArgumentException(
      "Tools::Geometry::Region::Region: arguments have different number of dimensions."
    );

  initialize( low.m_pCoords, high.m_pCoords, low.m_dimension );
}

Tools::Geometry::Region::Region( const Region& r )
{
  initialize( r.m_pLow, r.m_pHigh, r.m_dimension );
}

void Tools::Geometry::Region::initialize( const double* pLow, const double* pHigh, unsigned long dimension )
{
  m_pLow = 0;
  m_dimension = dimension;

#ifndef NDEBUG
  for ( unsigned long cDim = 0; cDim < m_dimension; cDim++ )
  {
    if ( pLow[cDim] > pHigh[cDim] )
    {
      throw Tools::IllegalArgumentException(
        "Tools::Geometry::Region::initialize: Low point has larger coordinates than High point."
      );
    }
  }
#endif

  try
  {
    m_pLow = new double[m_dimension];
    m_pHigh = new double[m_dimension];
  }
  catch ( ... )
  {
    delete[] m_pLow;
    throw;
  }

  memcpy( m_pLow, pLow, m_dimension * sizeof( double ) );
  memcpy( m_pHigh, pHigh, m_dimension * sizeof( double ) );
}

Tools::Geometry::Region::~Region()
{
  delete[] m_pLow;
  delete[] m_pHigh;
}

Tools::Geometry::Region& Tools::Geometry::Region::operator=( const Region & r )
{
  if ( this != &r )
  {
    makeDimension( r.m_dimension );
    memcpy( m_pLow, r.m_pLow, m_dimension * sizeof( double ) );
    memcpy( m_pHigh, r.m_pHigh, m_dimension * sizeof( double ) );
  }

  return *this;
}

bool Tools::Geometry::Region::operator==( const Region& r ) const
{
  if ( m_dimension != r.m_dimension )
    throw IllegalArgumentException(
      "Tools::Geometry::Region::operator==: Regions have different number of dimensions."
    );

  for ( unsigned long i = 0; i < m_dimension; i++ )
  {
    if (
      m_pLow[i] < r.m_pLow[i] - std::numeric_limits<double>::epsilon() ||
      m_pLow[i] > r.m_pLow[i] + std::numeric_limits<double>::epsilon() ||
      m_pHigh[i] < r.m_pHigh[i] - std::numeric_limits<double>::epsilon() ||
      m_pHigh[i] > r.m_pHigh[i] + std::numeric_limits<double>::epsilon() )
      return false;
  }
  return true;
}

//
// IObject interface
//
Tools::Geometry::Region* Tools::Geometry::Region::clone()
{
  return new Region( *this );
}

//
// ISerializable interface
//
unsigned long Tools::Geometry::Region::getByteArraySize()
{
  return ( sizeof( unsigned long ) + 2 * m_dimension * sizeof( double ) );
}

void Tools::Geometry::Region::loadFromByteArray( const byte* ptr )
{
  unsigned long dimension;
  memcpy( &dimension, ptr, sizeof( unsigned long ) );
  ptr += sizeof( unsigned long );

  makeDimension( dimension );
  memcpy( m_pLow, ptr, m_dimension * sizeof( double ) );
  ptr += m_dimension * sizeof( double );
  memcpy( m_pHigh, ptr, m_dimension * sizeof( double ) );
  //ptr += m_dimension * sizeof(double);
}

void Tools::Geometry::Region::storeToByteArray( byte** data, unsigned long& len )
{
  len = getByteArraySize();
  *data = new byte[len];
  byte* ptr = *data;

  memcpy( ptr, &m_dimension, sizeof( unsigned long ) );
  ptr += sizeof( unsigned long );
  memcpy( ptr, m_pLow, m_dimension * sizeof( double ) );
  ptr += m_dimension * sizeof( double );
  memcpy( ptr, m_pHigh, m_dimension * sizeof( double ) );
  //ptr += m_dimension * sizeof(double);
}

//
// IShape interface
//
bool Tools::Geometry::Region::intersectsShape( const IShape& s ) const
{
  const Region* pr = dynamic_cast<const Region*>( &s );
  if ( pr != 0 ) return intersectsRegion( *pr );

  const Point* ppt = dynamic_cast<const Point*>( &s );
  if ( ppt != 0 ) return containsPoint( *ppt );

  throw IllegalStateException(
    "Tools::Geometry::Region::intersectsShape: Not implemented yet!"
  );
}

bool Tools::Geometry::Region::containsShape( const IShape& s ) const
{
  const Region* pr = dynamic_cast<const Region*>( &s );
  if ( pr != 0 ) return containsRegion( *pr );

  const Point* ppt = dynamic_cast<const Point*>( &s );
  if ( ppt != 0 ) return containsPoint( *ppt );

  throw IllegalStateException(
    "Tools::Geometry::Region::containsShape: Not implemented yet!"
  );
}

bool Tools::Geometry::Region::touchesShape( const IShape& s ) const
{
  const Region* pr = dynamic_cast<const Region*>( &s );
  if ( pr != 0 ) return touchesRegion( *pr );

  const Point* ppt = dynamic_cast<const Point*>( &s );
  if ( ppt != 0 ) return touchesPoint( *ppt );

  throw IllegalStateException(
    "Tools::Geometry::Region::touchesShape: Not implemented yet!"
  );
}

void Tools::Geometry::Region::getCenter( Point& out ) const
{
  out.makeDimension( m_dimension );
  for ( unsigned long i = 0; i < m_dimension; i++ )
  {
    out.m_pCoords[i] = ( m_pLow[i] + m_pHigh[i] ) / 2.0;
  }
}

unsigned long Tools::Geometry::Region::getDimension() const
{
  return m_dimension;
}

void Tools::Geometry::Region::getMBR( Region& out ) const
{
  out = *this;
}

double Tools::Geometry::Region::getArea() const
{
  double area = 1.0;

  for ( unsigned long i = 0; i < m_dimension; i++ )
  {
    area *= m_pHigh[i] - m_pLow[i];
  }

  return area;
}

double Tools::Geometry::Region::getMinimumDistance( const IShape& s ) const
{
  const Region* pr = dynamic_cast<const Region*>( &s );
  if ( pr != 0 ) return getMinimumDistance( *pr );

  const Point* ppt = dynamic_cast<const Point*>( &s );
  if ( ppt != 0 ) return getMinimumDistance( *ppt );

  throw IllegalStateException(
    "Tools::Geometry::Region::getMinimumDistance: Not implemented yet!"
  );
}

bool Tools::Geometry::Region::intersectsRegion( const Region& r ) const
{
  if ( m_dimension != r.m_dimension )
    throw IllegalArgumentException(
      "Tools::Geometry::Region::intersectsRegion: Regions have different number of dimensions."
    );

  for ( unsigned long i = 0; i < m_dimension; i++ )
  {
    if ( m_pLow[i] > r.m_pHigh[i] || m_pHigh[i] < r.m_pLow[i] ) return false;
  }
  return true;
}

bool Tools::Geometry::Region::containsRegion( const Region& r ) const
{
  if ( m_dimension != r.m_dimension )
    throw IllegalArgumentException(
      "Tools::Geometry::Region::containsRegion: Regions have different number of dimensions."
    );

  for ( unsigned long i = 0; i < m_dimension; i++ )
  {
    if ( m_pLow[i] > r.m_pLow[i] || m_pHigh[i] < r.m_pHigh[i] ) return false;
  }
  return true;
}

bool Tools::Geometry::Region::touchesRegion( const Region& r ) const
{
  if ( m_dimension != r.m_dimension )
    throw IllegalArgumentException(
      "Tools::Geometry::Region::touchesRegion: Regions have different number of dimensions."
    );

  for ( unsigned long i = 0; i < m_dimension; i++ )
  {
    if (
      ( m_pLow[i] >= r.m_pLow[i] - std::numeric_limits<double>::epsilon() &&
        m_pLow[i] <= r.m_pLow[i] + std::numeric_limits<double>::epsilon() ) ||
      ( m_pHigh[i] >= r.m_pHigh[i] - std::numeric_limits<double>::epsilon() &&
        m_pHigh[i] <= r.m_pHigh[i] + std::numeric_limits<double>::epsilon() ) )
      return true;
  }
  return false;
}

double Tools::Geometry::Region::getMinimumDistance( const Region& r ) const
{
  if ( m_dimension != r.m_dimension )
    throw IllegalArgumentException(
      "Tools::Geometry::Region::getMinimumDistance: Regions have different number of dimensions."
    );

  double ret = 0.0;

  for ( unsigned long i = 0; i < m_dimension; i++ )
  {
    double x = 0.0;

    if ( r.m_pHigh[i] < m_pLow[i] )
    {
      x = qAbs( r.m_pHigh[i] - m_pLow[i] );
    }
    else if ( m_pHigh[i] < r.m_pLow[i] )
    {
      x = qAbs( r.m_pLow[i] - m_pHigh[i] );
    }

    ret += x * x;
  }

  return std::sqrt( ret );
}

bool Tools::Geometry::Region::containsPoint( const Point& p ) const
{
  if ( m_dimension != p.m_dimension )
    throw IllegalArgumentException(
      "Tools::Geometry::Region::containsPoint: Point has different number of dimensions."
    );

  for ( unsigned long i = 0; i < m_dimension; i++ )
  {
    if ( m_pLow[i] > p.getCoordinate( i ) || m_pHigh[i] < p.getCoordinate( i ) ) return false;
  }
  return true;
}

bool Tools::Geometry::Region::touchesPoint( const Point& p ) const
{
  if ( m_dimension != p.m_dimension )
    throw IllegalArgumentException(
      "Tools::Geometry::Region::touchesPoint: Point has different number of dimensions."
    );

  for ( unsigned long i = 0; i < m_dimension; i++ )
  {
    if (
      ( m_pLow[i] >= p.getCoordinate( i ) - std::numeric_limits<double>::epsilon() &&
        m_pLow[i] <= p.getCoordinate( i ) + std::numeric_limits<double>::epsilon() ) ||
      ( m_pHigh[i] >= p.getCoordinate( i ) - std::numeric_limits<double>::epsilon() &&
        m_pHigh[i] <= p.getCoordinate( i ) + std::numeric_limits<double>::epsilon() ) )
      return true;
  }
  return false;
}

double Tools::Geometry::Region::getMinimumDistance( const Point& p ) const
{
  if ( m_dimension != p.m_dimension )
    throw IllegalArgumentException(
      "Tools::Geometry::Region::getMinimumDistance: Point has different number of dimensions."
    );

  double ret = 0.0;

  for ( unsigned long i = 0; i < m_dimension; i++ )
  {
    if ( p.getCoordinate( i ) < m_pLow[i] )
    {
      ret += std::pow( m_pLow[i] - p.getCoordinate( i ), 2.0 );
    }
    else if ( p.getCoordinate( i ) > m_pHigh[i] )
    {
      ret += std::pow( p.getCoordinate( i ) - m_pHigh[i], 2.0 );
    }
  }

  return std::sqrt( ret );
}

Tools::Geometry::Region Tools::Geometry::Region::getIntersectingRegion( const Region& r ) const
{
  if ( m_dimension != r.m_dimension )
    throw IllegalArgumentException(
      "Tools::Geometry::Region::getIntersectingRegion: Regions have different number of dimensions."
    );

  Region ret;
  ret.makeInfinite( m_dimension );

  // check for intersection.
  // marioh: avoid function call since this is called billions of times.
  for ( unsigned long cDim = 0; cDim < m_dimension; cDim++ )
  {
    if ( m_pLow[cDim] > r.m_pHigh[cDim] || m_pHigh[cDim] < r.m_pLow[cDim] ) return ret;
  }

  for ( unsigned long cDim = 0; cDim < m_dimension; cDim++ )
  {
    ret.m_pLow[cDim] = std::max( m_pLow[cDim], r.m_pLow[cDim] );
    ret.m_pHigh[cDim] = std::min( m_pHigh[cDim], r.m_pHigh[cDim] );
  }

  return ret;
}

double Tools::Geometry::Region::getIntersectingArea( const Region& r ) const
{
  if ( m_dimension != r.m_dimension )
    throw IllegalArgumentException(
      "Tools::Geometry::Region::getIntersectingArea: Regions have different number of dimensions."
    );

  double ret = 1.0;
  double f1, f2;

  for ( unsigned long cDim = 0; cDim < m_dimension; cDim++ )
  {
    if ( m_pLow[cDim] > r.m_pHigh[cDim] || m_pHigh[cDim] < r.m_pLow[cDim] ) return 0.0;

    f1 = std::max( m_pLow[cDim], r.m_pLow[cDim] );
    f2 = std::min( m_pHigh[cDim], r.m_pHigh[cDim] );
    ret *= f2 - f1;
  }

  return ret;
}

/*
 * Returns the margin of a region. It is calcuated as the sum of  2^(d-1) * width, in each dimension.
 * It is actually the sum of all edges, no matter what the dimensionality is.
*/
double Tools::Geometry::Region::getMargin() const
{
  double mul = std::pow( 2.0, static_cast<double>( m_dimension ) - 1.0 );
  double margin = 0.0;

  for ( unsigned long i = 0; i < m_dimension; i++ )
  {
    margin += ( m_pHigh[i] - m_pLow[i] ) * mul;
  }

  return margin;
}

void Tools::Geometry::Region::combineRegion( const Region& r )
{
  if ( m_dimension != r.m_dimension )
    throw IllegalArgumentException(
      "Tools::Geometry::Region::combineRegion: Region has different number of dimensions."
    );

  for ( unsigned long cDim = 0; cDim < m_dimension; cDim++ )
  {
    m_pLow[cDim] = std::min( m_pLow[cDim], r.m_pLow[cDim] );
    m_pHigh[cDim] = std::max( m_pHigh[cDim], r.m_pHigh[cDim] );
  }
}

void Tools::Geometry::Region::combinePoint( const Point& p )
{
  if ( m_dimension != p.m_dimension )
    throw IllegalArgumentException(
      "Tools::Geometry::Region::combinePoint: Point has different number of dimensions."
    );

  for ( unsigned long cDim = 0; cDim < m_dimension; cDim++ )
  {
    m_pLow[cDim] = std::min( m_pLow[cDim], p.m_pCoords[cDim] );
    m_pHigh[cDim] = std::max( m_pHigh[cDim], p.m_pCoords[cDim] );
  }
}

void Tools::Geometry::Region::getCombinedRegion( Region& out, const Region& in ) const
{
  if ( m_dimension != in.m_dimension )
    throw IllegalArgumentException(
      "Tools::Geometry::Region::getCombinedRegion: Regions have different number of dimensions."
    );

  out = *this;
  out.combineRegion( in );
}

double Tools::Geometry::Region::getLow( unsigned long index ) const
{
  if ( index < 0 || index >= m_dimension )
    throw IndexOutOfBoundsException( index );

  return m_pLow[index];
}

double Tools::Geometry::Region::getHigh( unsigned long index ) const
{
  if ( index < 0 || index >= m_dimension )
    throw IndexOutOfBoundsException( index );

  return m_pHigh[index];
}

void Tools::Geometry::Region::makeInfinite( unsigned long dimension )
{
  makeDimension( dimension );
  for ( unsigned long cIndex = 0; cIndex < m_dimension; cIndex++ )
  {
    m_pLow[cIndex] = std::numeric_limits<double>::max();
    m_pHigh[cIndex] = -std::numeric_limits<double>::max();
  }
}

void Tools::Geometry::Region::makeDimension( unsigned long dimension )
{
  if ( m_dimension != dimension )
  {
    delete[] m_pLow;
    delete[] m_pHigh;

    // remember that this is not a constructor. The object will be destructed normally if
    // something goes wrong (bad_alloc), so we must take care not to leave the object at an intermediate state.
    m_pLow = 0; m_pHigh = 0;

    m_dimension = dimension;
    m_pLow = new double[m_dimension];
    m_pHigh = new double[m_dimension];
  }
}

std::ostream& Tools::Geometry::operator<<( std::ostream& os, const Region& r )
{
  unsigned long i;

  os << "Low: ";
  for ( i = 0; i < r.m_dimension; i++ )
  {
    os << r.m_pLow[i] << " ";
  }

  os << ", High: ";

  for ( i = 0; i < r.m_dimension; i++ )
  {
    os << r.m_pHigh[i] << " ";
  }

  return os;
}
