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

#include <cstring>
#include <limits>
#include <Tools.h>

Tools::Geometry::LineSegment::LineSegment()
    : m_dimension( 0 ), m_pStartPoint( 0 ), m_pEndPoint( 0 )
{
}

Tools::Geometry::LineSegment::LineSegment( const double* pStartPoint, const double* pEndPoint, unsigned long dimension )
    : m_dimension( dimension )
{
  // no need to initialize arrays to 0 since if a bad_alloc is raised the destructor will not be called.

  m_pStartPoint = new double[m_dimension];
  m_pEndPoint = new double[m_dimension];
  memcpy( m_pStartPoint, pStartPoint, m_dimension * sizeof( double ) );
  memcpy( m_pEndPoint, pEndPoint, m_dimension * sizeof( double ) );
}

Tools::Geometry::LineSegment::LineSegment( const Point& startPoint, const Point& endPoint )
    : m_dimension( startPoint.m_dimension )
{
  if ( startPoint.m_dimension != endPoint.m_dimension )
    throw Tools::IllegalArgumentException(
      "Tools::Geometry::LineSegment::LineSegment: Points have different dimensionalities."
    );

  // no need to initialize arrays to 0 since if a bad_alloc is raised the destructor will not be called.

  m_pStartPoint = new double[m_dimension];
  m_pEndPoint = new double[m_dimension];
  memcpy( m_pStartPoint, startPoint.m_pCoords, m_dimension * sizeof( double ) );
  memcpy( m_pEndPoint, endPoint.m_pCoords, m_dimension * sizeof( double ) );
}

Tools::Geometry::LineSegment::LineSegment( const LineSegment& l )
    : m_dimension( l.m_dimension )
{
  // no need to initialize arrays to 0 since if a bad_alloc is raised the destructor will not be called.

  m_pStartPoint = new double[m_dimension];
  m_pEndPoint = new double[m_dimension];
  memcpy( m_pStartPoint, l.m_pStartPoint, m_dimension * sizeof( double ) );
  memcpy( m_pEndPoint, l.m_pEndPoint, m_dimension * sizeof( double ) );
}

Tools::Geometry::LineSegment::~LineSegment()
{
  delete[] m_pStartPoint;
  delete[] m_pEndPoint;
}

Tools::Geometry::LineSegment& Tools::Geometry::LineSegment::operator=( const LineSegment & l )
{
  if ( this != &l )
  {
    makeDimension( l.m_dimension );
    memcpy( m_pStartPoint, l.m_pStartPoint, m_dimension * sizeof( double ) );
    memcpy( m_pEndPoint, l.m_pEndPoint, m_dimension * sizeof( double ) );
  }

  return *this;
}

bool Tools::Geometry::LineSegment::operator==( const LineSegment& l ) const
{
  if ( m_dimension != l.m_dimension )
    throw IllegalArgumentException(
      "Tools::Geometry::LineSegment::operator==: LineSegments have different number of dimensions."
    );

  for ( unsigned long i = 0; i < m_dimension; i++ )
  {
    if (
      m_pStartPoint[i] < l.m_pStartPoint[i] - std::numeric_limits<double>::epsilon() ||
      m_pStartPoint[i] > l.m_pStartPoint[i] + std::numeric_limits<double>::epsilon() )  return false;

    if (
      m_pEndPoint[i] < l.m_pEndPoint[i] - std::numeric_limits<double>::epsilon() ||
      m_pEndPoint[i] > l.m_pEndPoint[i] + std::numeric_limits<double>::epsilon() )  return false;
  }

  return true;
}

//
// IObject interface
//
Tools::Geometry::LineSegment* Tools::Geometry::LineSegment::clone()
{
  return new LineSegment( *this );
}

//
// ISerializable interface
//
unsigned long Tools::Geometry::LineSegment::getByteArraySize()
{
  return ( sizeof( unsigned long ) + m_dimension * sizeof( double ) * 2 );
}

void Tools::Geometry::LineSegment::loadFromByteArray( const byte* ptr )
{
  unsigned long dimension;
  memcpy( &dimension, ptr, sizeof( unsigned long ) );
  ptr += sizeof( unsigned long );

  makeDimension( dimension );
  memcpy( m_pStartPoint, ptr, m_dimension * sizeof( double ) );
  ptr += m_dimension * sizeof( double );
  memcpy( m_pEndPoint, ptr, m_dimension * sizeof( double ) );
  //ptr += m_dimension * sizeof(double);
}

void Tools::Geometry::LineSegment::storeToByteArray( byte** data, unsigned long& len )
{
  len = getByteArraySize();
  *data = new byte[len];
  byte* ptr = *data;

  memcpy( ptr, &m_dimension, sizeof( unsigned long ) );
  ptr += sizeof( unsigned long );
  memcpy( ptr, m_pStartPoint, m_dimension * sizeof( double ) );
  ptr += m_dimension * sizeof( double );
  memcpy( ptr, m_pEndPoint, m_dimension * sizeof( double ) );
  //ptr += m_dimension * sizeof(double);
}

