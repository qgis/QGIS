/***************************************************************************
  qgsmatrix4x4.cpp
  --------------------------------------
  Date                 : July 2023
  Copyright            : (C) 2023 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmatrix4x4.h"

// the implementation is partially based on Qt's QMatrix4x4 (simplified)


// clang-format off
QgsMatrix4x4::QgsMatrix4x4(
  double m11, double m12, double m13, double m14,
  double m21, double m22, double m23, double m24,
  double m31, double m32, double m33, double m34,
  double m41, double m42, double m43, double m44
)
// clang-format on
{
  m[0][0] = m11;
  m[0][1] = m21;
  m[0][2] = m31;
  m[0][3] = m41;
  m[1][0] = m12;
  m[1][1] = m22;
  m[1][2] = m32;
  m[1][3] = m42;
  m[2][0] = m13;
  m[2][1] = m23;
  m[2][2] = m33;
  m[2][3] = m43;
  m[3][0] = m14;
  m[3][1] = m24;
  m[3][2] = m34;
  m[3][3] = m44;
}

bool QgsMatrix4x4::fuzzyEqual( const QgsMatrix4x4 &other, double epsilon ) const
{
  const double *data = *m;
  const double *otherData = *( other.m );
  for ( int i = 0; i < 16; ++i, data++, otherData++ )
  {
    if ( !qgsDoubleNear( *data, *otherData, epsilon ) )
    {
      return false;
    }
  }

  return true;
}

void QgsMatrix4x4::translate( const QgsVector3D &vector )
{
  m[3][0] += m[0][0] * vector.x() + m[1][0] * vector.y() + m[2][0] * vector.z();
  m[3][1] += m[0][1] * vector.x() + m[1][1] * vector.y() + m[2][1] * vector.z();
  m[3][2] += m[0][2] * vector.x() + m[1][2] * vector.y() + m[2][2] * vector.z();
  m[3][3] += m[0][3] * vector.x() + m[1][3] * vector.y() + m[2][3] * vector.z();
}

QList< double > QgsMatrix4x4::dataList() const
{
  QList< double > res;
  res.reserve( 16 );
  for ( int i = 0; i < 16; ++i )
  {
    res.append( m[i / 4][i % 4] );
  }
  return res;
}

QgsVector3D operator*( const QgsMatrix4x4 &matrix, const QgsVector3D &vector )
{
  double x, y, z, w;

  x = vector.x() * matrix.m[0][0] + vector.y() * matrix.m[1][0] + vector.z() * matrix.m[2][0] + matrix.m[3][0];
  y = vector.x() * matrix.m[0][1] + vector.y() * matrix.m[1][1] + vector.z() * matrix.m[2][1] + matrix.m[3][1];
  z = vector.x() * matrix.m[0][2] + vector.y() * matrix.m[1][2] + vector.z() * matrix.m[2][2] + matrix.m[3][2];
  w = vector.x() * matrix.m[0][3] + vector.y() * matrix.m[1][3] + vector.z() * matrix.m[2][3] + matrix.m[3][3];
  if ( w == 1.0 )
    return QgsVector3D( x, y, z );
  else
    return QgsVector3D( x / w, y / w, z / w );
}

bool QgsMatrix4x4::isIdentity() const
{
  if ( m[0][0] != 1.0 || m[0][1] != 0.0 || m[0][2] != 0.0 )
    return false;
  if ( m[0][3] != 0.0 || m[1][0] != 0.0 || m[1][1] != 1.0 )
    return false;
  if ( m[1][2] != 0.0 || m[1][3] != 0.0 || m[2][0] != 0.0 )
    return false;
  if ( m[2][1] != 0.0 || m[2][2] != 1.0 || m[2][3] != 0.0 )
    return false;
  if ( m[3][0] != 0.0 || m[3][1] != 0.0 || m[3][2] != 0.0 )
    return false;
  return ( m[3][3] == 1.0 );
}

void QgsMatrix4x4::setToIdentity()
{
  m[0][0] = 1.0;
  m[0][1] = 0.0;
  m[0][2] = 0.0;
  m[0][3] = 0.0;
  m[1][0] = 0.0;
  m[1][1] = 1.0;
  m[1][2] = 0.0;
  m[1][3] = 0.0;
  m[2][0] = 0.0;
  m[2][1] = 0.0;
  m[2][2] = 1.0;
  m[2][3] = 0.0;
  m[3][0] = 0.0;
  m[3][1] = 0.0;
  m[3][2] = 0.0;
  m[3][3] = 1.0;
}


QgsMatrix4x4 operator*( const QgsMatrix4x4 &m1, const QgsMatrix4x4 &m2 )
{
  QgsMatrix4x4 m( 1 );
  m.m[0][0] = m1.m[0][0] * m2.m[0][0] + m1.m[1][0] * m2.m[0][1] + m1.m[2][0] * m2.m[0][2] + m1.m[3][0] * m2.m[0][3];
  m.m[0][1] = m1.m[0][1] * m2.m[0][0] + m1.m[1][1] * m2.m[0][1] + m1.m[2][1] * m2.m[0][2] + m1.m[3][1] * m2.m[0][3];
  m.m[0][2] = m1.m[0][2] * m2.m[0][0] + m1.m[1][2] * m2.m[0][1] + m1.m[2][2] * m2.m[0][2] + m1.m[3][2] * m2.m[0][3];
  m.m[0][3] = m1.m[0][3] * m2.m[0][0] + m1.m[1][3] * m2.m[0][1] + m1.m[2][3] * m2.m[0][2] + m1.m[3][3] * m2.m[0][3];

  m.m[1][0] = m1.m[0][0] * m2.m[1][0] + m1.m[1][0] * m2.m[1][1] + m1.m[2][0] * m2.m[1][2] + m1.m[3][0] * m2.m[1][3];
  m.m[1][1] = m1.m[0][1] * m2.m[1][0] + m1.m[1][1] * m2.m[1][1] + m1.m[2][1] * m2.m[1][2] + m1.m[3][1] * m2.m[1][3];
  m.m[1][2] = m1.m[0][2] * m2.m[1][0] + m1.m[1][2] * m2.m[1][1] + m1.m[2][2] * m2.m[1][2] + m1.m[3][2] * m2.m[1][3];
  m.m[1][3] = m1.m[0][3] * m2.m[1][0] + m1.m[1][3] * m2.m[1][1] + m1.m[2][3] * m2.m[1][2] + m1.m[3][3] * m2.m[1][3];

  m.m[2][0] = m1.m[0][0] * m2.m[2][0] + m1.m[1][0] * m2.m[2][1] + m1.m[2][0] * m2.m[2][2] + m1.m[3][0] * m2.m[2][3];
  m.m[2][1] = m1.m[0][1] * m2.m[2][0] + m1.m[1][1] * m2.m[2][1] + m1.m[2][1] * m2.m[2][2] + m1.m[3][1] * m2.m[2][3];
  m.m[2][2] = m1.m[0][2] * m2.m[2][0] + m1.m[1][2] * m2.m[2][1] + m1.m[2][2] * m2.m[2][2] + m1.m[3][2] * m2.m[2][3];
  m.m[2][3] = m1.m[0][3] * m2.m[2][0] + m1.m[1][3] * m2.m[2][1] + m1.m[2][3] * m2.m[2][2] + m1.m[3][3] * m2.m[2][3];

  m.m[3][0] = m1.m[0][0] * m2.m[3][0] + m1.m[1][0] * m2.m[3][1] + m1.m[2][0] * m2.m[3][2] + m1.m[3][0] * m2.m[3][3];
  m.m[3][1] = m1.m[0][1] * m2.m[3][0] + m1.m[1][1] * m2.m[3][1] + m1.m[2][1] * m2.m[3][2] + m1.m[3][1] * m2.m[3][3];
  m.m[3][2] = m1.m[0][2] * m2.m[3][0] + m1.m[1][2] * m2.m[3][1] + m1.m[2][2] * m2.m[3][2] + m1.m[3][2] * m2.m[3][3];
  m.m[3][3] = m1.m[0][3] * m2.m[3][0] + m1.m[1][3] * m2.m[3][1] + m1.m[2][3] * m2.m[3][2] + m1.m[3][3] * m2.m[3][3];
  return m;
}

void QgsMatrix4x4::scale( const QgsVector3D &vector )
{
  m[0][0] *= vector.x();
  m[0][1] *= vector.x();
  m[0][2] *= vector.x();
  m[0][3] *= vector.x();

  m[1][0] *= vector.y();
  m[1][1] *= vector.y();
  m[1][2] *= vector.y();
  m[1][3] *= vector.y();

  m[2][0] *= vector.z();
  m[2][1] *= vector.z();
  m[2][2] *= vector.z();
  m[2][3] *= vector.z();
}

void QgsMatrix4x4::rotate( double angle, double x, double y, double z )
{
  const double length = std::sqrt( x * x + y * y + z * z );
  if ( qgsDoubleNear( length, 0.0 ) )
  {
    return;
  }

  const double nx = x / length;
  const double ny = y / length;
  const double nz = z / length;

  const double angleRad = angle * M_PI / 180.0;
  const double angleCos = std::cos( angleRad );
  const double angleSin = std::sin( angleRad );
  const double angleCosInv = 1.0 - angleCos;

  const double nxny = nx * ny;
  const double nxnz = nx * nz;
  const double nynz = ny * nz;

  // Rotation matrix
  QgsMatrix4x4 rot;
  rot.m[0][0] = angleCosInv * nx * nx + angleCos;
  rot.m[0][1] = angleCosInv * nxny + nz * angleSin;
  rot.m[0][2] = angleCosInv * nxnz - ny * angleSin;
  rot.m[0][3] = 0.0;

  rot.m[1][0] = angleCosInv * nxny - nz * angleSin;
  rot.m[1][1] = angleCosInv * ny * ny + angleCos;
  rot.m[1][2] = angleCosInv * nynz + nx * angleSin;
  rot.m[1][3] = 0.0;

  rot.m[2][0] = angleCosInv * nxnz + ny * angleSin;
  rot.m[2][1] = angleCosInv * nynz - nx * angleSin;
  rot.m[2][2] = angleCosInv * nz * nz + angleCos;
  rot.m[2][3] = 0.0;

  rot.m[3][0] = 0.0;
  rot.m[3][1] = 0.0;
  rot.m[3][2] = 0.0;
  rot.m[3][3] = 1.0;

  *this = *this * rot;
}

void QgsMatrix4x4::rotate( double angle, const QgsVector3D &vector )
{
  rotate( angle, vector.x(), vector.y(), vector.z() );
}
