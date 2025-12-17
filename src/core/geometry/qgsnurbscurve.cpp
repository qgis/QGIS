/***************************************************************************
                         qgsnurbscurve.cpp
                         -----------------
    begin                : September 2025
    copyright            : (C) 2025 by Loïc Bartoletti
    email                : loic dot bartoletti at oslandia dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsnurbscurve.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <memory>
#include <nlohmann/json.hpp>

#include "qgsapplication.h"
#include "qgsbox3d.h"
#include "qgscoordinatetransform.h"
#include "qgsfeedback.h"
#include "qgsgeometrytransformer.h"
#include "qgsgeometryutils.h"
#include "qgsgeometryutils_base.h"
#include "qgslinestring.h"
#include "qgspoint.h"
#include "qgsrectangle.h"
#include "qgswkbptr.h"
#include "qgswkbtypes.h"

#include <QPainterPath>

using namespace nlohmann;

QgsNurbsCurve::QgsNurbsCurve()
{
  mWkbType = Qgis::WkbType::NurbsCurve;
}

QgsNurbsCurve::QgsNurbsCurve( const QVector<QgsPoint> &ctrlPoints, int degree, const QVector<double> &knots, const QVector<double> &weights, bool closed )
  : mControlPoints( ctrlPoints )
  , mKnots( knots )
  , mWeights( weights )
  , mDegree( degree )
  , mClosed( closed )
{
  mWkbType = Qgis::WkbType::NurbsCurve;

  // Update WKB type based on coordinate dimensions
  if ( !mControlPoints.isEmpty() )
  {
    const QgsPoint &firstPoint = mControlPoints.first();
    if ( firstPoint.is3D() )
      mWkbType = QgsWkbTypes::addZ( mWkbType );
    if ( firstPoint.isMeasure() )
      mWkbType = QgsWkbTypes::addM( mWkbType );
  }
}

QgsCurve *QgsNurbsCurve::clone() const
{
  return new QgsNurbsCurve( *this );
}

/**
 * \brief Find the knot span index for parameter u using binary search.
 *
 * This is Algorithm A2.1 from "The NURBS Book" (Piegl & Tiller).
 * Returns the index i such that knots[i] <= u < knots[i+1].
 * For a NURBS curve with n control points and degree p, valid spans
 * are in range [p, n-1].
 *
 * \param degree polynomial degree of the NURBS curve
 * \param u parameter value to locate
 * \param knots knot vector
 * \param nPoints number of control points
 * \returns knot span index
 */
static int findKnotSpan( const int degree, const double u, const QVector<double> &knots, const int nPoints )
{
  // Special case: u at or beyond end of parameter range
  if ( u >= knots[nPoints] )
    return nPoints - 1;

  // Special case: u at or before start of parameter range
  if ( u <= knots[degree] )
    return degree;

  // Binary search for the knot span
  int low = degree;
  int high = nPoints;
  int mid = ( low + high ) / 2;

  while ( u < knots[mid] || u >= knots[mid + 1] )
  {
    if ( u < knots[mid] )
      high = mid;
    else
      low = mid;
    mid = ( low + high ) / 2;
  }

  return mid;
}

// Evaluates the NURBS curve at parameter t using De Boor's algorithm.
// Implements Algorithm A4.1 from "The NURBS Book" (Piegl & Tiller).
QgsPoint QgsNurbsCurve::evaluate( double t ) const
{
  const int n = mControlPoints.size();
  if ( n == 0 )
  {
    return QgsPoint();
  }

  // Clamp parameter t to valid range [0,1]
  if ( t <= 0.0 )
    return mControlPoints.first();
  if ( t >= 1.0 )
    return mControlPoints.last();

  const bool hasZ = !mControlPoints.isEmpty() && mControlPoints.first().is3D();
  const bool hasM = !mControlPoints.isEmpty() && mControlPoints.first().isMeasure();

  // Remap parameter from [0,1] to knot vector range [knots[degree], knots[n]]
  const double u = mKnots[mDegree] + t * ( mKnots[n] - mKnots[mDegree] );

  // Find the knot span containing parameter u (Algorithm A2.1)
  const int span = findKnotSpan( mDegree, u, mKnots, n );

  // Temporary arrays for De Boor iteration (degree+1 points)
  // Using homogeneous coordinates: (w*x, w*y, w*z, w) for rational curves
  QVector<double> tempX( mDegree + 1 );
  QVector<double> tempY( mDegree + 1 );
  QVector<double> tempZ( mDegree + 1 );
  QVector<double> tempM( mDegree + 1 );
  QVector<double> tempW( mDegree + 1 );

  // Initialize temp arrays with control points and weights
  for ( int j = 0; j <= mDegree; ++j )
  {
    const int cpIdx = span - mDegree + j;
    const QgsPoint &cp = mControlPoints[cpIdx];
    const double w = ( cpIdx < mWeights.size() ) ? mWeights[cpIdx] : 1.0;

    // Store in homogeneous coordinates (w * P)
    tempX[j] = cp.x() * w;
    tempY[j] = cp.y() * w;
    tempZ[j] = hasZ ? cp.z() * w : 0.0;
    tempM[j] = hasM ? cp.m() : 0.0; // M is not weighted
    tempW[j] = w;
  }

  // De Boor iteration (Algorithm A4.1) in homogeneous space
  for ( int k = 1; k <= mDegree; ++k )
  {
    for ( int j = mDegree; j >= k; --j )
    {
      const int knotIdx = span - mDegree + j;
      const double denom = mKnots[knotIdx + mDegree - k + 1] - mKnots[knotIdx];

      if ( !qgsDoubleNear( denom, 0.0 ) )
      {
        const double alpha = ( u - mKnots[knotIdx] ) / denom;
        const double oneMinusAlpha = 1.0 - alpha;

        // Linear interpolation in homogeneous space
        tempX[j] = oneMinusAlpha * tempX[j - 1] + alpha * tempX[j];
        tempY[j] = oneMinusAlpha * tempY[j - 1] + alpha * tempY[j];
        if ( hasZ )
          tempZ[j] = oneMinusAlpha * tempZ[j - 1] + alpha * tempZ[j];
        if ( hasM )
          tempM[j] = oneMinusAlpha * tempM[j - 1] + alpha * tempM[j];

        // Interpolate weights
        tempW[j] = oneMinusAlpha * tempW[j - 1] + alpha * tempW[j];
      }
    }
  }

  // Result is in temp[degree], stored in homogeneous coordinates
  // Project back to Cartesian by dividing by weight
  double x = tempX[mDegree];
  double y = tempY[mDegree];
  double z = tempZ[mDegree];
  double m = tempM[mDegree];
  const double w = tempW[mDegree];

  if ( !qgsDoubleNear( w, 0.0 ) && !qgsDoubleNear( w, 1.0 ) )
  {
    x /= w;
    y /= w;
    if ( hasZ )
      z /= w;
    // M is not divided by weight (it's not in homogeneous space)
  }

  // Create point with appropriate dimensionality
  if ( hasZ && hasM )
    return QgsPoint( x, y, z, m );
  else if ( hasZ )
    return QgsPoint( x, y, z );
  else if ( hasM )
    return QgsPoint( x, y, std::numeric_limits<double>::quiet_NaN(), m );
  else
    return QgsPoint( x, y );
}

