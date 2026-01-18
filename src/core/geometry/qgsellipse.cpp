/***************************************************************************
                         qgsellipse.cpp
                         --------------
    begin                : March 2017
    copyright            : (C) 2017 by Loîc Bartoletti
    email                : lbartoletti at tuxfamily dot org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsellipse.h"

#include <limits>
#include <memory>

#include "qgsgeometryutils.h"
#include "qgslinestring.h"
#include "qgsunittypes.h"

void QgsEllipse::normalizeAxis()
{
  mSemiMajorAxis = std::fabs( mSemiMajorAxis );
  mSemiMinorAxis = std::fabs( mSemiMinorAxis );
  if ( mSemiMajorAxis < mSemiMinorAxis )
  {
    std::swap( mSemiMajorAxis, mSemiMinorAxis );
    mAzimuth = 180.0 / M_PI * QgsGeometryUtilsBase::normalizedAngle( M_PI / 180.0 * ( mAzimuth + 90 ) );
  }
}

QgsEllipse::QgsEllipse( const QgsPoint &center, const double axis_a, const double axis_b, const double azimuth )
  : mCenter( center )
  , mSemiMajorAxis( axis_a )
  , mSemiMinorAxis( axis_b )
  , mAzimuth( azimuth )
{
  normalizeAxis();
}

QgsEllipse QgsEllipse::fromFoci( const QgsPoint &pt1, const QgsPoint &pt2, const QgsPoint &pt3 )
{
  const double dist_p1p2 = pt1.distance( pt2 );
  const double dist_p1p3 = pt1.distance( pt3 );
  const double dist_p2p3 = pt2.distance( pt3 );

  const double dist = dist_p1p3 + dist_p2p3;
  const double azimuth = 180.0 / M_PI * QgsGeometryUtilsBase::lineAngle( pt1.x(), pt1.y(), pt2.x(), pt2.y() );
  QgsPoint center = QgsGeometryUtils::midpoint( pt1, pt2 );

  const double axis_a = dist / 2.0;
  const double axis_b = std::sqrt( std::pow( axis_a, 2.0 ) - std::pow( dist_p1p2 / 2.0, 2.0 ) );

  QgsGeometryUtils::transferFirstZOrMValueToPoint( QgsPointSequence() << pt1 << pt2 << pt3, center );

  return QgsEllipse( center, axis_a, axis_b, azimuth );
}

QgsEllipse QgsEllipse::fromExtent( const QgsPoint &pt1, const QgsPoint &pt2 )
{
  QgsPoint center = QgsGeometryUtils::midpoint( pt1, pt2 );
  const double axis_a = std::fabs( pt2.x() - pt1.x() ) / 2.0;
  const double axis_b = std::fabs( pt2.y() - pt1.y() ) / 2.0;
  const double azimuth = 90.0;

  QgsGeometryUtils::transferFirstZOrMValueToPoint( QgsPointSequence() << pt1 << pt2, center );

  return QgsEllipse( center, axis_a, axis_b, azimuth );
}

QgsEllipse QgsEllipse::fromCenterPoint( const QgsPoint &center, const QgsPoint &pt1 )
{
  const double axis_a = std::fabs( pt1.x() - center.x() );
  const double axis_b = std::fabs( pt1.y() - center.y() );
  const double azimuth = 90.0;

  QgsPoint centerPt( center );
  QgsGeometryUtils::transferFirstZOrMValueToPoint( QgsPointSequence() << center << pt1, centerPt );

  return QgsEllipse( centerPt, axis_a, axis_b, azimuth );
}

QgsEllipse QgsEllipse::fromCenter2Points( const QgsPoint &center, const QgsPoint &pt1, const QgsPoint &pt2 )
{
  const double azimuth = 180.0 / M_PI * QgsGeometryUtilsBase::lineAngle( center.x(), center.y(), pt1.x(), pt1.y() );
  const double axis_a = center.distance( pt1 );

  const double length = pt2.distance( QgsGeometryUtils::projectPointOnSegment( pt2, center, pt1 ) );
  const QgsPoint pp = center.project( length, 90 + azimuth );
  const double axis_b = center.distance( pp );

  QgsPoint centerPt( center );
  QgsGeometryUtils::transferFirstZOrMValueToPoint( QgsPointSequence() << center << pt1 << pt2, centerPt );

  return QgsEllipse( centerPt, axis_a, axis_b, azimuth );
}

QgsEllipse QgsEllipse::from4Points( const QgsPoint &pt1, const QgsPoint &pt2, const QgsPoint &pt3, const QgsPoint &pt4 )
{
  // Algorithm based on LibreCAD's RS_Ellipse::createFrom4P()
  // Solves the linear system for axis-aligned ellipse: A*x² + B*x + C*y² + D*y = 1

  constexpr double tolerance = 1e-10;

  // Normalize coordinates to improve numerical stability
  // Translate to centroid and scale by characteristic length
  const double cx = ( pt1.x() + pt2.x() + pt3.x() + pt4.x() ) / 4.0;
  const double cy = ( pt1.y() + pt2.y() + pt3.y() + pt4.y() ) / 4.0;

  // Compute relative vectors from centroid
  const std::array<QgsVector, 4> relativeVectors =
  {
    QgsVector( pt1.x() - cx, pt1.y() - cy ),
    QgsVector( pt2.x() - cx, pt2.y() - cy ),
    QgsVector( pt3.x() - cx, pt3.y() - cy ),
    QgsVector( pt4.x() - cx, pt4.y() - cy )
  };

  // Compute scale as max distance from centroid
  double scale = 0.0;
  for ( const QgsVector &v : relativeVectors )
  {
    scale = std::max( scale, v.length() );
  }

  if ( scale < tolerance )
    return QgsEllipse(); // All points coincident

  const double invScale = 1.0 / scale;

  // Build the 4x4 augmented matrix [M|b] where M*coeffs = b
  // Each row: [x², x, y², y | 1] with normalized coordinates
  double matrix[4][5];

  for ( int i = 0; i < 4; ++i )
  {
    const double x = relativeVectors[i].x() * invScale;
    const double y = relativeVectors[i].y() * invScale;
    matrix[i][0] = x * x;
    matrix[i][1] = x;
    matrix[i][2] = y * y;
    matrix[i][3] = y;
    matrix[i][4] = 1.0;
  }

  // Gaussian elimination with partial pivoting
  for ( int col = 0; col < 4; ++col )
  {
    // Find pivot
    int maxRow = col;
    double maxVal = std::fabs( matrix[col][col] );
    for ( int row = col + 1; row < 4; ++row )
    {
      if ( std::fabs( matrix[row][col] ) > maxVal )
      {
        maxVal = std::fabs( matrix[row][col] );
        maxRow = row;
      }
    }

    // Swap rows
    if ( maxRow != col )
    {
      for ( int k = 0; k < 5; ++k )
        std::swap( matrix[col][k], matrix[maxRow][k] );
    }

    // Check for singular matrix
    if ( std::fabs( matrix[col][col] ) < tolerance )
      return QgsEllipse();

    // Eliminate column
    for ( int row = col + 1; row < 4; ++row )
    {
      const double factor = matrix[row][col] / matrix[col][col];
      for ( int k = col; k < 5; ++k )
        matrix[row][k] -= factor * matrix[col][k];
    }
  }

  // Back substitution
  double coeffs[4];
  for ( int i = 3; i >= 0; --i )
  {
    coeffs[i] = matrix[i][4];
    for ( int j = i + 1; j < 4; ++j )
      coeffs[i] -= matrix[i][j] * coeffs[j];
    if ( std::fabs( matrix[i][i] ) < tolerance )
      return QgsEllipse();
    coeffs[i] /= matrix[i][i];
  }

  const double A = coeffs[0]; // coefficient of x²
  const double B = coeffs[1]; // coefficient of x
  const double C = coeffs[2]; // coefficient of y²
  const double D = coeffs[3]; // coefficient of y

  // Validate ellipse conditions
  // For a valid ellipse: A > 0, C > 0, and discriminant d > 0
  if ( std::fabs( A ) < tolerance || std::fabs( C ) < tolerance )
    return QgsEllipse();

  const double d = 1.0 + 0.25 * ( B * B / A + D * D / C );

  if ( d / A < tolerance || d / C < tolerance )
    return QgsEllipse();

  // Extract ellipse parameters in normalized coordinates
  const double centerXNorm = -0.5 * B / A;
  const double centerYNorm = -0.5 * D / C;
  const double semiAxisXNorm = std::sqrt( d / A );
  const double semiAxisYNorm = std::sqrt( d / C );

  // Transform back to original coordinates
  const double centerX = centerXNorm * scale + cx;
  const double centerY = centerYNorm * scale + cy;
  const double semiAxisX = semiAxisXNorm * scale;
  const double semiAxisY = semiAxisYNorm * scale;

  QgsPoint center( centerX, centerY );
  QgsGeometryUtils::transferFirstZOrMValueToPoint( QgsPointSequence() << pt1 << pt2 << pt3 << pt4, center );

  // Azimuth is 90° for axis-aligned ellipse (major axis along X or Y)
  return QgsEllipse( center, semiAxisX, semiAxisY, 90.0 );
}

QgsEllipse QgsEllipse::fromCenter3Points( const QgsPoint &center, const QgsPoint &pt1, const QgsPoint &pt2, const QgsPoint &pt3 )
{
  // Algorithm based on LibreCAD's RS_Ellipse::createFromCenter3Points() and createFromQuadratic()
  // Solves the quadratic form: a*x² + b*x*y + c*y² = 1
  // where (x,y) are coordinates relative to the center

  constexpr double tolerance = 1e-10;

  // Compute relative vectors from center
  const QgsVector vp1( pt1.x() - center.x(), pt1.y() - center.y() );
  const QgsVector vp2( pt2.x() - center.x(), pt2.y() - center.y() );
  const QgsVector vp3( pt3.x() - center.x(), pt3.y() - center.y() );

  // Distances from center (before normalization)
  const double r1 = vp1.length();
  const double r2 = vp2.length();
  const double r3 = vp3.length();

  // Check if any point is at the center
  if ( r1 < tolerance || r2 < tolerance || r3 < tolerance )
    return QgsEllipse();

  // Compute scale for normalization (max distance from center)
  const double scale = std::max( { r1, r2, r3 } );
  const double invScale = 1.0 / scale;

  // Normalize vectors for numerical stability
  const QgsVector vp1Norm = vp1 * invScale;
  const QgsVector vp2Norm = vp2 * invScale;
  const QgsVector vp3Norm = vp3 * invScale;

  // Normalized distances squared
  const double r1sq = vp1Norm.lengthSquared();
  const double r2sq = vp2Norm.lengthSquared();
  const double r3sq = vp3Norm.lengthSquared();

  // Check if pt2 and pt3 are identical (2-point case)
  const double dist23sq = ( vp3Norm - vp2Norm ).lengthSquared();
  const bool twoPointCase = dist23sq < tolerance;

  double a, b, c; // Quadratic form coefficients

  if ( twoPointCase )
  {
    // 2-point case: axis-aligned ellipse
    // Solve: a*x² + c*y² = 1 (b = 0)
    // System: vp1x²*a + vp1y²*c = 1
    //         vp2x²*a + vp2y²*c = 1

    const double vp1x2 = vp1Norm.x() * vp1Norm.x();
    const double vp1y2 = vp1Norm.y() * vp1Norm.y();
    const double vp2x2 = vp2Norm.x() * vp2Norm.x();
    const double vp2y2 = vp2Norm.y() * vp2Norm.y();

    const double det = vp1x2 * vp2y2 - vp2x2 * vp1y2;
    if ( std::fabs( det ) < tolerance )
      return QgsEllipse();

    a = ( vp2y2 - vp1y2 ) / det;
    c = ( vp1x2 - vp2x2 ) / det;

    if ( a < tolerance || c < tolerance )
      return QgsEllipse();

    // Semi-axes in normalized coordinates, then scale back
    const double semiAxisX = scale / std::sqrt( a );
    const double semiAxisY = scale / std::sqrt( c );

    QgsPoint centerPt( center );
    QgsGeometryUtils::transferFirstZOrMValueToPoint( QgsPointSequence() << center << pt1 << pt2 << pt3, centerPt );

    return QgsEllipse( centerPt, semiAxisX, semiAxisY, 90.0 );
  }

  // Check if all 3 points are equidistant from center (circle case)
  // Use relative tolerance for robustness
  const double maxRsq = std::max( { r1sq, r2sq, r3sq } );
  if ( std::fabs( r1sq - r2sq ) < tolerance * maxRsq && std::fabs( r1sq - r3sq ) < tolerance * maxRsq )
  {
    // Points are on a circle - use original (non-normalized) radius
    const double radius = r1 * scale; // r1 is normalized, scale back

    QgsPoint centerPt( center );
    QgsGeometryUtils::transferFirstZOrMValueToPoint( QgsPointSequence() << center << pt1 << pt2 << pt3, centerPt );

    return QgsEllipse( centerPt, radius, radius, 0.0 );
  }

  // Check if pt1 and pt3 are antipodal (opposite through center)
  // This happens when vp1 ≈ -vp3, making rows 1 and 3 of the matrix identical
  const QgsVector sumVp13 = vp1Norm + vp3Norm;
  const bool antipodal13 = std::fabs( sumVp13.x() ) < tolerance && std::fabs( sumVp13.y() ) < tolerance;

  if ( antipodal13 && std::fabs( r1sq - r3sq ) < tolerance * maxRsq )
  {
    // pt1 and pt3 define a diameter, pt2 is a third point
    // The semi-axis along pt1 direction has length r1 (original distance)
    // We need to find the semi-axis perpendicular to it

    // Direction of pt1 from center (this is one axis of the ellipse)
    const double angle1 = vp1Norm.angle();

    // Project pt2 onto the perpendicular axis
    const double cosAngle1 = std::cos( angle1 );
    const double sinAngle1 = std::sin( angle1 );

    // pt2 in rotated coordinates (aligned with pt1 axis)
    const double pt2AlongAxis = vp2Norm.x() * cosAngle1 + vp2Norm.y() * sinAngle1;
    const double pt2PerpAxis = -vp2Norm.x() * sinAngle1 + vp2Norm.y() * cosAngle1;

    // For ellipse: (pt2AlongAxis / a)² + (pt2PerpAxis / b)² = 1
    // where a = normalized r1 = sqrt(r1sq), solve for b (normalized)
    const double aNorm = std::sqrt( r1sq ); // normalized semi-axis along pt1
    const double alongRatio = pt2AlongAxis / aNorm;
    const double perpSq = pt2PerpAxis * pt2PerpAxis;

    if ( alongRatio * alongRatio >= 1.0 - tolerance || perpSq < tolerance )
      return QgsEllipse(); // pt2 not strictly inside or on wrong position

    const double bSqNorm = perpSq / ( 1.0 - alongRatio * alongRatio );
    if ( bSqNorm < tolerance )
      return QgsEllipse();

    // Scale back to original coordinates
    // r1 is the original (non-normalized) distance to pt1
    const double semiAxis1 = r1; // original distance, not r1*scale!
    const double semiAxis2 = std::sqrt( bSqNorm ) * scale;

    // Convert angle to QGIS azimuth (from north, clockwise)
    double azimuth = 90.0 - angle1 * 180.0 / M_PI;
    azimuth = 180.0 / M_PI * QgsGeometryUtilsBase::normalizedAngle( M_PI / 180.0 * azimuth );

    QgsPoint centerPt( center );
    QgsGeometryUtils::transferFirstZOrMValueToPoint( QgsPointSequence() << center << pt1 << pt2 << pt3, centerPt );

    return QgsEllipse( centerPt, semiAxis1, semiAxis2, azimuth );
  }

  // 3-point case: general ellipse with possible rotation
  // Solve: a*x² + b*x*y + c*y² = 1
  // Direct 3x3 system (exact solution when 3 points define unique ellipse)

  const double m00 = vp1Norm.x() * vp1Norm.x(), m01 = vp1Norm.x() * vp1Norm.y(), m02 = vp1Norm.y() * vp1Norm.y();
  const double m10 = vp2Norm.x() * vp2Norm.x(), m11 = vp2Norm.x() * vp2Norm.y(), m12 = vp2Norm.y() * vp2Norm.y();
  const double m20 = vp3Norm.x() * vp3Norm.x(), m21 = vp3Norm.x() * vp3Norm.y(), m22 = vp3Norm.y() * vp3Norm.y();

  // Build augmented matrix for Gaussian elimination
  double matrix[3][4] =
  {
    { m00, m01, m02, 1.0 },
    { m10, m11, m12, 1.0 },
    { m20, m21, m22, 1.0 }
  };

  // Gaussian elimination with partial pivoting
  for ( int col = 0; col < 3; ++col )
  {
    int maxRow = col;
    double maxVal = std::fabs( matrix[col][col] );
    for ( int row = col + 1; row < 3; ++row )
    {
      if ( std::fabs( matrix[row][col] ) > maxVal )
      {
        maxVal = std::fabs( matrix[row][col] );
        maxRow = row;
      }
    }

    if ( maxRow != col )
    {
      for ( int k = 0; k < 4; ++k )
        std::swap( matrix[col][k], matrix[maxRow][k] );
    }

    if ( std::fabs( matrix[col][col] ) < tolerance )
      return QgsEllipse(); // Singular matrix - points don't define valid ellipse

    for ( int row = col + 1; row < 3; ++row )
    {
      const double factor = matrix[row][col] / matrix[col][col];
      for ( int k = col; k < 4; ++k )
        matrix[row][k] -= factor * matrix[col][k];
    }
  }

  // Back substitution
  if ( std::fabs( matrix[2][2] ) < tolerance || std::fabs( matrix[1][1] ) < tolerance )
    return QgsEllipse();

  c = matrix[2][3] / matrix[2][2];
  b = ( matrix[1][3] - matrix[1][2] * c ) / matrix[1][1];
  a = ( matrix[0][3] - matrix[0][1] * b - matrix[0][2] * c ) / matrix[0][0];

  // Convert quadratic form to ellipse parameters using eigenvalue decomposition #spellOk
  // Matrix: | a    b/2 |
  //         | b/2  c   |
  // Eigenvalues: λ = (a+c ± sqrt((a-c)² + b²)) / 2

  const double d = a - c;
  const double s = std::hypot( d, b );

  // For a valid ellipse, both eigenvalues must be positive
  // This requires: s < a + c
  if ( s >= a + c - tolerance )
    return QgsEllipse();

  const double lambda1 = 0.5 * ( a + c - s ); // smaller eigenvalue
  const double lambda2 = 0.5 * ( a + c + s ); // larger eigenvalue

  if ( lambda1 < tolerance || lambda2 < tolerance )
    return QgsEllipse();

  // Semi-axes from eigenvalues (in normalized coordinates), then scale back
  const double semiMajor = scale / std::sqrt( lambda1 );
  const double semiMinor = scale / std::sqrt( lambda2 );

  // Compute azimuth from eigenvector
  // For eigenvalue (a+c-s)/2, eigenvector is:
  // if a >= c: (-b, d+s)
  // if a < c:  (s-d, -b)
  // Angle of vector (x, y) is atan2(y, x)
  double azimuth;
  if ( std::fabs( b ) < tolerance )
  {
    // Axis-aligned case
    azimuth = ( a <= c ) ? 90.0 : 0.0;
  }
  else if ( a >= c )
  {
    // Eigenvector (-b, d+s), angle = atan2(d+s, -b)
    azimuth = std::atan2( d + s, -b );
    azimuth = 90.0 - azimuth * 180.0 / M_PI;
  }
  else
  {
    // Eigenvector (s-d, -b), angle = atan2(-b, s-d)
    azimuth = std::atan2( -b, s - d );
    azimuth = 90.0 - azimuth * 180.0 / M_PI;
  }

  azimuth = 180.0 / M_PI * QgsGeometryUtilsBase::normalizedAngle( M_PI / 180.0 * azimuth );

  QgsPoint centerPt( center );
  QgsGeometryUtils::transferFirstZOrMValueToPoint( QgsPointSequence() << center << pt1 << pt2 << pt3, centerPt );

  return QgsEllipse( centerPt, semiMajor, semiMinor, azimuth );
}

bool QgsEllipse::operator==( const QgsEllipse &elp ) const
{
  return ( ( mCenter == elp.mCenter ) && qgsDoubleNear( mSemiMajorAxis, elp.mSemiMajorAxis, 1E-8 ) && qgsDoubleNear( mSemiMinorAxis, elp.mSemiMinorAxis, 1E-8 ) && qgsDoubleNear( mAzimuth, elp.mAzimuth, 1E-8 ) );
}

bool QgsEllipse::operator!=( const QgsEllipse &elp ) const
{
  return !operator==( elp );
}

bool QgsEllipse::isEmpty() const
{
  return ( qgsDoubleNear( mSemiMajorAxis, 0.0, 1E-8 ) || qgsDoubleNear( mSemiMinorAxis, 0.0, 1E-8 ) );
}

void QgsEllipse::setSemiMajorAxis( const double axis_a )
{
  mSemiMajorAxis = axis_a;
  normalizeAxis();
}
void QgsEllipse::setSemiMinorAxis( const double axis_b )
{
  mSemiMinorAxis = axis_b;
  normalizeAxis();
}

void QgsEllipse::setAzimuth( const double azimuth )
{
  mAzimuth = 180.0 / M_PI * QgsGeometryUtilsBase::normalizedAngle( M_PI / 180.0 * azimuth );
}

double QgsEllipse::focusDistance() const
{
  return std::sqrt( mSemiMajorAxis * mSemiMajorAxis - mSemiMinorAxis * mSemiMinorAxis );
}

QVector<QgsPoint> QgsEllipse::foci() const
{
  const double dist_focus = focusDistance();
  return
  {
    mCenter.project( dist_focus, mAzimuth ),
    mCenter.project( -dist_focus, mAzimuth ),
  };
}

double QgsEllipse::eccentricity() const
{
  if ( isEmpty() )
  {
    return std::numeric_limits<double>::quiet_NaN();
  }
  return focusDistance() / mSemiMajorAxis;
}

double QgsEllipse::area() const
{
  return M_PI * mSemiMajorAxis * mSemiMinorAxis;
}

double QgsEllipse::perimeter() const
{
  const double a = mSemiMajorAxis;
  const double b = mSemiMinorAxis;
  return M_PI * ( 3 * ( a + b ) - std::sqrt( 10 * a * b + 3 * ( a * a + b * b ) ) );
}

QVector<QgsPoint> QgsEllipse::quadrant() const
{
  return
  {
    mCenter.project( mSemiMajorAxis, mAzimuth ),
    mCenter.project( mSemiMinorAxis, mAzimuth + 90 ),
    mCenter.project( -mSemiMajorAxis, mAzimuth ),
    mCenter.project( -mSemiMinorAxis, mAzimuth + 90 )
  };
}

QgsPointSequence QgsEllipse::points( unsigned int segments ) const
{
  QgsPointSequence pts;

  QVector<double> x;
  QVector<double> y;
  QVector<double> z;
  QVector<double> m;

  pointsInternal( segments, x, y, z, m );
  const bool hasZ = !z.empty();
  const bool hasM = !m.empty();
  pts.reserve( x.size() );
  for ( int i = 0; i < x.size(); ++i )
  {
    pts.append( QgsPoint( x[i], y[i], hasZ ? z[i] : std::numeric_limits< double >::quiet_NaN(), hasM ? m[i] : std::numeric_limits< double >::quiet_NaN() ) );
  }
  return pts;
}

void QgsEllipse::pointsInternal( unsigned int segments, QVector<double> &x, QVector<double> &y, QVector<double> &z, QVector<double> &m ) const
{
  if ( segments < 3 )
  {
    return;
  }

  const double centerX = mCenter.x();
  const double centerY = mCenter.y();
  const double centerZ = mCenter.z();
  const double centerM = mCenter.m();
  const bool hasZ = mCenter.is3D();
  const bool hasM = mCenter.isMeasure();

  std::vector<double> t( segments );
  const QgsPoint p1 = mCenter.project( mSemiMajorAxis, mAzimuth );
  const double azimuth = std::atan2( p1.y() - mCenter.y(), p1.x() - mCenter.x() );
  for ( unsigned int i = 0; i < segments; ++i )
  {
    t[i] = 2 * M_PI - ( ( 2 * M_PI ) / segments * i ); // Since the algorithm used rotates in the trigonometric direction (counterclockwise)
  }

  x.resize( segments );
  y.resize( segments );
  if ( hasZ )
    z.resize( segments );
  if ( hasM )
    m.resize( segments );
  double *xOut = x.data();
  double *yOut = y.data();
  double *zOut = hasZ ? z.data() : nullptr;
  double *mOut = hasM ? m.data() : nullptr;

  const double cosAzimuth = std::cos( azimuth );
  const double sinAzimuth = std::sin( azimuth );
  for ( double it : t )
  {
    const double cosT { std::cos( it ) };
    const double sinT { std::sin( it ) };
    *xOut++ = centerX + mSemiMajorAxis * cosT * cosAzimuth - mSemiMinorAxis * sinT * sinAzimuth;
    *yOut++ = centerY + mSemiMajorAxis * cosT * sinAzimuth + mSemiMinorAxis * sinT * cosAzimuth;
    if ( zOut )
      *zOut++ = centerZ;
    if ( mOut )
      *mOut++ = centerM;
  }
}

QgsPolygon *QgsEllipse::toPolygon( unsigned int segments ) const
{
  auto polygon = std::make_unique<QgsPolygon>();
  if ( segments < 3 )
  {
    return polygon.release();
  }

  polygon->setExteriorRing( toLineString( segments ) );

  return polygon.release();
}

QgsLineString *QgsEllipse::toLineString( unsigned int segments ) const
{
  if ( segments < 3 )
  {
    return new QgsLineString();
  }

  QVector<double> x;
  QVector<double> y;
  QVector<double> z;
  QVector<double> m;

  pointsInternal( segments, x, y, z, m );
  if ( x.empty() )
    return new QgsLineString();

  // close linestring
  x.append( x.at( 0 ) );
  y.append( y.at( 0 ) );
  if ( !z.empty() )
    z.append( z.at( 0 ) );
  if ( !m.empty() )
    m.append( m.at( 0 ) );

  return new QgsLineString( x, y, z, m );
}

QgsRectangle QgsEllipse::boundingBox() const
{
  if ( isEmpty() )
  {
    return QgsRectangle();
  }

  const double angle = mAzimuth * M_PI / 180.0;

  const double ux = mSemiMajorAxis * std::cos( angle );
  const double uy = mSemiMinorAxis * std::sin( angle );
  const double vx = mSemiMajorAxis * std::sin( angle );
  const double vy = mSemiMinorAxis * std::cos( angle );

  const double halfHeight = std::sqrt( ux * ux + uy * uy );
  const double halfWidth = std::sqrt( vx * vx + vy * vy );

  const QgsPointXY p1( mCenter.x() - halfWidth, mCenter.y() - halfHeight );
  const QgsPointXY p2( mCenter.x() + halfWidth, mCenter.y() + halfHeight );

  return QgsRectangle( p1, p2 );
}

QString QgsEllipse::toString( int pointPrecision, int axisPrecision, int azimuthPrecision ) const
{
  QString rep;
  if ( isEmpty() )
    rep = u"Empty"_s;
  else
    rep = u"Ellipse (Center: %1, Semi-Major Axis: %2, Semi-Minor Axis: %3, Azimuth: %4)"_s
          .arg( mCenter.asWkt( pointPrecision ), 0, 's' )
          .arg( qgsDoubleToString( mSemiMajorAxis, axisPrecision ), 0, 'f' )
          .arg( qgsDoubleToString( mSemiMinorAxis, axisPrecision ), 0, 'f' )
          .arg( qgsDoubleToString( mAzimuth, azimuthPrecision ), 0, 'f' );

  return rep;
}

QgsPolygon *QgsEllipse::orientedBoundingBox() const
{
  auto ombb = std::make_unique<QgsPolygon>();
  if ( isEmpty() )
  {
    return ombb.release();
  }

  const QVector<QgsPoint> q = quadrant();

  const QgsPoint p1 = q.at( 0 ).project( mSemiMinorAxis, mAzimuth - 90 );
  const QgsPoint p2 = q.at( 0 ).project( mSemiMinorAxis, mAzimuth + 90 );
  const QgsPoint p3 = q.at( 2 ).project( mSemiMinorAxis, mAzimuth + 90 );
  const QgsPoint p4 = q.at( 2 ).project( mSemiMinorAxis, mAzimuth - 90 );

  QgsLineString *ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << p1 << p2 << p3 << p4 );

  ombb->setExteriorRing( ext );

  return ombb.release();
}
