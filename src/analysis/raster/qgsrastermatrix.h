/***************************************************************************
                          qgsrastermatrix.h
                          -----------------
    begin                : 2010-10-23
    copyright            : (C) 20010 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSRASTERMATRIX_H
#define QGSRASTERMATRIX_H

#include "qgis_analysis.h"
#include "qgis_sip.h"

/**
 * \ingroup analysis
 * \class QgsRasterMatrix
 * \brief Represents a matrix in a raster calculator operation.
 */
class ANALYSIS_EXPORT QgsRasterMatrix
{
  public:

    enum TwoArgOperator
    {
      opPLUS,
      opMINUS,
      opMUL,
      opDIV,
      opPOW,
      opEQ,         // =
      opNE,         // != resp. <>
      opGT,         // >
      opLT,         // <
      opGE,         // >=
      opLE,         // <=
      opAND,
      opOR,
      opMIN,
      opMAX
    };

    enum OneArgOperator
    {
      opSQRT,
      opSIN,
      opCOS,
      opTAN,
      opASIN,
      opACOS,
      opATAN,
      opSIGN,
      opLOG,
      opLOG10,
      opABS,
    };

    QgsRasterMatrix() = default;

    /**
     * Takes ownership of \a data array.
     * \note note available in Python bindings
     */
    QgsRasterMatrix( int nCols, int nRows, double *data, double nodataValue ) SIP_SKIP;
    QgsRasterMatrix( const QgsRasterMatrix &m );
    ~QgsRasterMatrix();

    //! Returns TRUE if matrix is 1x1 (=scalar number)
    bool isNumber() const { return ( mColumns == 1 && mRows == 1 ); }
    double number() const { return mData[0]; }

    /**
     * Returns data array (but not ownership)
     * \note not available in Python bindings
     */
    double *data() { return mData; } SIP_SKIP

    /**
     * Returns data and ownership. Sets data and nrows, ncols of this matrix to 0
     * \note not available in Python bindings
     */
    double *takeData() SIP_SKIP;

    void setData( int cols, int rows, double *data, double nodataValue );

    int nColumns() const { return mColumns; }
    int nRows() const { return mRows; }

    double nodataValue() const { return mNodataValue; }
    void setNodataValue( double d ) { mNodataValue = d; }

    QgsRasterMatrix &operator=( const QgsRasterMatrix &m );
    //! Adds another matrix to this one
    bool add( const QgsRasterMatrix &other );
    //! Subtracts another matrix from this one
    bool subtract( const QgsRasterMatrix &other );
    bool multiply( const QgsRasterMatrix &other );
    bool divide( const QgsRasterMatrix &other );
    bool power( const QgsRasterMatrix &other );
    bool equal( const QgsRasterMatrix &other );
    bool notEqual( const QgsRasterMatrix &other );
    bool greaterThan( const QgsRasterMatrix &other );
    bool lesserThan( const QgsRasterMatrix &other );
    bool greaterEqual( const QgsRasterMatrix &other );
    bool lesserEqual( const QgsRasterMatrix &other );
    bool logicalAnd( const QgsRasterMatrix &other );
    bool logicalOr( const QgsRasterMatrix &other );

    /**
     * Calculates the maximum value between two matrices
     * \return TRUE on success
     * \since QGIS 3.10
     */
    bool max( const QgsRasterMatrix &other );

    /**
     * Calculates the minimum value between two matrices
     * \return TRUE on success
     * \since QGIS 3.10
     */
    bool min( const QgsRasterMatrix &other );

    bool squareRoot();
    bool sinus();
    bool asinus();
    bool cosinus();
    bool acosinus();
    bool tangens();
    bool atangens();
    bool changeSign();
    bool log();
    bool log10();

    /**
     * Calculates the absolute value
     * \return TRUE on success
     * \since QGIS 3.10
     */
    bool absoluteValue();

  private:
    int mColumns = 0;
    int mRows = 0;
    double *mData = nullptr;
    double mNodataValue = -1;

    //! +,-,*,/,^,<,>,<=,>=,=,!=, and, or
    bool twoArgumentOperation( TwoArgOperator op, const QgsRasterMatrix &other );
    double calculateTwoArgumentOp( TwoArgOperator op, double arg1, double arg2 ) const;

    /*sqrt, std::sin, std::cos, tan, asin, acos, atan*/
    bool oneArgumentOperation( OneArgOperator op );
    bool testPowerValidity( double base, double power ) const;
};

#endif // QGSRASTERMATRIX_H
