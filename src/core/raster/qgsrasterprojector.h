/***************************************************************************
    qgsrasterprojector.h - Raster projector
     --------------------------------------
    Date                 : Jan 16, 2011
    Copyright            : (C) 2005 by Radim Blazek
    email                : radim dot blazek at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/* This code takes ideas from WarpBuilder in Geotools.
 * Thank to Ing. Andrea Aime, Ing. Simone Giannecchini and GeoSolutions S.A.S.
 * See : http://geo-solutions.blogspot.com/2011/01/developers-corner-improving.html
 */

#ifndef QGSRASTERPROJECTOR_H
#define QGSRASTERPROJECTOR_H

#include <QVector>
#include <QList>

#include "qgsrectangle.h"
#include "qgscoordinatereferencesystem.h"
#include "qgscoordinatetransform.h"
#include "qgsrasterinterface.h"

#include <cmath>

class QgsPoint;

/** \ingroup core
 * \class QgsRasterProjector
 */
class CORE_EXPORT QgsRasterProjector : public QgsRasterInterface
{
  public:
    /** Precison defines if each pixel is reprojected or approximate reprojection based
     *  on an approximation matrix of reprojected points is used.
     */
    enum Precision
    {
      Approximate = 0, //!< Approximate (default), fast but possibly inaccurate
      Exact = 1,   //!< Exact, precise but slow
    };

    /** \brief QgsRasterProjector implements approximate projection support for
     * it calculates grid of points in source CRS for target CRS + extent
     * which are used to calculate affine transformation matrices.
     */

    QgsRasterProjector( const QgsCoordinateReferenceSystem& theSrcCRS,
                        const QgsCoordinateReferenceSystem& theDestCRS,
                        int theSrcDatumTransform,
                        int theDestDatumTransform,
                        const QgsRectangle& theDestExtent,
                        int theDestRows, int theDestCols,
                        double theMaxSrcXRes, double theMaxSrcYRes,
                        const QgsRectangle& theExtent
                      );

    QgsRasterProjector( const QgsCoordinateReferenceSystem& theSrcCRS,
                        const QgsCoordinateReferenceSystem& theDestCRS,
                        const QgsRectangle& theDestExtent,
                        int theDestRows, int theDestCols,
                        double theMaxSrcXRes, double theMaxSrcYRes,
                        const QgsRectangle& theExtent
                      );
    QgsRasterProjector( const QgsCoordinateReferenceSystem& theSrcCRS,
                        const QgsCoordinateReferenceSystem& theDestCRS,
                        double theMaxSrcXRes, double theMaxSrcYRes,
                        const QgsRectangle& theExtent
                      );
    QgsRasterProjector();
    /** \brief Copy constructor */
    // To avoid synthesized which fails on copy of QgsCoordinateTransform
    // (QObject child) in Python bindings
    QgsRasterProjector( const QgsRasterProjector &projector );

    /** \brief The destructor */
    ~QgsRasterProjector();

    QgsRasterProjector & operator=( const QgsRasterProjector &projector );

    QgsRasterProjector *clone() const override;

    int bandCount() const override;

    QGis::DataType dataType( int bandNo ) const override;

    /** \brief set source and destination CRS */
    void setCRS( const QgsCoordinateReferenceSystem & theSrcCRS, const QgsCoordinateReferenceSystem & theDestCRS,
                 int srcDatumTransform = -1, int destDatumTransform = -1 );

    /** \brief Get source CRS */
    QgsCoordinateReferenceSystem srcCrs() const { return mSrcCRS; }

    /** \brief Get destination CRS */
    QgsCoordinateReferenceSystem destCrs() const { return mDestCRS; }

    /** \brief set maximum source resolution */
    void setMaxSrcRes( double theMaxSrcXRes, double theMaxSrcYRes )
    {
      mMaxSrcXRes = theMaxSrcXRes;
      mMaxSrcYRes = theMaxSrcYRes;
    }

    Precision precision() const { return mPrecision; }
    void setPrecision( Precision precision ) { mPrecision = precision; }
    // Translated precision mode, for use in ComboBox etc.
    static QString precisionLabel( Precision precision );

    QgsRasterBlock *block( int bandNo, const QgsRectangle & extent, int width, int height ) override;

    /** Calculate destination extent and size from source extent and size */
    bool destExtentSize( const QgsRectangle& theSrcExtent, int theSrcXSize, int theSrcYSize,
                         QgsRectangle& theDestExtent, int& theDestXSize, int& theDestYSize );

    /** Calculate destination extent and size from source extent and size */
    static bool extentSize( const QgsCoordinateTransform* ct,
                            const QgsRectangle& theSrcExtent, int theSrcXSize, int theSrcYSize,
                            QgsRectangle& theDestExtent, int& theDestXSize, int& theDestYSize );

  private:
    /** Get source extent */
    QgsRectangle srcExtent() { return mSrcExtent; }