bool QgsNurbsCurve::isBezier() const
{
  // A Bézier curve is a special case of NURBS:
  // - Non-rational (all weights equal to 1.0)
  // - Number of control points equals degree + 1
  // - Open uniform knot vector
  return ( !isRational() && mDegree > 0 && mControlPoints.size() == mDegree + 1 );
}

bool QgsNurbsCurve::isBSpline() const
{
  return !isRational();
}

bool QgsNurbsCurve::isRational() const
{
  constexpr double epsilon = 1e-8;
  for ( const double w : mWeights )
  {
    if ( std::fabs( w - 1.0 ) > epsilon )
    {
      return true;
    }
  }
  return false;
}

bool QgsNurbsCurve::isClosed() const
{
  if ( mClosed )
    return true;

  if ( mControlPoints.size() < 2 )
    return false;

  // Check if curve endpoints are the same by evaluating at t=0 and t=1
  const QgsPoint startPt = evaluate( 0.0 );
  const QgsPoint endPt = evaluate( 1.0 );

  bool closed = qgsDoubleNear( startPt.x(), endPt.x() ) && qgsDoubleNear( startPt.y(), endPt.y() );

  if ( is3D() && closed )
    closed &= qgsDoubleNear( startPt.z(), endPt.z() ) || ( std::isnan( startPt.z() ) && std::isnan( endPt.z() ) );

  return closed;
}

bool QgsNurbsCurve::isClosed2D() const
{
  if ( mClosed )
    return true;

  if ( mControlPoints.size() < 2 )
    return false;

  // Check if curve endpoints are the same in 2D
  const QgsPoint startPt = evaluate( 0.0 );
  const QgsPoint endPt = evaluate( 1.0 );

  return qgsDoubleNear( startPt.x(), endPt.x() ) && qgsDoubleNear( startPt.y(), endPt.y() );
}

QgsLineString *QgsNurbsCurve::curveToLine( double tolerance, SegmentationToleranceType toleranceType ) const
{
  Q_UNUSED( toleranceType );

  // Determine number of segments based on tolerance (angular approximation)
  // For NURBS curves, we use uniform parameterization as a first approximation
  const int steps = std::max( 2, static_cast<int>( 2 * M_PI / tolerance ) );

  auto line = new QgsLineString();
  for ( int i = 0; i <= steps; ++i )
  {
    const double t = static_cast<double>( i ) / steps;
    const QgsPoint pt = evaluate( t );
    line->addVertex( pt );
  }

  return line;
}

void QgsNurbsCurve::draw( QPainter &p ) const
{
  std::unique_ptr<QgsLineString> line( curveToLine() );
  if ( line )
    line->draw( p );
}

void QgsNurbsCurve::drawAsPolygon( QPainter &p ) const
{
  std::unique_ptr<QgsLineString> line( curveToLine() );
  if ( line )
    line->drawAsPolygon( p );
}

QPolygonF QgsNurbsCurve::asQPolygonF() const
{
  std::unique_ptr<QgsLineString> line( curveToLine() );
  return line ? line->asQPolygonF() : QPolygonF();
}

QgsPoint QgsNurbsCurve::endPoint() const
{
  return mControlPoints.isEmpty() ? QgsPoint() : mControlPoints.last();
}

bool QgsNurbsCurve::equals( const QgsCurve &other ) const
{
  if ( geometryType() != other.geometryType() )
  {
    return false;
  }

  const QgsNurbsCurve *o = qgsgeometry_cast<const QgsNurbsCurve *>( &other );
  if ( !o )
    return false;

  if ( o->mDegree != mDegree || o->mClosed != mClosed || o->mControlPoints.size() != mControlPoints.size() )
  {
    return false;
  }

  if ( o->mWeights.size() != mWeights.size() || o->mKnots.size() != mKnots.size() )
  {
    return false;
  }

  for ( int i = 0; i < mControlPoints.size(); ++i )
  {
    if ( mControlPoints[i] != o->mControlPoints[i] )
      return false;
    if ( i < mWeights.size() && i < o->mWeights.size() )
    {
      if ( !qgsDoubleNear( mWeights[i], o->mWeights[i] ) )
        return false;
    }
  }

  for ( int i = 0; i < mKnots.size(); ++i )
  {
    if ( !qgsDoubleNear( mKnots[i], o->mKnots[i] ) )
      return false;
  }

  return true;
}

int QgsNurbsCurve::indexOf( const QgsPoint &point ) const
{
  for ( int i = 0; i < mControlPoints.size(); ++i )
  {
    if ( qgsDoubleNear( mControlPoints[i].distance( point ), 0.0 ) )
    {
      return i;
    }
  }
  return -1;
}

QgsPoint *QgsNurbsCurve::interpolatePoint( double distance ) const
{
  std::unique_ptr<QgsLineString> line( curveToLine() );
  if ( !line )
  {
    return nullptr;
  }
  return line->interpolatePoint( distance );
}

int QgsNurbsCurve::numPoints() const
{
  return mControlPoints.size();
}