//
// IShape interface
//
bool Tools::Geometry::LineSegment::intersectsShape( const IShape& s ) const
{
  Q_UNUSED( s );
  throw IllegalStateException(
    "Tools::Geometry::LineSegment::intersectsShape: Not implemented yet!"
  );
}

bool Tools::Geometry::LineSegment::containsShape( const IShape& s ) const
{
  Q_UNUSED( s );
  return false;
}

bool Tools::Geometry::LineSegment::touchesShape( const IShape& s ) const
{
  Q_UNUSED( s );
  throw IllegalStateException(
    "Tools::Geometry::LineSegment::touchesShape: Not implemented yet!"
  );
}

void Tools::Geometry::LineSegment::getCenter( Point& out ) const
{
  double* coords = new double[m_dimension];

  for ( unsigned long cDim = 0; cDim < m_dimension; cDim++ )
  {
    coords[cDim] =
      ( qAbs( m_pStartPoint[cDim] - m_pEndPoint[cDim] ) / 2.0 ) +
      qMin( m_pStartPoint[cDim], m_pEndPoint[cDim] );
  }

  out = Point( coords, m_dimension );

  delete[] coords;
}

unsigned long Tools::Geometry::LineSegment::getDimension() const
{
  return m_dimension;
}

void Tools::Geometry::LineSegment::getMBR( Region& out ) const
{
  double* low = new double[m_dimension];
  double* high = new double[m_dimension];

  for ( unsigned long cDim = 0; cDim < m_dimension; cDim++ )
  {
    low[cDim] = qMin( m_pStartPoint[cDim], m_pEndPoint[cDim] );
    high[cDim] = qMax( m_pStartPoint[cDim], m_pEndPoint[cDim] );
  }

  out = Region( low, high, m_dimension );

  delete[] low;
  delete[] high;
}

double Tools::Geometry::LineSegment::getArea() const
{
  return 0.0;
}

double Tools::Geometry::LineSegment::getMinimumDistance( const IShape& s ) const
{
  const Point* ppt = dynamic_cast<const Point*>( &s );
  if ( ppt != 0 )
  {
    return getMinimumDistance( *ppt );
  }

  /*
   const Region* pr = dynamic_cast<const Region*>(&s);
   if (pr != 0)
   {
    return pr->getMinimumDistance(*this);
   }
  */

  throw IllegalStateException(
    "Tools::Geometry::LineSegment::getMinimumDistance: Not implemented yet!"
  );
}

double Tools::Geometry::LineSegment::getMinimumDistance( const Point& p ) const
{
  if ( m_dimension == 1 )
    throw Tools::NotSupportedException(
      "Tools::Geometry::LineSegment::getMinimumDistance: Use an Interval instead."
    );

  if ( m_dimension != 2 )
    throw Tools::NotSupportedException(
      "Tools::Geometry::LineSegment::getMinimumDistance: Distance for high dimensional spaces not supported!"
    );

  if ( m_pEndPoint[0] >= m_pStartPoint[0] - std::numeric_limits<double>::epsilon() &&
       m_pEndPoint[0] <= m_pStartPoint[0] + std::numeric_limits<double>::epsilon() ) return qAbs( p.m_pCoords[0] - m_pStartPoint[0] );

  if ( m_pEndPoint[1] >= m_pStartPoint[1] - std::numeric_limits<double>::epsilon() &&
       m_pEndPoint[1] <= m_pStartPoint[1] + std::numeric_limits<double>::epsilon() ) return qAbs( p.m_pCoords[1] - m_pStartPoint[1] );

  double x1 = m_pStartPoint[0];
  double x2 = m_pEndPoint[0];
  double x0 = p.m_pCoords[0];
  double y1 = m_pStartPoint[1];
  double y2 = m_pEndPoint[1];
  double y0 = p.m_pCoords[1];

  return qAbs(( x2 - x1 ) *( y1 - y0 ) - ( x1 - x0 ) *( y2 - y1 ) ) / ( std::sqrt(( x2 - x1 ) *( x2 - x1 ) + ( y2 - y1 ) *( y2 - y1 ) ) );
}

