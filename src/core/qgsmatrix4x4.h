/***************************************************************************
  qgsmatrix4x4.h
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

#ifndef QGSMATRIX4X4_H
#define QGSMATRIX4X4_H

#include "qgis_core.h"
#include "qgis_sip.h"

#include "qgsvector3d.h"


/**
 * \ingroup core
 * \brief A simple 4x4 matrix implementation useful for transformation in 3D space.
 *
 * It is similar to QMatrix4x4, but working with double precision values.
 * Most of the time, doing transform using QMatrix4x4 is fine, however QgsMatrix4x4
 * is useful in situations where single precision floats are not enough.
 * For example, when using transform matrix where translation component has
 * values in order of millions.
 *
 * \warning Non-stable API, exposed to Python for unit testing only.
 *
 * \since QGIS 3.34
 */
class CORE_EXPORT QgsMatrix4x4
{
  public:
    //! Initializes identity matrix
    QgsMatrix4x4() { setToIdentity(); }
    //! Initializes matrix by setting all values in row-major order
    QgsMatrix4x4( double m11, double m12, double m13, double m14,
                  double m21, double m22, double m23, double m24,
                  double m31, double m32, double m33, double m34,
                  double m41, double m42, double m43, double m44 );

    //! Returns pointer to the matrix data (stored in column-major order)
    const double *constData() const SIP_SKIP { return *m; }
    //! Returns pointer to the matrix data (stored in column-major order)
    double *data() SIP_SKIP { return *m; }
    //! Returns matrix data (in column-major order)
    QList< double > dataList() const SIP_PYNAME( data );

    //! Matrix-vector multiplication (vector is converted to homogenous coordinates [X,Y,Z,1] and back)
    QgsVector3D map( const QgsVector3D &vector ) const
    {
      return *this * vector;
    }

    //! Returns whether this matrix is an identity matrix
    bool isIdentity() const;
    //! Sets matrix to be identity matrix
    void setToIdentity();

#ifndef SIP_RUN
    friend QgsMatrix4x4 operator*( const QgsMatrix4x4 &m1, const QgsMatrix4x4 &m2 );
    friend QgsVector3D operator*( const QgsMatrix4x4 &matrix, const QgsVector3D &vector );
#endif

  private:
    // Matrix data - in column-major order
    double m[4][4];

    //! Construct without initializing identity matrix.
    explicit QgsMatrix4x4( int ) { }   // cppcheck-suppress uninitMemberVarPrivate
};

//! Matrix-matrix multiplication (useful to concatenate transforms)
CORE_EXPORT QgsVector3D operator*( const QgsMatrix4x4 &matrix, const QgsVector3D &vector );
//! Matrix-vector multiplication (vector is converted to homogenous coordinates [X,Y,Z,1] and back)
CORE_EXPORT QgsMatrix4x4 operator*( const QgsMatrix4x4 &m1, const QgsMatrix4x4 &m2 );


#endif