bool QgsNurbsCurve::pointAt( int node, QgsPoint &point, Qgis::VertexType &type ) const
{
  if ( node < 0 || node >= mControlPoints.size() )
  {
    return false;
  }
  point = mControlPoints[node];
  type = Qgis::VertexType::Segment;
  return true;
}

void QgsNurbsCurve::points( QgsPointSequence &pts ) const
{
  for ( const QgsPoint &p : mControlPoints )
  {
    pts.append( p );
  }
}

QgsCurve *QgsNurbsCurve::reversed() const
{
  auto rev = new QgsNurbsCurve( *this );
  std::reverse( rev->mControlPoints.begin(), rev->mControlPoints.end() );
  std::reverse( rev->mWeights.begin(), rev->mWeights.end() );

  // Reverse and remap knot vector: new_knot[i] = max_knot + min_knot - old_knot[n-1-i]
  if ( !rev->mKnots.isEmpty() )
  {
    const double maxKnot = rev->mKnots.last();
    const double minKnot = rev->mKnots.first();
    std::reverse( rev->mKnots.begin(), rev->mKnots.end() );
    for ( double &knot : rev->mKnots )
    {
      knot = maxKnot + minKnot - knot;
    }
  }

  return rev;
}

void QgsNurbsCurve::scroll( int firstVertexIndex )
{
  // Scrolling only makes sense for closed curves
  if ( !mClosed || firstVertexIndex <= 0 || firstVertexIndex >= mControlPoints.size() )
  {
    return;
  }

  // Rotate control points and weights
  std::rotate( mControlPoints.begin(), mControlPoints.begin() + firstVertexIndex, mControlPoints.end() );
  std::rotate( mWeights.begin(), mWeights.begin() + firstVertexIndex, mWeights.end() );

  // Rotate knot vector and adjust values to preserve parameter domain
  if ( !mKnots.isEmpty() && firstVertexIndex < mKnots.size() )
  {
    const double delta = mKnots[firstVertexIndex] - mKnots[0];
    std::rotate( mKnots.begin(), mKnots.begin() + firstVertexIndex, mKnots.end() );
    // Shift all knot values by -delta to preserve the start parameter
    for ( double &knot : mKnots )
    {
      knot -= delta;
    }
  }

  clearCache();
}

std::tuple<std::unique_ptr<QgsCurve>, std::unique_ptr<QgsCurve>>
    QgsNurbsCurve::splitCurveAtVertex( int index ) const
{
  std::unique_ptr<QgsLineString> line( curveToLine() );
  if ( !line )
  {
    return std::make_tuple( nullptr, nullptr );
  }
  return line->splitCurveAtVertex( index );
}

QgsPoint QgsNurbsCurve::startPoint() const
{
  return mControlPoints.isEmpty() ? QgsPoint() : mControlPoints.first();
}

void QgsNurbsCurve::sumUpArea( double &sum ) const
{
  std::unique_ptr<QgsLineString> line( curveToLine() );
  if ( line )
    line->sumUpArea( sum );
}

double QgsNurbsCurve::xAt( int index ) const
{
  if ( index < 0 || index >= mControlPoints.size() )
    return 0.0;
  return mControlPoints[index].x();
}

double QgsNurbsCurve::yAt( int index ) const
{
  if ( index < 0 || index >= mControlPoints.size() )
    return 0.0;
  return mControlPoints[index].y();
}

double QgsNurbsCurve::zAt( int index ) const
{
  if ( index < 0 || index >= mControlPoints.size() )
    return 0.0;
  return mControlPoints[index].is3D() ? mControlPoints[index].z() : std::numeric_limits<double>::quiet_NaN();
}

double QgsNurbsCurve::mAt( int index ) const
{
  if ( index < 0 || index >= mControlPoints.size() )
    return 0.0;
  return mControlPoints[index].isMeasure() ? mControlPoints[index].m() : std::numeric_limits<double>::quiet_NaN();
}

bool QgsNurbsCurve::addZValue( double zValue )
{
  if ( QgsWkbTypes::hasZ( mWkbType ) )
    return false;

  for ( QgsPoint &p : mControlPoints )
  {
    if ( !p.is3D() )
      p.setZ( zValue );
  }

  mWkbType = QgsWkbTypes::addZ( mWkbType );
  clearCache();
  return true;
}

bool QgsNurbsCurve::addMValue( double mValue )
{
  if ( QgsWkbTypes::hasM( mWkbType ) )
    return false;

  for ( QgsPoint &p : mControlPoints )
  {
    if ( !p.isMeasure() )
      p.setM( mValue );
  }

  mWkbType = QgsWkbTypes::addM( mWkbType );
  clearCache();
  return true;
}

bool QgsNurbsCurve::dropZValue()
{
  if ( !is3D() )
    return false;

  for ( QgsPoint &p : mControlPoints )
  {
    p.setZ( std::numeric_limits<double>::quiet_NaN() );
  }

  mWkbType = QgsWkbTypes::dropZ( mWkbType );
  clearCache();
  return true;
}

bool QgsNurbsCurve::dropMValue()
{
  if ( !isMeasure() )
    return false;

  for ( QgsPoint &p : mControlPoints )
  {
    p.setM( std::numeric_limits<double>::quiet_NaN() );
  }

  mWkbType = QgsWkbTypes::dropM( mWkbType );
  clearCache();
  return true;
}

bool QgsNurbsCurve::deleteVertex( QgsVertexId position )
{
  if ( position.part != 0 || position.ring != 0 )
  {
    return false;
  }
  const int idx = position.vertex;
  if ( idx < 0 || idx >= mControlPoints.size() )
  {
    return false;
  }
  mControlPoints.remove( idx );
  if ( idx < mWeights.size() )
  {
    mWeights.remove( idx );
  }
  clearCache();
  return true;
}

void QgsNurbsCurve::filterVertices( const std::function<bool( const QgsPoint & )> &filter )
{
  QVector<QgsPoint> newPts;
  QVector<double> newWeights;
  for ( int i = 0; i < mControlPoints.size(); ++i )
  {
    if ( filter( mControlPoints[i] ) )
    {
      newPts.append( mControlPoints[i] );
      if ( i < mWeights.size() )
        newWeights.append( mWeights[i] );
    }
  }
  mControlPoints = newPts;
  mWeights = newWeights;
  clearCache();
}

