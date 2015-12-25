/***************************************************************************
     qgsleastsquares.cpp
     --------------------------------------
    Date                 : Sun Sep 16 12:03:37 AKDT 2007
    Copyright            : (C) 2007 by Gary E. Sherman
    Email                : sherman at mrcc dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <cmath>
#include <stdexcept>

#include <gsl/gsl_linalg.h>
#include <gsl/gsl_blas.h>

#include <QObject>

#include "qgsleastsquares.h"


void QgsLeastSquares::linear( QVector<QgsPoint> mapCoords,
                              QVector<QgsPoint> pixelCoords,
                              QgsPoint& origin, double& pixelXSize, double& pixelYSize )
{
  int n = mapCoords.size();
  if ( n < 2 )
  {
    throw std::domain_error( QObject::tr( "Fit to a linear transform requires at least 2 points." ).toLocal8Bit().constData() );
  }

  double sumPx( 0 ), sumPy( 0 ), sumPx2( 0 ), sumPy2( 0 ), sumPxMx( 0 ), sumPyMy( 0 ), sumMx( 0 ), sumMy( 0 );
  for ( int i = 0; i < n; ++i )
  {
    sumPx += pixelCoords.at( i ).x();
    sumPy += pixelCoords.at( i ).y();
    sumPx2 += std::pow( pixelCoords.at( i ).x(), 2 );
    sumPy2 += std::pow( pixelCoords.at( i ).y(), 2 );
    sumPxMx += pixelCoords.at( i ).x() * mapCoords.at( i ).x();
    sumPyMy += pixelCoords.at( i ).y() * mapCoords.at( i ).y();
    sumMx += mapCoords.at( i ).x();
    sumMy += mapCoords.at( i ).y();
  }

  double deltaX = n * sumPx2 - std::pow( sumPx, 2 );
  double deltaY = n * sumPy2 - std::pow( sumPy, 2 );

  double aX = ( sumPx2 * sumMx - sumPx * sumPxMx ) / deltaX;
  double aY = ( sumPy2 * sumMy - sumPy * sumPyMy ) / deltaY;
  double bX = ( n * sumPxMx - sumPx * sumMx ) / deltaX;
  double bY = ( n * sumPyMy - sumPy * sumMy ) / deltaY;

  origin.setX( aX );
  origin.setY( aY );

  pixelXSize = qAbs( bX );
  pixelYSize = qAbs( bY );
}


void QgsLeastSquares::helmert( QVector<QgsPoint> mapCoords,
                               QVector<QgsPoint> pixelCoords,
                               QgsPoint& origin, double& pixelSize,
                               double& rotation )
{
  int n = mapCoords.size();
  if ( n < 2 )
  {
    throw std::domain_error( QObject::tr( "Fit to a Helmert transform requires at least 2 points." ).toLocal8Bit().constData() );
  }

  double A = 0, B = 0, C = 0, D = 0, E = 0, F = 0, G = 0, H = 0, I = 0, J = 0;
  for ( int i = 0; i < n; ++i )
  {
    A += pixelCoords.at( i ).x();
    B += pixelCoords.at( i ).y();
    C += mapCoords.at( i ).x();
    D += mapCoords.at( i ).y();
    E += mapCoords.at( i ).x() * pixelCoords.at( i ).x();
    F += mapCoords.at( i ).y() * pixelCoords.at( i ).y();
    G += std::pow( pixelCoords.at( i ).x(), 2 );
    H += std::pow( pixelCoords.at( i ).y(), 2 );
    I += mapCoords.at( i ).x() * pixelCoords.at( i ).y();
    J += pixelCoords.at( i ).x() * mapCoords.at( i ).y();
  }

  /* The least squares fit for the parameters { a, b, x0, y0 } is the solution
     to the matrix equation Mx = b, where M and b is given below. I *think*
     that this is correct but I derived it myself late at night. Look at
     helmert.jpg if you suspect bugs. */

  double MData[] = { A,   -B, ( double ) n,    0.,
                     B,    A,    0., ( double ) n,
                     G + H,  0.,    A,    B,
                     0.,    G + H, -B,    A
                   };

  double bData[] = { C,    D,    E + F,  J - I };

  // we want to solve the equation M*x = b, where x = [a b x0 y0]
  gsl_matrix_view M = gsl_matrix_view_array( MData, 4, 4 );
  gsl_vector_view b = gsl_vector_view_array( bData, 4 );
  gsl_vector* x = gsl_vector_alloc( 4 );
  gsl_permutation* p = gsl_permutation_alloc( 4 );
  int s;
  gsl_linalg_LU_decomp( &M.matrix, p, &s );
  gsl_linalg_LU_solve( &M.matrix, p, &b.vector, x );
  gsl_permutation_free( p );

  origin.setX( gsl_vector_get( x, 2 ) );
  origin.setY( gsl_vector_get( x, 3 ) );
  pixelSize = std::sqrt( std::pow( gsl_vector_get( x, 0 ), 2 ) +
                         std::pow( gsl_vector_get( x, 1 ), 2 ) );
  rotation = std::atan2( gsl_vector_get( x, 1 ), gsl_vector_get( x, 0 ) );
}


