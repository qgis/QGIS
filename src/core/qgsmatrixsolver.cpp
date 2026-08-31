/***************************************************************************
    qgsmatrixsolver.cpp
    ----------------------
    begin                : August 2026
    copyright            : (C) 2026 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgsconfig.h"
#include "qgsmatrixsolver.h"

#include <mutex>

#include "qgsexception.h"

#include <QString>

using namespace Qt::StringLiterals;

#ifdef HAVE_GSL
#define GSL_RANGE_CHECK_OFF
#define HAVE_INLINE 1

#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_linalg.h>
#include <gsl/gsl_errno.h>
#endif

struct QgsMatrixSolver::GslData
{
    int maximumDimension = 0;
#ifdef HAVE_GSL
    gsl_matrix *maxMatrix = nullptr;
    gsl_vector *maxVectorB = nullptr;
    gsl_vector *maxVectorX = nullptr;

    // cached permutations for all possible sizes up to maxSize to avoid allocations
    std::vector<gsl_permutation *> permutations;

    gsl_matrix *matrixA_Copy = nullptr;
    gsl_matrix *matrixV = nullptr;
    gsl_vector *vectorS = nullptr;
    gsl_vector *workSvd = nullptr;
#endif
};

bool QgsMatrixSolver::isAvailable()
{
#ifdef HAVE_GSL
  return true;
#else
  return false;
#endif
}

QgsMatrixSolver::QgsMatrixSolver( int maximumDimension )
  : mData( std::make_unique<GslData>() )
{
  if ( maximumDimension <= 0 )
  {
    return;
  }

  mData->maximumDimension = maximumDimension;
#ifdef HAVE_GSL
  // disable GSL aborts for singular matrices -- we handle this via return code instead
  static std::once_flag sGslErrorHandlerFlag;
  std::call_once( sGslErrorHandlerFlag, []() { gsl_set_error_handler_off(); } );

  mData->maxMatrix = gsl_matrix_calloc( maximumDimension, maximumDimension );
  mData->maxVectorB = gsl_vector_calloc( maximumDimension );
  mData->maxVectorX = gsl_vector_alloc( maximumDimension );

  mData->matrixA_Copy = gsl_matrix_calloc( maximumDimension, maximumDimension );
  mData->matrixV = gsl_matrix_calloc( maximumDimension, maximumDimension );
  mData->vectorS = gsl_vector_calloc( maximumDimension );
  mData->workSvd = gsl_vector_alloc( maximumDimension );

  // Preallocate permutation arrays for all possible dynamic sizes 1 to maxSize
  mData->permutations.resize( maximumDimension + 1, nullptr );
  for ( int i = 1; i <= maximumDimension; ++i )
  {
    mData->permutations[i] = gsl_permutation_alloc( i );
  }
#endif
}

QgsMatrixSolver::~QgsMatrixSolver()
{
#ifdef HAVE_GSL
  if ( mData->maxMatrix )
  {
    gsl_matrix_free( mData->maxMatrix );
  }
  if ( mData->maxVectorB )
  {
    gsl_vector_free( mData->maxVectorB );
  }
  if ( mData->maxVectorX )
  {
    gsl_vector_free( mData->maxVectorX );
  }
  if ( mData->matrixA_Copy )
  {
    gsl_matrix_free( mData->matrixA_Copy );
  }
  if ( mData->matrixV )
  {
    gsl_matrix_free( mData->matrixV );
  }
  if ( mData->vectorS )
  {
    gsl_vector_free( mData->vectorS );
  }
  if ( mData->workSvd )
  {
    gsl_vector_free( mData->workSvd );
  }

  for ( auto *p : mData->permutations )
  {
    if ( p )
    {
      gsl_permutation_free( p );
    }
  }
#endif
}

int QgsMatrixSolver::maximumDimension() const
{
  return mData->maximumDimension;
}

void QgsMatrixSolver::setValue( int row, int column, double value )
{
#ifdef HAVE_GSL
  gsl_matrix_set( mData->maxMatrix, row, column, value );
#else
  ( void ) row;
  ( void ) column;
  ( void ) value;
  throw QgsNotSupportedException( u"QgsMatrixSolver requires a QGIS build with GSL support enabled"_s );
#endif
}

void QgsMatrixSolver::setRightHandSide( int row, double value )
{
#ifdef HAVE_GSL
  gsl_vector_set( mData->maxVectorB, row, value );
#else
  ( void ) row;
  ( void ) value;
  throw QgsNotSupportedException( u"QgsMatrixSolver requires a QGIS build with GSL support enabled"_s );
#endif
}

bool QgsMatrixSolver::solve( int dimension, QVector<double> &result, Qgis::LinearMatrixMethod method )
{
  if ( dimension <= 0 )
  {
    throw QgsInvalidArgumentException( u"Invalid value for dimension, must be > 0"_s );
  }
  if ( dimension > mData->maximumDimension )
  {
    throw QgsInvalidArgumentException( u"Invalid value for dimension, must be < %1"_s.arg( mData->maximumDimension ) );
  }
  switch ( method )
  {
    case Qgis::LinearMatrixMethod::Lu:
      return solveLu( dimension, result, false );

    case Qgis::LinearMatrixMethod::Svd:
      return solveSvd( dimension, result, false );

    case Qgis::LinearMatrixMethod::LuWithSvdFallback:
    {
      if ( solveLu( dimension, result, true ) )
      {
        return true;
      }
      return solveSvd( dimension, result, false );
    }
  }

  return false;
}

bool QgsMatrixSolver::solveLu( int dimension, QVector<double> &result, bool retainOriginalMatrices )
{
#ifdef HAVE_GSL
  // create views into the preallocated memory block
  gsl_matrix_view A_view = gsl_matrix_submatrix( mData->maxMatrix, 0, 0, dimension, dimension );
  gsl_matrix_view A_copy = gsl_matrix_submatrix( mData->matrixA_Copy, 0, 0, dimension, dimension );

  gsl_vector_view B_view = gsl_vector_subvector( mData->maxVectorB, 0, dimension );
  gsl_vector_view X_view = gsl_vector_subvector( mData->maxVectorX, 0, dimension );

  gsl_matrix *A = &A_view.matrix;
  if ( retainOriginalMatrices )
  {
    gsl_matrix_memcpy( &A_copy.matrix, &A_view.matrix );
    A = &A_copy.matrix;
  }

  gsl_permutation *p = mData->permutations[dimension];
  int signum = 0;
  gsl_linalg_LU_decomp( A, p, &signum );
  if ( gsl_linalg_LU_solve( A, p, &B_view.vector, &X_view.vector ) != 0 )
  {
    return false;
  }

  result.resize( dimension );
  for ( int i = 0; i < dimension; ++i )
  {
    result[i] = gsl_vector_get( &X_view.vector, i );
  }

  return true;
#else
  ( void ) dimension;
  ( void ) result;
  throw QgsNotSupportedException( u"QgsMatrixSolver requires a QGIS build with GSL support enabled"_s );
#endif
}

bool QgsMatrixSolver::solveSvd( int dimension, QVector<double> &result, bool retainOriginalMatrices )
{
#ifdef HAVE_GSL
  gsl_matrix_view A_view = gsl_matrix_submatrix( mData->maxMatrix, 0, 0, dimension, dimension );
  gsl_matrix_view A_copy = gsl_matrix_submatrix( mData->matrixA_Copy, 0, 0, dimension, dimension );
  gsl_matrix_view V = gsl_matrix_submatrix( mData->matrixV, 0, 0, dimension, dimension );
  gsl_vector_view S = gsl_vector_subvector( mData->vectorS, 0, dimension );
  gsl_vector_view b = gsl_vector_subvector( mData->maxVectorB, 0, dimension );
  gsl_vector_view x = gsl_vector_subvector( mData->maxVectorX, 0, dimension );
  gsl_vector_view work = gsl_vector_subvector( mData->workSvd, 0, dimension );

  gsl_matrix *A = &A_view.matrix;
  if ( retainOriginalMatrices )
  {
    gsl_matrix_memcpy( &A_copy.matrix, &A_view.matrix );
    A = &A_copy.matrix;
  }

  // Compute SVD: A_copy = U * S * V^T
  if ( gsl_linalg_SV_decomp( A, &V.matrix, &S.vector, &work.vector ) != 0 )
  {
    return false;
  }

  // Solve SVD: U * S * V^T * x = b with singular value truncation
  // gsl_linalg_SV_solve automatically uses pseudo-inverse for singular values < tol
  if ( gsl_linalg_SV_solve( A, &V.matrix, &S.vector, &b.vector, &x.vector ) != 0 )
  {
    return false;
  }

  result.resize( dimension );
  for ( int i = 0; i < dimension; ++i )
  {
    result[i] = gsl_vector_get( &x.vector, i );
  }

  return true;
#else
  ( void ) dimension;
  ( void ) result;
  throw QgsNotSupportedException( u"QgsMatrixSolver requires a QGIS build with GSL support enabled"_s );
#endif
}