bool QgsNurbsCurve::fromWkb( QgsConstWkbPtr &wkb )
{
  clear();

  if ( !wkb )
    return false;

  Qgis::WkbType type = wkb.readHeader();
  if ( QgsWkbTypes::flatType( type ) != Qgis::WkbType::NurbsCurve )
    return false;

  mWkbType = type;
  const bool is3D = QgsWkbTypes::hasZ( type );
  const bool isMeasure = QgsWkbTypes::hasM( type );

  // Read degree (4 bytes uint32)
  quint32 degree;
  wkb >> degree;
  mDegree = static_cast<int>( degree );

  // Read number of control points (4 bytes uint32)
  quint32 numControlPoints;
  wkb >> numControlPoints;

  mControlPoints.clear();
  mWeights.clear();
  mControlPoints.reserve( numControlPoints );
  mWeights.reserve( numControlPoints );

  // Read control points
  for ( quint32 i = 0; i < numControlPoints; ++i )
  {
    // Read byte order for this point (1 byte)
    char pointEndianness;
    wkb >> pointEndianness;

    // Read coordinates
    double x, y, z = 0.0, m = 0.0;
    wkb >> x >> y;

    if ( is3D )
      wkb >> z;
    if ( isMeasure )
      wkb >> m;

    // Read weight flag (1 byte)
    char weightFlag;
    wkb >> weightFlag;

    double weight = 1.0;
    if ( weightFlag == 1 )
    {
      // Read custom weight (8 bytes double)
      wkb >> weight;
    }

    // Create point with appropriate dimensionality
    QgsPoint point;
    if ( is3D && isMeasure )
      point = QgsPoint( x, y, z, m );
    else if ( is3D )
      point = QgsPoint( x, y, z );
    else if ( isMeasure )
      point = QgsPoint( x, y, std::numeric_limits<double>::quiet_NaN(), m );
    else
      point = QgsPoint( x, y );

    mControlPoints.append( point );
    mWeights.append( weight );
  }

  // Read number of knots (4 bytes uint32)
  quint32 numKnots;
  wkb >> numKnots;

  mKnots.clear();
  mKnots.reserve( numKnots );

  // Read knot values (8 bytes double each)
  for ( quint32 i = 0; i < numKnots; ++i )
  {
    double knot;
    wkb >> knot;
    mKnots.append( knot );
  }

  return true;
}

bool QgsNurbsCurve::fromWkt( const QString &wkt )
{
  clear();

  const QString geomTypeStr = wkt.split( '(' )[0].trimmed().toUpper();

  if ( !geomTypeStr.startsWith( QLatin1String( "NURBSCURVE" ) ) )
  {
    return false;
  }

  // Determine dimensionality from the geometry type string
  if ( geomTypeStr.contains( QLatin1String( "ZM" ) ) )
    mWkbType = Qgis::WkbType::NurbsCurveZM;
  else if ( geomTypeStr.endsWith( QLatin1String( " Z" ) ) || geomTypeStr == QLatin1String( "NURBSCURVE Z" ) )
    mWkbType = Qgis::WkbType::NurbsCurveZ;
  else if ( geomTypeStr.endsWith( QLatin1String( " M" ) ) || geomTypeStr == QLatin1String( "NURBSCURVE M" ) )
    mWkbType = Qgis::WkbType::NurbsCurveM;
  else
    mWkbType = Qgis::WkbType::NurbsCurve;

  QPair<Qgis::WkbType, QString> parts = QgsGeometryUtils::wktReadBlock( wkt );

  if ( parts.second.compare( QLatin1String( "EMPTY" ), Qt::CaseInsensitive ) == 0 || parts.second.isEmpty() )
    return true;

  // Split the content by commas at parentheses level 0
  QStringList blocks = QgsGeometryUtils::wktGetChildBlocks( parts.second, QString() );

  if ( blocks.isEmpty() )
    return false;

  // First block should be the degree
  bool ok = true;
  int degree = blocks[0].trimmed().toInt( &ok );
  if ( !ok || degree < 1 )
    return false;

  if ( blocks.size() < 2 )
    return false;

  // Second block should be the control points
  QString pointsStr = blocks[1];
  pointsStr = pointsStr.remove( '(' ).remove( ')' ).trimmed();

  // Parse control points
  QStringList pointsCoords = pointsStr.split( ',', Qt::SkipEmptyParts );
  QVector<QgsPoint> controlPoints;

  for ( const QString &pointStr : pointsCoords )
  {
    QStringList coords = pointStr.trimmed().split( QRegularExpression( QStringLiteral( "\\s+" ) ), Qt::SkipEmptyParts );

    if ( coords.size() < 2 )
      return false;

    QgsPoint point;
    bool ok = true;

    double x = coords[0].toDouble( &ok );
    if ( !ok )
      return false;

    double y = coords[1].toDouble( &ok );
    if ( !ok )
      return false;

    // Handle different coordinate patterns based on declared geometry type
    if ( coords.size() >= 3 )
    {
      if ( isMeasure() && !is3D() && coords.size() == 3 )
      {
        // NURBSCURVE M pattern: (x y m) - third coordinate is M, not Z
        double m = coords[2].toDouble( &ok );
        if ( !ok )
          return false;
        point = QgsPoint( x, y, std::numeric_limits<double>::quiet_NaN(), m );
      }
      else if ( is3D() && !isMeasure() && coords.size() >= 3 )
      {
        // NURBSCURVE Z pattern: (x y z)
        double z = coords[2].toDouble( &ok );
        if ( !ok )
          return false;
        point = QgsPoint( x, y, z );
      }
      else if ( is3D() && isMeasure() && coords.size() >= 4 )
      {
        // NURBSCURVE ZM pattern: (x y z m)
        double z = coords[2].toDouble( &ok );
        if ( !ok )
          return false;
        double m = coords[3].toDouble( &ok );
        if ( !ok )
          return false;
        point = QgsPoint( x, y, z, m );
      }
      else if ( isMeasure() && coords.size() >= 4 )
      {
        // NURBSCURVE M pattern with 4 coords: (x y z m) - upgrade to ZM
        double z = coords[2].toDouble( &ok );
        if ( !ok )
          return false;
        double m = coords[3].toDouble( &ok );
        if ( !ok )
          return false;
        point = QgsPoint( x, y, z, m );
        if ( !is3D() )
          mWkbType = QgsWkbTypes::addZ( mWkbType );
      }
      else if ( !is3D() && !isMeasure() && coords.size() == 3 )
      {
        // No explicit dimension - auto-upgrade to 3D: (x y z)
        double z = coords[2].toDouble( &ok );
        if ( !ok )
          return false;
        point = QgsPoint( x, y, z );
        mWkbType = QgsWkbTypes::addZ( mWkbType );
      }
      else if ( !is3D() && !isMeasure() && coords.size() >= 4 )
      {
        // No explicit dimension - auto-upgrade to ZM: (x y z m)
        double z = coords[2].toDouble( &ok );
        if ( !ok )
          return false;
        double m = coords[3].toDouble( &ok );
        if ( !ok )
          return false;
        point = QgsPoint( x, y, z, m );
        mWkbType = QgsWkbTypes::addZ( mWkbType );
        mWkbType = QgsWkbTypes::addM( mWkbType );
      }
      else
      {
        point = QgsPoint( x, y );
      }
    }
    else
    {
      point = QgsPoint( x, y );
    }

    controlPoints.append( point );
  }

  mControlPoints = controlPoints;

  // Initialize weights to 1.0 (non-rational by default)
  mWeights.clear();
  for ( int i = 0; i < controlPoints.size(); ++i )
  {
    mWeights.append( 1.0 );
  }

  // Parse additional parameters (degree already parsed at the beginning)
  bool hasWeights = false;
  bool hasKnots = false;

  // Process remaining blocks (starting from index 2 since 0=degree, 1=control points)
  for ( int i = 2; i < blocks.size(); ++i )
  {
    QString block = blocks[i].trimmed();

    if ( block.startsWith( '(' ) )
    {
      // This could be weights or knots vector
      block = block.remove( '(' ).remove( ')' ).trimmed();
      QStringList values = block.split( ',', Qt::SkipEmptyParts );

      QVector<double> parsedValues;
      for ( const QString &valueStr : values )
      {
        bool ok = true;
        double value = valueStr.trimmed().toDouble( &ok );
        if ( !ok )
          return false;
        parsedValues.append( value );
      }

      if ( !hasWeights && parsedValues.size() == controlPoints.size() )
      {
        // This is the weights vector
        mWeights = parsedValues;
        hasWeights = true;
      }
      else if ( !hasKnots )
      {
        // This is the knots vector
        mKnots = parsedValues;
        hasKnots = true;
      }
    }
  }

  mDegree = degree;

  // If no knots were provided, create default knots (open uniform)
  if ( !hasKnots )
  {
    const int n = controlPoints.size();
    const int knotsSize = n + degree + 1;
    mKnots.clear();
    mKnots.reserve( knotsSize );

    // Open uniform knot vector
    for ( int i = 0; i < knotsSize; ++i )
    {
      if ( i <= degree )
        mKnots.append( 0.0 );
      else if ( i >= n )
        mKnots.append( 1.0 );
      else
        mKnots.append( static_cast<double>( i - degree ) / ( n - degree ) );
    }
  }

  return true;
}