    /** Get/set source width/height */
    int srcRows() { return mSrcRows; }
    int srcCols() { return mSrcCols; }
    void setSrcRows( int theRows ) { mSrcRows = theRows; mSrcXRes = mSrcExtent.height() / mSrcRows; }
    void setSrcCols( int theCols ) { mSrcCols = theCols; mSrcYRes = mSrcExtent.width() / mSrcCols; }

    /** \brief Get source row and column indexes for current source extent and resolution
        If source pixel is outside source extent theSrcRow and theSrcCol are left unchanged.
        @return true if inside source
     */
    bool srcRowCol( int theDestRow, int theDestCol, int *theSrcRow, int *theSrcCol, const QgsCoordinateTransform* ct );

    int dstRows() const { return mDestRows; }
    int dstCols() const { return mDestCols; }

    /** \brief get destination point for _current_ destination position */
    void destPointOnCPMatrix( int theRow, int theCol, double *theX, double *theY );

    /** \brief Get matrix upper left row/col indexes for destination row/col */
    int matrixRow( int theDestRow );
    int matrixCol( int theDestCol );

    /** \brief get destination point for _current_ matrix position */
    QgsPoint srcPoint( int theRow, int theCol );

    /** \brief Get precise source row and column indexes for current source extent and resolution */
    inline bool preciseSrcRowCol( int theDestRow, int theDestCol, int *theSrcRow, int *theSrcCol, const QgsCoordinateTransform* ct );

    /** \brief Get approximate source row and column indexes for current source extent and resolution */
    inline bool approximateSrcRowCol( int theDestRow, int theDestCol, int *theSrcRow, int *theSrcCol );

    /** \brief Calculate matrix */
    void calc();

    /** \brief insert rows to matrix */
    void insertRows( const QgsCoordinateTransform* ct );

    /** \brief insert columns to matrix */
    void insertCols( const QgsCoordinateTransform* ct );

    /** Calculate single control point in current matrix */
    void calcCP( int theRow, int theCol, const QgsCoordinateTransform* ct );

    /** \brief calculate matrix row */
    bool calcRow( int theRow, const QgsCoordinateTransform* ct );

    /** \brief calculate matrix column */
    bool calcCol( int theCol, const QgsCoordinateTransform* ct );

    /** \brief calculate source extent */
    void calcSrcExtent();

    /** \brief calculate minimum source width and height */
    void calcSrcRowsCols();

    /** \brief check error along columns
      * returns true if within threshold */
    bool checkCols( const QgsCoordinateTransform* ct );

    /** \brief check error along rows
      * returns true if within threshold */
    bool checkRows( const QgsCoordinateTransform* ct );

    /** Calculate array of src helper points */
    void calcHelper( int theMatrixRow, QgsPoint *thePoints );

    /** Calc / switch helper */
    void nextHelper();

    /** Get mCPMatrix as string */
    QString cpToString();

    /** Source CRS */
    QgsCoordinateReferenceSystem mSrcCRS;

    /** Destination CRS */
    QgsCoordinateReferenceSystem mDestCRS;

    /** Source datum transformation id (or -1 if none) */
    int mSrcDatumTransform;

    /** Destination datum transformation id (or -1 if none) */
    int mDestDatumTransform;

    /** Destination extent */
    QgsRectangle mDestExtent;

    /** Source extent */
    QgsRectangle mSrcExtent;

    /** Source raster extent */
    QgsRectangle mExtent;

    /** Number of destination rows */
    int mDestRows;

    /** Number of destination columns */
    int mDestCols;

    /** Destination x resolution */
    double mDestXRes;

    /** Destination y resolution */
    double mDestYRes;

    /** Number of source rows */
    int mSrcRows;

    /** Number of source columns */
    int mSrcCols;

    /** Source x resolution */
    double mSrcXRes;

    /** Source y resolution */
    double mSrcYRes;

    /** Number of destination rows per matrix row */
    double mDestRowsPerMatrixRow;

    /** Number of destination cols per matrix col */
    double mDestColsPerMatrixCol;

    /** Grid of source control points */
    QList< QList<QgsPoint> > mCPMatrix;

    /** Grid of source control points transformation possible indicator */
    /* Same size as mCPMatrix */
    QList< QList<bool> > mCPLegalMatrix;

    /** Array of source points for each destination column on top of current CPMatrix grid row */
    /* Warning: using QList is slow on access */
    QgsPoint *pHelperTop;

    /** Array of source points for each destination column on bottom of current CPMatrix grid row */
    /* Warning: using QList is slow on access */
    QgsPoint *pHelperBottom;

    /** Current mHelperTop matrix row */
    int mHelperTopRow;

    /** Number of mCPMatrix columns */
    int mCPCols;
    /** Number of mCPMatrix rows */
    int mCPRows;

    /** Maximum tolerance in destination units */
    double mSqrTolerance;

    /** Maximum source resolution */
    double mMaxSrcXRes;
    double mMaxSrcYRes;

    /** Requested precision */
    Precision mPrecision;

    /** Use approximation (requested precision is Approximate and it is possible to calculate
     *  an approximation matrix with a sufficient precision) */
    bool mApproximate;
};

#endif

