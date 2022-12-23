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

#include "qgis_core.h"
#include "qgis_sip.h"
#include <QVector>
#include <QList>

#include "qgsrectangle.h"
#include "qgscoordinatereferencesystem.h"
#include "qgscoordinatetransform.h"
#include "qgsrasterinterface.h"

#include <cmath>

class QgsPointXY;

/**
 * \ingroup core
 * \brief QgsRasterProjector implements approximate projection support for
 * it calculates grid of points in source CRS for target CRS + extent
 * which are used to calculate affine transformation matrices.
 * \class QgsRasterProjector
 */
class CORE_EXPORT QgsRasterProjector : public QgsRasterInterface
{
    Q_GADGET

  public:

    /**
     * Precision defines if each pixel is reprojected or approximate reprojection based
     *  on an approximation matrix of reprojected points is used.
     */
    enum Precision
    {
      Approximate = 0, //!< Approximate (default), fast but possibly inaccurate
      Exact = 1,   //!< Exact, precise but slow
    };
    Q_ENUM( Precision )

    QgsRasterProjector();

    QgsRasterProjector *clone() const override SIP_FACTORY;

    int bandCount() const override;

    Qgis::DataType dataType( int bandNo ) const override;

    /**
     * Sets the source and destination CRS
     * \deprecated since QGIS 3.8, use transformContext version instead
     */
    Q_DECL_DEPRECATED void setCrs( const QgsCoordinateReferenceSystem &srcCRS, const QgsCoordinateReferenceSystem &destCRS,
                                   int srcDatumTransform = -1, int destDatumTransform = -1 ) SIP_DEPRECATED;

    /**
     * Sets source CRS to \a srcCRS and destination CRS to \a destCRS and the transformation context to \a transformContext
     * \since QGIS 3.8
     */
    void setCrs( const QgsCoordinateReferenceSystem &srcCRS, const QgsCoordinateReferenceSystem &destCRS,
                 QgsCoordinateTransformContext transformContext );

    //! Returns the source CRS
    QgsCoordinateReferenceSystem sourceCrs() const { return mSrcCRS; }

    //! Returns the destination CRS
    QgsCoordinateReferenceSystem destinationCrs() const { return mDestCRS; }

    Precision precision() const { return mPrecision; }
    void setPrecision( Precision precision ) { mPrecision = precision; }
    // Translated precision mode, for use in ComboBox etc.
    static QString precisionLabel( Precision precision );

    QgsRasterBlock *block( int bandNo, const QgsRectangle &extent, int width, int height, QgsRasterBlockFeedback *feedback = nullptr ) override SIP_FACTORY;

    //! Calculate destination extent and size from source extent and size
    bool destExtentSize( const QgsRectangle &srcExtent, int srcXSize, int srcYSize,
                         QgsRectangle &destExtent SIP_OUT, int &destXSize SIP_OUT, int &destYSize SIP_OUT );

    //! Calculate destination extent and size from source extent and size
    static bool extentSize( const QgsCoordinateTransform &ct,
                            const QgsRectangle &srcExtent, int srcXSize, int srcYSize,
                            QgsRectangle &destExtent SIP_OUT, int &destXSize SIP_OUT, int &destYSize SIP_OUT );

  private:

    //! Source CRS
    QgsCoordinateReferenceSystem mSrcCRS;

    //! Destination CRS
    QgsCoordinateReferenceSystem mDestCRS;

    //! Source datum transformation id (or -1 if none)
    Q_DECL_DEPRECATED int mSrcDatumTransform = -1;

    //! Destination datum transformation id (or -1 if none)
    Q_DECL_DEPRECATED int mDestDatumTransform = -1;

    //! Requested precision
    Precision mPrecision = Approximate;

    QgsCoordinateTransformContext mTransformContext;

};


#ifndef SIP_RUN
/// @cond PRIVATE

/**
 * Internal class for reprojection of rasters - either exact or approximate.
 * QgsRasterProjector creates it and then keeps calling srcRowCol() to get source pixel position
 * for every destination pixel position.
 */
class ProjectorData
{
  public:
    //! Initialize reprojector and calculate matrix
    ProjectorData( const QgsRectangle &extent, int width, int height, QgsRasterInterface *input, const QgsCoordinateTransform &inverseCt, QgsRasterProjector::Precision precision, QgsRasterBlockFeedback *feedback = nullptr );
    ~ProjectorData();

