/***************************************************************************
    qgsmatrixsolver.h
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

#include "qgis.h"
#include "qgis_core.h"
#include "qgis_sip.h"

#include <QObject>

#ifndef QGSMATRIXSOLVER_H
#define QGSMATRIXSOLVER_H

/**
 * \ingroup core
 * \brief Contains utility functions for solving matrix operations.
 *
 * This utility class is designed to solve systems of linear equations in the form
 * ``Ax = b`` using LU decomposition.
 *
 * \note In situations where many linear systems need to be solved sequentially this class
 * supports a preallocation strategy. By constructing the solver initially with a \a maximumDimension
 * (where \a maximumDimension is ``N`` for the largest ``N × N`` matrix to solve),
 * all backend memory is allocated upfront. Subsequent calls to solve() using a specified \a dimension
 * create zero-allocation views into this memory block. This guarantees that no heap
 * allocations occur during calculations.
 *
 * \warning This class requires a QGIS build with the GSL library enabled. See isAvailable() to determine
 * if the current system supports its functionality.
 *
 * \since QGIS 4.4
 */
class CORE_EXPORT QgsMatrixSolver
{
  public:
    /**
     * Returns TRUE if the matrix solver functionality is available on the current system.
     */
    static bool isAvailable();

    /**
     * Constructor for QgsMatrixSolver, pre-allocated to solve matrices with the specified \a maximumDimension.
     *
     * \param maximumDimension The dimension ``N`` for the largest ``N × N`` matrix ``A`` to solve with this object. (E.g.
     * 4 if a ``4 × 4`` matrix is the largest ``A`` to solve.)
     */
    explicit QgsMatrixSolver( int maximumDimension );

    ~QgsMatrixSolver();

#ifndef SIP_RUN
    QgsMatrixSolver( const QgsMatrixSolver & ) = delete;
    QgsMatrixSolver &operator=( const QgsMatrixSolver & ) = delete;
#endif

    /**
     * Returns the maximum dimension supported by this solver.
     */
    int maximumDimension() const;

#ifndef SIP_RUN
    /**
     * Sets the \a value for \a row, \a column in the preallocated matrix ``A``.
     *
     * \param row The row index (0-based)
     * \param column The column index (0-based)
     * \param value The value to insert
     *
     * \warning The c++ version of this method performs no bounds checking on \a row or \a column, this is the caller's responsibility.
     *
     * \throws QgsNotSupportedException for QGIS builds without GSL support.
     */
    void setValue( int row, int column, double value );
#else
    // clang-format off

    /**
     * Sets the \a value for \a row, \a column in the preallocated matrix ``A``.
     *
     * \param row The row index (0-based)
     * \param column The column index (0-based)
     * \param value The value to insert
     *
     * \throws QgsNotSupportedException for QGIS builds without GSL support.
     * \throws IndexError if the row or column index is out of bounds.
     */
    void setValue( int row, int column, double value );
  % MethodCode
    const int maximumDimension = sipCpp->maximumDimension();
    if ( !QgsMatrixSolver::isAvailable() )
    {
      PyErr_SetString( sipException_QgsNotSupportedException, "QgsMatrixSolver requires a QGIS build with GSL support enabled" );
      sipIsErr = 1;
    }
    else if ( a0 < 0 || a0 >= maximumDimension  )
    {
      PyErr_SetString( PyExc_IndexError, QByteArray::number( a0 ) );
      sipIsErr = 1;
    }
    else if ( a1 < 0 || a1 >= maximumDimension )
    {
      PyErr_SetString( PyExc_IndexError, QByteArray::number( a1 ) );
      sipIsErr = 1;
    }
    else
    {
      sipCpp->setValue( a0, a1, a2 );
    }
    % End
// clang-format on
#endif

#ifndef SIP_RUN
    /**
     * Sets a value in the preallocated right-hand-side vector ``b``.
     *
     * \param row The row index (0-based)
     * \param value The value to insert
     *
     * \warning The c++ version of this method performs no bounds checking on \a row, this is the caller's responsibility.
     *
     * \throws QgsNotSupportedException for QGIS builds without GSL support.
     */
    void setRightHandSide( int row, double value );
#else
      // clang-format off

      /**
       * Sets a value in the preallocated right-hand-side vector ``b``.
       * \param row The row index (0-based)
       * \param value The value to insert
       * \throws QgsNotSupportedException for QGIS builds without GSL support.
       * \throws IndexError if the row index is out of bounds.
       */
      void setRightHandSide( int row, double value ) SIP_THROW( QgsNotSupportedException );
      % MethodCode
      const int maximumDimension = sipCpp->maximumDimension();
      if ( !QgsMatrixSolver::isAvailable() )
      {
        PyErr_SetString( sipException_QgsNotSupportedException, "QgsMatrixSolver requires a QGIS build with GSL support enabled" );
        sipIsErr = 1;
      }
      else if ( a0 < 0 || a0 >= maximumDimension )
      {
        PyErr_SetString( PyExc_IndexError, QByteArray::number( a0 ) );
        sipIsErr = 1;
      }
      else
      {
        sipCpp->setRightHandSide( a0, a1 );
      }
      % End
    // clang-format on
#endif

    /**
     * Solves the system ``Ax = b`` for a specific active dimension.
     *
     * \param dimension The dimension ``N`` for the ``N × N`` sub-matrix to solve.
     * \param result Solution vector ``x``
     * \param method Solving strategy to use
     *
     * \returns TRUE on success, FALSE if the matrix is singular and could not be solved.
     *
     * \throws QgsNotSupportedException for QGIS builds without GSL support.
     * \throws QgsInvalidArgumentException for invalid \a dimension values
     */
    bool solve( int dimension, QVector<double> &result SIP_OUT, Qgis::LinearMatrixMethod method = Qgis::LinearMatrixMethod::Lu ) SIP_THROW( QgsNotSupportedException, QgsInvalidArgumentException );

  private:
#ifdef SIP_RUN
    QgsMatrixSolver( const QgsMatrixSolver & );
#endif

    bool solveLu( int dimension, QVector<double> &result, bool retainOriginalMatrices );
    bool solveSvd( int dimension, QVector<double> &result, bool retainOriginalMatrices );

    struct GslData;
    std::unique_ptr<GslData> mData;
};

#endif // QGSMATRIXSOLVER_H