bool QgsNurbsCurve::fuzzyEqual( const QgsAbstractGeometry &other, double epsilon ) const
{
  const QgsNurbsCurve *o = qgsgeometry_cast<const QgsNurbsCurve *>( &other );
  if ( !o )
    return false;

  if ( mDegree != o->mDegree || mClosed != o->mClosed || mControlPoints.size() != o->mControlPoints.size() || mWeights.size() != o->mWeights.size() || mKnots.size() != o->mKnots.size() )
  {
    return false;
  }

  for ( int i = 0; i < mControlPoints.size(); ++i )
  {
    if ( mControlPoints[i].distance( o->mControlPoints[i] ) >= epsilon )
      return false;
  }

  for ( int i = 0; i < mWeights.size(); ++i )
  {
    if ( std::fabs( mWeights[i] - o->mWeights[i] ) > epsilon )
      return false;
  }

  for ( int i = 0; i < mKnots.size(); ++i )
  {
    if ( std::fabs( mKnots[i] - o->mKnots[i] ) > epsilon )
      return false;
  }

  return true;
}

bool QgsNurbsCurve::fuzzyDistanceEqual( const QgsAbstractGeometry &other, double epsilon ) const
{
  const QgsNurbsCurve *o = qgsgeometry_cast<const QgsNurbsCurve *>( &other );
  if ( !o )
    return false;

  if ( mDegree != o->mDegree || mClosed != o->mClosed || mControlPoints.size() != o->mControlPoints.size() || mWeights.size() != o->mWeights.size() || mKnots.size() != o->mKnots.size() )
  {
    return false;
  }

  for ( int i = 0; i < mControlPoints.size(); ++i )
  {
    if ( mControlPoints[i].distance( o->mControlPoints[i] ) > epsilon )
      return false;
  }

  for ( int i = 0; i < mWeights.size(); ++i )
  {
    if ( std::fabs( mWeights[i] - o->mWeights[i] ) > epsilon )
      return false;
  }

  for ( int i = 0; i < mKnots.size(); ++i )
  {
    if ( std::fabs( mKnots[i] - o->mKnots[i] ) > epsilon )
      return false;
  }

  return true;
}

QString QgsNurbsCurve::geometryType() const
{
  return QStringLiteral( "NurbsCurve" );
}

bool QgsNurbsCurve::hasCurvedSegments() const
{
  return true;
}

int QgsNurbsCurve::partCount() const
{
  return 1;
}

QgsCurve *QgsNurbsCurve::toCurveType() const
{
  return new QgsNurbsCurve( *this );
}

QgsPoint QgsNurbsCurve::vertexAt( QgsVertexId id ) const
{
  if ( id.part != 0 || id.ring != 0 )
  {
    return QgsPoint();
  }
  const int idx = id.vertex;
  if ( idx < 0 || idx >= mControlPoints.size() )
  {
    return QgsPoint();
  }
  return mControlPoints[idx];
}

