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
     * Sets the \a value for \a row, \a column in the preallocated matrix ``A``.
     *
     * \param row The row index (0-based)
     * \param column The column index (0-based)
     * \param value The value to insert
     *
     * \throws QgsNotSupportedException for QGIS builds without GSL support.
     * \throws QgsInvalidArgumentException if the row or column index is out of bounds.
     */
    void setValue( int row, int column, double value ) SIP_THROW( QgsNotSupportedException );

    /**
     * Sets a value in the preallocated right-hand-side vector ``b``.
     *
     * \param row The row index (0-based)
     * \param value The value to insert
     *
     * \throws QgsNotSupportedException for QGIS builds without GSL support.
     * \throws QgsInvalidArgumentException if the row index is out of bounds.
     */
    void setRightHandSide( int row, double value ) SIP_THROW( QgsNotSupportedException );

    /**
     * Solves the system ``Ax = b`` for a specific active dimension.
     *
     * \param dimension The dimension ``N`` for the ``N × N`` sub-matrix to solve.
     * \param result Solution vector ``x``
     *
     * \returns TRUE on success, FALSE if the matrix is singular and could not be solved.
     *
     * \throws QgsNotSupportedException for QGIS builds without GSL support.
     * \throws QgsInvalidArgumentException for invalid \a dimension values
     */
    bool solve( int dimension, QVector<double> &result SIP_OUT ) SIP_THROW( QgsNotSupportedException, QgsInvalidArgumentException );

  private:
#ifdef SIP_RUN
    QgsMatrixSolver( const QgsMatrixSolver & );
#endif

    struct GslData;
    std::unique_ptr<GslData> mData;
};

#endif // QGSMATRIXSOLVER_H