void QgsLeastSquares::affine( QVector<QgsPoint> mapCoords,
                              QVector<QgsPoint> pixelCoords )
{
  int n = mapCoords.size();
  if ( n < 4 )
  {
    throw std::domain_error( QObject::tr( "Fit to an affine transform requires at least 4 points." ).toLocal8Bit().constData() );
  }

  double A = 0, B = 0, C = 0, D = 0, E = 0, F = 0,
                                         G = 0, H = 0, I = 0, J = 0, K = 0;
  for ( int i = 0; i < n; ++i )
  {
    A += pixelCoords[i].x();
    B += pixelCoords[i].y();
    C += mapCoords[i].x();
    D += mapCoords[i].y();
    E += std::pow( pixelCoords[i].x(), 2 );
    F += std::pow( pixelCoords[i].y(), 2 );
    G += pixelCoords[i].x() * pixelCoords[i].y();
    H += pixelCoords[i].x() * mapCoords[i].x();
    I += pixelCoords[i].y() * mapCoords[i].y();
    J += pixelCoords[i].x() * mapCoords[i].y();
    K += mapCoords[i].x() * pixelCoords[i].y();
  }

  /* The least squares fit for the parameters { a, b, c, d, x0, y0 } is the
     solution to the matrix equation Mx = b, where M and b is given below.
     I *think* that this is correct but I derived it myself late at night.
     Look at affine.jpg if you suspect bugs. */

  double MData[] = { A,    B,    0,    0, ( double ) n,    0,
                     0,    0,    A,    B,    0, ( double ) n,
                     E,    G,    0,    0,    A,    0,
                     G,    F,    0,    0,    B,    0,
                     0,    0,    E,    G,    0,    A,
                     0,    0,    G,    F,    0,    B
                   };

  double bData[] = { C,    D,    H,    K,    J,    I };

  // we want to solve the equation M*x = b, where x = [a b c d x0 y0]
  gsl_matrix_view M = gsl_matrix_view_array( MData, 6, 6 );
  gsl_vector_view b = gsl_vector_view_array( bData, 6 );
  gsl_vector* x = gsl_vector_alloc( 6 );
  gsl_permutation* p = gsl_permutation_alloc( 6 );
  int s;
  gsl_linalg_LU_decomp( &M.matrix, p, &s );
  gsl_linalg_LU_solve( &M.matrix, p, &b.vector, x );
  gsl_permutation_free( p );

}


/**
 * Scales the given coordinates so that the center of gravity is at the origin and the mean distance to the origin is sqrt(2).
 *
 * Also returns 3x3 homogenous matrices which can be used to normalize and de-normalize coordinates.
 */
void normalizeCoordinates( const QVector<QgsPoint> &coords, QVector<QgsPoint> &normalizedCoords,
                           double normalizeMatrix[9], double denormalizeMatrix[9] )
{
  // Calculate center of gravity
  double cogX = 0.0, cogY = 0.0;
  for ( int i = 0; i < coords.size(); i++ )
  {
    cogX += coords[i].x();
    cogY += coords[i].y();
  }
  cogX *= 1.0 / coords.size();
  cogY *= 1.0 / coords.size();

  // Calculate mean distance to origin
  double meanDist = 0.0;
  for ( int i = 0; i < coords.size(); i++ )
  {
    double X = ( coords[i].x() - cogX );
    double Y = ( coords[i].y() - cogY );
    meanDist += sqrt( X * X + Y * Y );
  }
  meanDist *= 1.0 / coords.size();

  double OOD = meanDist / sqrt( 2.0 );
  double D   = 1.0 / OOD;
  normalizedCoords.resize( coords.size() );
  for ( int i = 0; i < coords.size(); i++ )
  {
    normalizedCoords[i] = QgsPoint(( coords[i].x() - cogX ) * D, ( coords[i].y() - cogY ) * D );
  }

  normalizeMatrix[0] =   D;
  normalizeMatrix[1] = 0.0;
  normalizeMatrix[2] = -cogX * D;
  normalizeMatrix[3] = 0.0;
  normalizeMatrix[4] =   D;
  normalizeMatrix[5] = -cogY * D;
  normalizeMatrix[6] = 0.0;
  normalizeMatrix[7] = 0.0;
  normalizeMatrix[8] =   1.0;

  denormalizeMatrix[0] = OOD;
  denormalizeMatrix[1] = 0.0;
  denormalizeMatrix[2] = cogX;
  denormalizeMatrix[3] = 0.0;
  denormalizeMatrix[4] = OOD;
  denormalizeMatrix[5] = cogY;
  denormalizeMatrix[6] = 0.0;
  denormalizeMatrix[7] = 0.0;
  denormalizeMatrix[8] =  1.0;
}