int QgsNurbsCurve::vertexCount( int part, int ring ) const
{
  return ( part == 0 && ring == 0 ) ? mControlPoints.size() : 0;
}

int QgsNurbsCurve::vertexNumberFromVertexId( QgsVertexId id ) const
{
  if ( id.part == 0 && id.ring == 0 )
  {
    return id.vertex;
  }
  return -1;
}

bool QgsNurbsCurve::isValid( QString &error, Qgis::GeometryValidityFlags flags ) const
{
  Q_UNUSED( flags );

  if ( mDegree < 1 )
  {
    error = QStringLiteral( "Degree must be >= 1" );
    return false;
  }

  const int n = mControlPoints.size();
  if ( n < mDegree + 1 )
  {
    error = QStringLiteral( "Not enough control points for degree" );
    return false;
  }

  if ( mKnots.size() != n + mDegree + 1 )
  {
    error = QStringLiteral( "Knot vector size is incorrect" );
    return false;
  }

  if ( mWeights.size() != n )
  {
    error = QStringLiteral( "Weights vector size mismatch" );
    return false;
  }

  return true;
}

void QgsNurbsCurve::addToPainterPath( QPainterPath &path ) const
{
  std::unique_ptr<QgsLineString> line( curveToLine() );
  if ( line )
    line->addToPainterPath( path );
}

QgsCurve *QgsNurbsCurve::curveSubstring( double startDistance, double endDistance ) const
{
  std::unique_ptr<QgsLineString> line( curveToLine() );
  if ( !line )
    return nullptr;
  return line->curveSubstring( startDistance, endDistance );
}

double QgsNurbsCurve::length() const
{
  std::unique_ptr<QgsLineString> line( curveToLine() );
  return line ? line->length() : 0.0;
}

double QgsNurbsCurve::segmentLength( QgsVertexId startVertex ) const
{
  std::unique_ptr<QgsLineString> line( curveToLine() );
  if ( !line )
    return 0.0;
  return line->segmentLength( startVertex );
}

double QgsNurbsCurve::distanceBetweenVertices( QgsVertexId fromVertex, QgsVertexId toVertex ) const
{
  std::unique_ptr<QgsLineString> line( curveToLine() );
  if ( !line )
    return -1.0;
  return line->distanceBetweenVertices( fromVertex, toVertex );
}

QgsAbstractGeometry *QgsNurbsCurve::snappedToGrid( double hSpacing, double vSpacing, double dSpacing, double mSpacing, bool removeRedundantPoints ) const
{
  auto result = new QgsNurbsCurve( *this );
  for ( QgsPoint &pt : result->mControlPoints )
  {
    if ( hSpacing > 0 )
      pt.setX( std::round( pt.x() / hSpacing ) * hSpacing );
    if ( vSpacing > 0 )
      pt.setY( std::round( pt.y() / vSpacing ) * vSpacing );
    if ( pt.is3D() && dSpacing > 0 )
      pt.setZ( std::round( pt.z() / dSpacing ) * dSpacing );
    if ( pt.isMeasure() && mSpacing > 0 )
      pt.setM( std::round( pt.m() / mSpacing ) * mSpacing );
  }

  if ( removeRedundantPoints )
    result->removeDuplicateNodes();

  return result;
}

QgsAbstractGeometry *QgsNurbsCurve::simplifyByDistance( double tolerance ) const
{
  std::unique_ptr<QgsLineString> line( curveToLine() );
  if ( !line )
    return new QgsNurbsCurve( *this );
  return line->simplifyByDistance( tolerance );
}

bool QgsNurbsCurve::removeDuplicateNodes( double epsilon, bool useZValues )
{
  Q_UNUSED( useZValues );

  if ( mControlPoints.size() < 2 )
    return false;

  QVector<QgsPoint> newPoints;
  QVector<double> newWeights;

  newPoints.reserve( mControlPoints.size() );
  newWeights.reserve( mWeights.size() );

  newPoints.append( mControlPoints.first() );
  if ( !mWeights.isEmpty() )
    newWeights.append( mWeights.first() );

  for ( int i = 1; i < mControlPoints.size(); ++i )
  {
    if ( mControlPoints[i].distance( mControlPoints[i - 1] ) >= epsilon )
    {
      newPoints.append( mControlPoints[i] );
      if ( i < mWeights.size() )
        newWeights.append( mWeights[i] );
    }
  }

  const bool changed = ( newPoints.size() != mControlPoints.size() );
  mControlPoints = newPoints;
  mWeights = newWeights;

  return changed;
}

double QgsNurbsCurve::vertexAngle( QgsVertexId vertex ) const
{
  std::unique_ptr<QgsLineString> line( curveToLine() );
  if ( !line )
    return 0.0;
  return line->vertexAngle( vertex );
}

void QgsNurbsCurve::swapXy()
{
  for ( QgsPoint &pt : mControlPoints )
  {
    const double x = pt.x();
    pt.setX( pt.y() );
    pt.setY( x );
  }
  clearCache();
}

bool QgsNurbsCurve::transform( QgsAbstractGeometryTransformer *transformer, QgsFeedback *feedback )
{
  Q_UNUSED( feedback );
  if ( !transformer )
    return false;

  for ( QgsPoint &pt : mControlPoints )
  {
    double x = pt.x(), y = pt.y(), z = pt.z(), m = pt.m();
    if ( !transformer->transformPoint( x, y, z, m ) )
      return false;
    pt.setX( x );
    pt.setY( y );
    pt.setZ( z );
    pt.setM( m );
  }

  clearCache();
  return true;
}

QgsAbstractGeometry *QgsNurbsCurve::createEmptyWithSameType() const
{
  return new QgsNurbsCurve();
}

double QgsNurbsCurve::closestSegment( const QgsPoint &pt, QgsPoint &segmentPt, QgsVertexId &vertexAfter, int *leftOf, double epsilon ) const
{
  std::unique_ptr<QgsLineString> line( curveToLine() );
  if ( !line )
  {
    segmentPt = QgsPoint();
    vertexAfter = QgsVertexId();
    if ( leftOf )
      *leftOf = 0;
    return -1;
  }
  return line->closestSegment( pt, segmentPt, vertexAfter, leftOf, epsilon );
}