    ProjectorData( const ProjectorData &other ) = delete;
    ProjectorData &operator=( const ProjectorData &other ) = delete;

    /**
     * Returns the source row and column indexes for current source extent and resolution.
     * If the source pixel is outside source extent srcRow and srcCol are left unchanged.
     * \returns TRUE if inside source
     */
    bool srcRowCol( int destRow, int destCol, int *srcRow, int *srcCol );

    QgsRectangle srcExtent() const { return mSrcExtent; }
    int srcRows() const { return mSrcRows; }
    int srcCols() const { return mSrcCols; }

  private:

    //! Returns the destination point for _current_ destination position.
    void destPointOnCPMatrix( int row, int col, double *theX, double *theY );

    //! Returns the matrix upper left row index for destination row.
    int matrixRow( int destRow );

    //! Returns the matrix upper left col index for destination col.
    int matrixCol( int destCol );

    //! Returns precise source row and column indexes for current source extent and resolution.
    inline bool preciseSrcRowCol( int destRow, int destCol, int *srcRow, int *srcCol );

    //! Returns approximate source row and column indexes for current source extent and resolution.
    inline bool approximateSrcRowCol( int destRow, int destCol, int *srcRow, int *srcCol );

    //! \brief insert rows to matrix
    void insertRows( const QgsCoordinateTransform &ct );

    //! \brief insert columns to matrix
    void insertCols( const QgsCoordinateTransform &ct );

    //! Calculate single control point in current matrix
    void calcCP( int row, int col, const QgsCoordinateTransform &ct );

    //! \brief calculate matrix row
    bool calcRow( int row, const QgsCoordinateTransform &ct );

    //! \brief calculate matrix column
    bool calcCol( int col, const QgsCoordinateTransform &ct );

    //! \brief calculate source extent
    void calcSrcExtent();

    //! \brief calculate minimum source width and height
    void calcSrcRowsCols();

    /**
     * \brief check error along columns
     * returns TRUE if within threshold
    */
    bool checkCols( const QgsCoordinateTransform &ct );

    /**
     * \brief check error along rows
     * returns TRUE if within threshold
    */
    bool checkRows( const QgsCoordinateTransform &ct );

    //! Calculate array of src helper points
    void calcHelper( int matrixRow, QgsPointXY *points );

    //! Calc / switch helper
    void nextHelper();

    //! Gets mCPMatrix as string
    QString cpToString() const;

    /**
     * Use approximation (requested precision is Approximate and it is possible to calculate
     * an approximation matrix with a sufficient precision).
    */
    bool mApproximate;

    //! Transformation from destination CRS to source CRS
    QgsCoordinateTransform mInverseCt;

    //! Destination extent
    QgsRectangle mDestExtent;

    //! Source extent
    QgsRectangle mSrcExtent;

    //! Source raster extent
    QgsRectangle mExtent;

    //! Number of destination rows
    int mDestRows;

    //! Number of destination columns
    int mDestCols;

    //! Destination x resolution
    double mDestXRes;

    //! Destination y resolution
    double mDestYRes;

    //! Number of source rows
    int mSrcRows;

    //! Number of source columns
    int mSrcCols;

    //! Source x resolution
    double mSrcXRes;

    //! Source y resolution
    double mSrcYRes;

    //! Number of destination rows per matrix row
    double mDestRowsPerMatrixRow;

    //! Number of destination cols per matrix col
    double mDestColsPerMatrixCol;

    //! Grid of source control points
    QList< QList<QgsPointXY> > mCPMatrix;

    //! Grid of source control points transformation possible indicator
    /* Same size as mCPMatrix */
    QList< QList<bool> > mCPLegalMatrix;

    //! Array of source points for each destination column on top of current CPMatrix grid row
    /* Warning: using QList is slow on access */
    QgsPointXY *pHelperTop = nullptr;

    //! Array of source points for each destination column on bottom of current CPMatrix grid row
    /* Warning: using QList is slow on access */
    QgsPointXY *pHelperBottom = nullptr;

    //! Current mHelperTop matrix row
    int mHelperTopRow;

    //! Number of mCPMatrix columns
    int mCPCols;
    //! Number of mCPMatrix rows
    int mCPRows;

    //! Maximum tolerance in destination units
    double mSqrTolerance;

    //! Maximum source resolution
    double mMaxSrcXRes;
    double mMaxSrcYRes;

};

/// @endcond
#endif

#endif