// Fits a homography to the given corresponding points, and
// return it in H (row-major format).
void QgsLeastSquares::projective( QVector<QgsPoint> mapCoords,
                                  QVector<QgsPoint> pixelCoords,
                                  double H[9] )
{
  Q_ASSERT( mapCoords.size() == pixelCoords.size() );

  if ( mapCoords.size() < 4 )
  {
    throw std::domain_error( QObject::tr( "Fitting a projective transform requires at least 4 corresponding points." ).toLocal8Bit().constData() );
  }

  QVector<QgsPoint> mapCoordsNormalized;
  QVector<QgsPoint> pixelCoordsNormalized;

  double normMap[9], denormMap[9];
  double normPixel[9], denormPixel[9];
  normalizeCoordinates( mapCoords, mapCoordsNormalized, normMap, denormMap );
  normalizeCoordinates( pixelCoords, pixelCoordsNormalized, normPixel, denormPixel );
  mapCoords = mapCoordsNormalized;
  pixelCoords = pixelCoordsNormalized;

  // GSL does not support a full SVD, so we artificially add a linear dependent row
  // to the matrix in case the system is underconstrained.
  uint m = qMax( 9u, ( uint )mapCoords.size() * 2u );
  uint n = 9;
  gsl_matrix *S = gsl_matrix_alloc( m, n );

  for ( int i = 0; i < mapCoords.size(); i++ )
  {
    gsl_matrix_set( S, i*2, 0, pixelCoords[i].x() );
    gsl_matrix_set( S, i*2, 1, pixelCoords[i].y() );
    gsl_matrix_set( S, i*2, 2, 1.0 );

    gsl_matrix_set( S, i*2, 3, 0.0 );
    gsl_matrix_set( S, i*2, 4, 0.0 );
    gsl_matrix_set( S, i*2, 5, 0.0 );

    gsl_matrix_set( S, i*2, 6, -mapCoords[i].x()*pixelCoords[i].x() );
    gsl_matrix_set( S, i*2, 7, -mapCoords[i].x()*pixelCoords[i].y() );
    gsl_matrix_set( S, i*2, 8, -mapCoords[i].x()*1.0 );

    gsl_matrix_set( S, i*2 + 1, 0, 0.0 );
    gsl_matrix_set( S, i*2 + 1, 1, 0.0 );
    gsl_matrix_set( S, i*2 + 1, 2, 0.0 );

    gsl_matrix_set( S, i*2 + 1, 3, pixelCoords[i].x() );
    gsl_matrix_set( S, i*2 + 1, 4, pixelCoords[i].y() );
    gsl_matrix_set( S, i*2 + 1, 5, 1.0 );

    gsl_matrix_set( S, i*2 + 1, 6, -mapCoords[i].y()*pixelCoords[i].x() );
    gsl_matrix_set( S, i*2 + 1, 7, -mapCoords[i].y()*pixelCoords[i].y() );
    gsl_matrix_set( S, i*2 + 1, 8, -mapCoords[i].y()*1.0 );
  }

  if ( mapCoords.size() == 4 )
  {
    // The GSL SVD routine only supports matrices with rows >= columns (m >= n)
    // Unfortunately, we can't use the SVD of the transpose (i.e. S^T = (U D V^T)^T = V D U^T)
    // to work around this, because the solution lies in the right nullspace of S, and
    // gsl only supports a thin SVD of S^T, which does not return these vectors.

    // HACK: duplicate last row to get a 9x9 equation system
    for ( int j = 0; j < 9; j++ )
    {
      gsl_matrix_set( S, 8, j, gsl_matrix_get( S, 7, j ) );
    }
  }

  // Solve Sh = 0 in the total least squares sense, i.e.
  // with Sh = min and |h|=1. The solution "h" is given by the
  // right singular eigenvector of S corresponding, to the smallest
  // singular value (via SVD).
  gsl_matrix *V = gsl_matrix_alloc( n, n );
  gsl_vector *singular_values = gsl_vector_alloc( n );
  gsl_vector *work = gsl_vector_alloc( n );

  // V = n x n
  // U = m x n (thin SVD)  U D V^T
  gsl_linalg_SV_decomp( S, V, singular_values, work );

  // Columns of V store the right singular vectors of S
  for ( unsigned int i = 0; i < n; i++ )
  {
    H[i] = gsl_matrix_get( V, i, n - 1 );
  }

  gsl_matrix *prodMatrix = gsl_matrix_alloc( 3, 3 );

  gsl_matrix_view Hmatrix = gsl_matrix_view_array( H, 3, 3 );
  gsl_matrix_view normPixelMatrix = gsl_matrix_view_array( normPixel, 3, 3 );
  gsl_matrix_view denormMapMatrix = gsl_matrix_view_array( denormMap, 3, 3 );

  // Change coordinate frame of image and pre-image from normalized to map and pixel coordinates.
  // H' = denormalizeMapCoords*H*normalizePixelCoords
  gsl_blas_dgemm( CblasNoTrans, CblasNoTrans, 1.0, &Hmatrix.matrix, &normPixelMatrix.matrix, 0.0, prodMatrix );
  gsl_blas_dgemm( CblasNoTrans, CblasNoTrans, 1.0, &denormMapMatrix.matrix, prodMatrix, 0.0, &Hmatrix.matrix );

  gsl_matrix_free( prodMatrix );
  gsl_matrix_free( S );
  gsl_matrix_free( V );
  gsl_vector_free( singular_values );
  gsl_vector_free( work );
}