void QgsNurbsCurve::transform( const QgsCoordinateTransform &ct, Qgis::TransformDirection d, bool transformZ )
{
  for ( QgsPoint &pt : mControlPoints )
  {
    double x = pt.x();
    double y = pt.y();
    double z = transformZ && pt.is3D() ? pt.z() : std::numeric_limits<double>::quiet_NaN();
    ct.transformInPlace( x, y, z, d );
    pt.setX( x );
    pt.setY( y );
    if ( transformZ && pt.is3D() )
      pt.setZ( z );
  }
  clearCache();
}

void QgsNurbsCurve::transform( const QTransform &t, double zTranslate, double zScale, double mTranslate, double mScale )
{
  for ( QgsPoint &pt : mControlPoints )
  {
    const QPointF p = t.map( QPointF( pt.x(), pt.y() ) );
    pt.setX( p.x() );
    pt.setY( p.y() );

    if ( pt.is3D() )
      pt.setZ( pt.z() * zScale + zTranslate );
    if ( pt.isMeasure() )
      pt.setM( pt.m() * mScale + mTranslate );
  }
  clearCache();
}

QgsRectangle QgsNurbsCurve::boundingBox() const
{
  return boundingBox3D().toRectangle();
}

QgsBox3D QgsNurbsCurve::boundingBox3D() const
{
  if ( mBoundingBox.isNull() )
  {
    mBoundingBox = calculateBoundingBox3D();
  }
  return mBoundingBox;
}

QgsBox3D QgsNurbsCurve::calculateBoundingBox3D() const
{
  if ( mControlPoints.isEmpty() )
    return QgsBox3D();

  const QgsPoint &first = mControlPoints[0];
  double xmin = first.x();
  double ymin = first.y();
  double zmin = first.is3D() ? first.z() : 0.0;
  double xmax = xmin;
  double ymax = ymin;
  double zmax = zmin;

  for ( const QgsPoint &pt : mControlPoints )
  {
    xmin = std::min( xmin, pt.x() );
    ymin = std::min( ymin, pt.y() );
    xmax = std::max( xmax, pt.x() );
    ymax = std::max( ymax, pt.y() );

    if ( pt.is3D() )
    {
      zmin = std::min( zmin, pt.z() );
      zmax = std::max( zmax, pt.z() );
    }
  }

  return QgsBox3D( xmin, ymin, zmin, xmax, ymax, zmax );
}

void QgsNurbsCurve::clearCache() const
{
  QgsCurve::clearCache();
}

bool QgsNurbsCurve::moveVertex( QgsVertexId position, const QgsPoint &newPos )
{
  if ( position.part != 0 || position.ring != 0 )
    return false;

  const int idx = position.vertex;
  if ( idx < 0 || idx >= mControlPoints.size() )
    return false;

  mControlPoints[idx] = newPos;
  clearCache();
  return true;
}

bool QgsNurbsCurve::insertVertex( QgsVertexId position, const QgsPoint &vertex )
{
  if ( position.part != 0 || position.ring != 0 )
    return false;

  const int idx = position.vertex;
  if ( idx < 0 || idx > mControlPoints.size() )
    return false;

  mControlPoints.insert( idx, vertex );
  if ( idx <= mWeights.size() )
    mWeights.insert( idx, 1.0 );

  clearCache();
  return true;
}

int QgsNurbsCurve::wkbSize( QgsAbstractGeometry::WkbFlags flags ) const
{
  Q_UNUSED( flags );

  const bool is3D = QgsWkbTypes::hasZ( mWkbType );
  const bool isMeasure = QgsWkbTypes::hasM( mWkbType );
  const int coordinateDimension = 2 + ( is3D ? 1 : 0 ) + ( isMeasure ? 1 : 0 );

  int size = 0;

  // WKB header (endianness + type)
  size += 1 + 4;

  // Degree (4 bytes)
  size += 4;

  // Number of control points (4 bytes)
  size += 4;

  // Control points data
  for ( int i = 0; i < mControlPoints.size(); ++i )
  {
    // Point byte order (1 byte)
    size += 1;

    // Coordinates (8 bytes per coordinate)
    size += coordinateDimension * 8;

    // Weight flag (1 byte)
    size += 1;

    // Weight value if not default (8 bytes)
    if ( i < mWeights.size() && std::fabs( mWeights[i] - 1.0 ) > 1e-10 )
      size += 8;
  }

  // Number of knots (4 bytes)
  size += 4;

  // Knot values (8 bytes each)
  size += mKnots.size() * 8;

  return size;
}

QByteArray QgsNurbsCurve::asWkb( QgsAbstractGeometry::WkbFlags flags ) const
{
  QByteArray wkbArray;
  wkbArray.resize( QgsNurbsCurve::wkbSize( flags ) );
  QgsWkbPtr wkbPtr( wkbArray );

  // Write WKB header
  wkbPtr << static_cast<char>( QgsApplication::endian() );
  wkbPtr << static_cast<quint32>( mWkbType );

  // Write degree (4 bytes uint32)
  wkbPtr << static_cast<quint32>( mDegree );

  // Write number of control points (4 bytes uint32)
  wkbPtr << static_cast<quint32>( mControlPoints.size() );

  const bool is3D = QgsWkbTypes::hasZ( mWkbType );
  const bool isMeasure = QgsWkbTypes::hasM( mWkbType );

  // Write control points
  for ( int i = 0; i < mControlPoints.size(); ++i )
  {
    const QgsPoint &point = mControlPoints[i];

    // Write byte order for this point (1 byte) - use same as global
    wkbPtr << static_cast<char>( QgsApplication::endian() );

    // Write coordinates
    wkbPtr << point.x() << point.y();

    if ( is3D )
      wkbPtr << point.z();
    if ( isMeasure )
      wkbPtr << point.m();

    // Write weight flag and weight
    const double weight = ( i < mWeights.size() ) ? mWeights[i] : 1.0;
    const bool hasCustomWeight = std::fabs( weight - 1.0 ) > 1e-10;

    wkbPtr << static_cast<char>( hasCustomWeight ? 1 : 0 );

    if ( hasCustomWeight )
    {
      wkbPtr << weight;
    }
  }

  // Write number of knots (4 bytes uint32)
  wkbPtr << static_cast<quint32>( mKnots.size() );

  // Write knot values (8 bytes double each)
  for ( const double knot : mKnots )
  {
    wkbPtr << knot;
  }

  return wkbArray;
}

