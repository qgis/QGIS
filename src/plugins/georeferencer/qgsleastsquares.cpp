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

#include <QObject>

#include "qgsleastsquares.h"


void QgsLeastSquares::linear( std::vector<QgsPoint> mapCoords,
                              std::vector<QgsPoint> pixelCoords,
                              QgsPoint& origin, double& pixelXSize, double& pixelYSize )
{
  int n = mapCoords.size();
  if ( n < 2 )
  {
    throw std::domain_error( QObject::tr( "Fit to a linear transform requires at least 2 points." ).toLocal8Bit().constData() );
  }

  double sumPx( 0 ), sumPy( 0 ), sumPx2( 0 ), sumPy2( 0 ), sumPxMx( 0 ), sumPyMy( 0 ),
  sumMx( 0 ), sumMy( 0 );
  for ( int i = 0; i < n; ++i )
  {
    sumPx += pixelCoords[i].x();
    sumPy += pixelCoords[i].y();
    sumPx2 += std::pow( pixelCoords[i].x(), 2 );
    sumPy2 += std::pow( pixelCoords[i].y(), 2 );
    sumPxMx += pixelCoords[i].x() * mapCoords[i].x();
    sumPyMy += pixelCoords[i].y() * mapCoords[i].y();
    sumMx += mapCoords[i].x();
    sumMy += mapCoords[i].y();
  }

  double deltaX = n * sumPx2 - std::pow( sumPx, 2 );
  double deltaY = n * sumPy2 - std::pow( sumPy, 2 );

  double aX = ( sumPx2 * sumMx - sumPx * sumPxMx ) / deltaX;
  double aY = ( sumPy2 * sumMy - sumPy * sumPyMy ) / deltaY;
  double bX = ( n * sumPxMx - sumPx * sumMx ) / deltaX;
  double bY = ( n * sumPyMy - sumPy * sumMy ) / deltaY;

  origin.setX( aX );
  origin.setY( aY );

  pixelXSize = std::abs( bX );
  pixelYSize = std::abs( bY );
}


void QgsLeastSquares::helmert( std::vector<QgsPoint> mapCoords,
                               std::vector<QgsPoint> pixelCoords,
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
    A += pixelCoords[i].x();
    B += pixelCoords[i].y();
    C += mapCoords[i].x();
    D += mapCoords[i].y();
    E += mapCoords[i].x() * pixelCoords[i].x();
    F += mapCoords[i].y() * pixelCoords[i].y();
    G += std::pow( pixelCoords[i].x(), 2 );
    H += std::pow( pixelCoords[i].y(), 2 );
    I += mapCoords[i].x() * pixelCoords[i].y();
    J += pixelCoords[i].x() * mapCoords[i].y();
  }

  /* The least squares fit for the parameters { a, b, x0, y0 } is the solution
     to the matrix equation Mx = b, where M and b is given below. I *think*
     that this is correct but I derived it myself late at night. Look at
     helmert.jpg if you suspect bugs. */

  double MData[] = { A,   -B,    n,    0,
                     B,    A,    0,    n,
                     G + H,  0,    A,    B,
                     0,    G + H, -B,    A
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


void QgsLeastSquares::affine( std::vector<QgsPoint> mapCoords,
                              std::vector<QgsPoint> pixelCoords )
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

  double MData[] = { A,    B,    0,    0,    n,    0,
                     0,    0,    A,    B,    0,    n,
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