// assuming moving from start to end, positive distance is from right hand side.
double Tools::Geometry::LineSegment::getRelativeMinimumDistance( const Point& p ) const
{
  if ( m_dimension == 1 )
    throw Tools::NotSupportedException(
      "Tools::Geometry::LineSegment::getRelativeMinimumDistance: Use an Interval instead."
    );

  if ( m_dimension != 2 )
    throw Tools::NotSupportedException(
      "Tools::Geometry::LineSegment::getRelativeMinimumDistance: Distance for high dimensional spaces not supported!"
    );

  if ( m_pEndPoint[0] >= m_pStartPoint[0] - std::numeric_limits<double>::epsilon() &&
       m_pEndPoint[0] <= m_pStartPoint[0] + std::numeric_limits<double>::epsilon() )
  {
    if ( m_pStartPoint[1] < m_pEndPoint[1] ) return m_pStartPoint[0] - p.m_pCoords[0];
    if ( m_pStartPoint[1] >= m_pEndPoint[1] ) return p.m_pCoords[0] - m_pStartPoint[0];
  }

  if ( m_pEndPoint[1] >= m_pStartPoint[1] - std::numeric_limits<double>::epsilon() &&
       m_pEndPoint[1] <= m_pStartPoint[1] + std::numeric_limits<double>::epsilon() )
  {
    if ( m_pStartPoint[0] < m_pEndPoint[0] ) return p.m_pCoords[1] - m_pStartPoint[1];
    if ( m_pStartPoint[0] >= m_pEndPoint[0] ) return m_pStartPoint[1] - p.m_pCoords[1];
  }

  double x1 = m_pStartPoint[0];
  double x2 = m_pEndPoint[0];
  double x0 = p.m_pCoords[0];
  double y1 = m_pStartPoint[1];
  double y2 = m_pEndPoint[1];
  double y0 = p.m_pCoords[1];

  return (( x1 - x0 ) *( y2 - y1 ) - ( x2 - x1 ) *( y1 - y0 ) ) / ( std::sqrt(( x2 - x1 ) *( x2 - x1 ) + ( y2 - y1 ) *( y2 - y1 ) ) );
}

double Tools::Geometry::LineSegment::getRelativeMaximumDistance( const Tools::Geometry::Region& r ) const
{
  if ( m_dimension == 1 )
    throw Tools::NotSupportedException(
      "Tools::Geometry::LineSegment::getRelativeMaximumDistance: Use an Interval instead."
    );

  if ( m_dimension != 2 )
    throw Tools::NotSupportedException(
      "Tools::Geometry::LineSegment::getRelativeMaximumDistance: Distance for high dimensional spaces not supported!"
    );

  // clockwise.
  double d1 = getRelativeMinimumDistance( Point( r.m_pLow, 2 ) );

  double coords[2];
  coords[0] = r.m_pLow[0];
  coords[1] = r.m_pHigh[1];
  double d2 = getRelativeMinimumDistance( Point( coords, 2 ) );

  double d3 = getRelativeMinimumDistance( Point( r.m_pHigh, 2 ) );

  coords[0] = r.m_pHigh[0];
  coords[1] = r.m_pLow[1];
  double d4 = getRelativeMinimumDistance( Point( coords, 2 ) );

  return qMax( d1, qMax( d2, qMax( d3, d4 ) ) );
}

double Tools::Geometry::LineSegment::getAngleOfPerpendicularRay()
{
  if ( m_dimension == 1 )
    throw Tools::NotSupportedException(
      "Tools::Geometry::LineSegment::getAngleOfPerpendicularRay: Use an Interval instead."
    );

  if ( m_dimension != 2 )
    throw Tools::NotSupportedException(
      "Tools::Geometry::LineSegment::getAngleOfPerpendicularRay: Distance for high dimensional spaces not supported!"
    );

  if ( m_pStartPoint[0] >= m_pEndPoint[0] - std::numeric_limits<double>::epsilon() &&
       m_pStartPoint[0] <= m_pEndPoint[0] + std::numeric_limits<double>::epsilon() ) return 0.0;

  if ( m_pStartPoint[1] >= m_pEndPoint[1] - std::numeric_limits<double>::epsilon() &&
       m_pStartPoint[1] <= m_pEndPoint[1] + std::numeric_limits<double>::epsilon() ) return M_PI_2;

  return std::atan( -( m_pStartPoint[0] - m_pEndPoint[0] ) / ( m_pStartPoint[1] - m_pEndPoint[1] ) );
}

void Tools::Geometry::LineSegment::makeInfinite( unsigned long dimension )
{
  makeDimension( dimension );
  for ( unsigned long cIndex = 0; cIndex < m_dimension; cIndex++ )
  {
    m_pStartPoint[cIndex] = std::numeric_limits<double>::max();
    m_pEndPoint[cIndex] = std::numeric_limits<double>::max();
  }
}

void Tools::Geometry::LineSegment::makeDimension( unsigned long dimension )
{
  if ( m_dimension != dimension )
  {
    delete[] m_pStartPoint;
    delete[] m_pEndPoint;

    // remember that this is not a constructor. The object will be destructed normally if
    // something goes wrong (bad_alloc), so we must take care not to leave the object at an intermediate state.
    m_pStartPoint = 0;
    m_pEndPoint = 0;

    m_dimension = dimension;
    m_pStartPoint = new double[m_dimension];
    m_pEndPoint = new double[m_dimension];
  }
}

std::ostream& Tools::Geometry::operator<<( std::ostream& os, const LineSegment& l )
{
  for ( unsigned long cDim = 0; cDim < l.m_dimension; cDim++ )
  {
    os << l.m_pStartPoint[cDim] << ", " << l.m_pEndPoint[cDim] << " ";
  }

  return os;
}