QString QgsNurbsCurve::asWkt( int precision ) const
{
  QString wkt = wktTypeStr();

  if ( isEmpty() )
  {
    wkt += QLatin1String( " EMPTY" );
  }
  else
  {
    wkt += QLatin1String( " (" );

    // Add degree first
    wkt += QString::number( mDegree );

    // Add control points
    wkt += QLatin1String( ", (" );
    for ( int i = 0; i < mControlPoints.size(); ++i )
    {
      if ( i > 0 )
        wkt += QLatin1String( ", " );

      const QgsPoint &pt = mControlPoints[i];
      wkt += qgsDoubleToString( pt.x(), precision ) + ' ' + qgsDoubleToString( pt.y(), precision );

      if ( pt.is3D() )
        wkt += ' ' + qgsDoubleToString( pt.z(), precision );

      if ( pt.isMeasure() )
        wkt += ' ' + qgsDoubleToString( pt.m(), precision );
    }
    wkt += ')';

    // Always add weights if they exist to ensure round-trip consistency
    if ( !mWeights.isEmpty() )
    {
      wkt += QLatin1String( ", (" );
      for ( int i = 0; i < mWeights.size(); ++i )
      {
        if ( i > 0 )
          wkt += QLatin1String( ", " );
        wkt += qgsDoubleToString( mWeights[i], precision );
      }
      wkt += ')';
    }

    // Always add knots if they exist to ensure round-trip consistency
    if ( !mKnots.isEmpty() )
    {
      wkt += QLatin1String( ", (" );
      for ( int i = 0; i < mKnots.size(); ++i )
      {
        if ( i > 0 )
          wkt += QLatin1String( ", " );
        wkt += qgsDoubleToString( mKnots[i], precision );
      }
      wkt += ')';
    }

    wkt += ')';
  }

  return wkt;
}

QDomElement QgsNurbsCurve::asGml2( QDomDocument &doc, int precision, const QString &ns, QgsAbstractGeometry::AxisOrder axisOrder ) const
{
  // GML2 does not support NURBS curves, convert to LineString
  // TODO: GML3 has BSpline support, but it's not clear how it's handled elsewhere in QGIS
  std::unique_ptr<QgsLineString> line( curveToLine() );
  if ( !line )
    return QDomElement();
  return line->asGml2( doc, precision, ns, axisOrder );
}

QDomElement QgsNurbsCurve::asGml3( QDomDocument &doc, int precision, const QString &ns, QgsAbstractGeometry::AxisOrder axisOrder ) const
{
  // TODO: GML3 has native BSpline support (gml:BSpline), but it's not clear how it's handled elsewhere in QGIS
  // For now, convert to LineString for compatibility
  std::unique_ptr<QgsLineString> line( curveToLine() );
  if ( !line )
    return QDomElement();
  return line->asGml3( doc, precision, ns, axisOrder );
}

json QgsNurbsCurve::asJsonObject( int precision ) const
{
  Q_UNUSED( precision );
  return json::object(); // Not implemented
}

QString QgsNurbsCurve::asKml( int precision ) const
{
  // KML does not support NURBS curves, convert to LineString
  std::unique_ptr<QgsLineString> line( curveToLine() );
  if ( !line )
    return QString();
  return line->asKml( precision );
}

int QgsNurbsCurve::dimension() const
{
  return 1;
}

bool QgsNurbsCurve::isEmpty() const
{
  return mControlPoints.isEmpty();
}

void QgsNurbsCurve::clear()
{
  mControlPoints.clear();
  mKnots.clear();
  mWeights.clear();
  mDegree = 0;
  mClosed = false;
  clearCache();
}

bool QgsNurbsCurve::boundingBoxIntersects( const QgsRectangle &rectangle ) const
{
  return boundingBox().intersects( rectangle );
}

bool QgsNurbsCurve::boundingBoxIntersects( const QgsBox3D &box3d ) const
{
  return boundingBox3D().intersects( box3d );
}

QgsPoint QgsNurbsCurve::centroid() const
{
  std::unique_ptr<QgsLineString> line( curveToLine() );
  return line ? line->centroid() : QgsPoint();
}

int QgsNurbsCurve::compareToSameClass( const QgsAbstractGeometry *other ) const
{
  const QgsNurbsCurve *otherCurve = qgsgeometry_cast<const QgsNurbsCurve *>( other );
  if ( !otherCurve )
    return -1;

  if ( mDegree < otherCurve->mDegree )
    return -1;
  else if ( mDegree > otherCurve->mDegree )
    return 1;

  const int nThis = mControlPoints.size();
  const int nOther = otherCurve->mControlPoints.size();

  if ( nThis < nOther )
    return -1;
  else if ( nThis > nOther )
    return 1;

  for ( int i = 0; i < nThis; ++i )
  {
    if ( mControlPoints[i].x() < otherCurve->mControlPoints[i].x() )
      return -1;
    else if ( mControlPoints[i].x() > otherCurve->mControlPoints[i].x() )
      return 1;
    else if ( mControlPoints[i].y() < otherCurve->mControlPoints[i].y() )
      return -1;
    else if ( mControlPoints[i].y() > otherCurve->mControlPoints[i].y() )
      return 1;
  }

  return 0;
}

double QgsNurbsCurve::weight( int index ) const
{
  if ( index < 0 || index >= mWeights.size() )
    return 1.0;
  return mWeights[index];
}

bool QgsNurbsCurve::setWeight( int index, double weight )
{
  if ( index < 0 || index >= mWeights.size() )
    return false;
  if ( weight <= 0.0 )
    return false;
  mWeights[index] = weight;
  clearCache();
  return true;
}
