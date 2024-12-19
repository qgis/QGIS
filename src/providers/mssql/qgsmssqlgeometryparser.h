/***************************************************************************
                    qgsmssqlgeometryparser.h  -  description
                             -------------------
    begin                : 2014-03-16
    copyright            : (C) 2014 by Tamas Szekeres
    email                : szekerest at gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMSSQLGEOMETRYPARSER_H
#define QGSMSSQLGEOMETRYPARSER_H

#include "qgsabstractgeometry.h"
#include "qgsmultilinestring.h"
#include "qgsmultipolygon.h"
#include "qgsmultipoint.h"
#include "qgslinestring.h"
#include "qgscircularstring.h"
#include "qgscompoundcurve.h"
#include "qgscurvepolygon.h"
#include "qgspolygon.h"
#include "qgspoint.h"



/**
 * \class QgsMssqlGeometryParser
 * \brief Geometry parser for SqlGeometry/SqlGeography.
*/
class QgsMssqlGeometryParser
{

  protected:
    unsigned char *mData = nullptr;
    /* version information */
    char mVersion = 0;
    /* serialization properties */
    char mProps = 0;
    /* point array */
    int mPointSize = 0;
    int mPointPos = 0;
    int mNumPoints = 0;
    /* figure array */
    int mFigurePos = 0;
    int mNumFigures = 0;
    /* shape array */
    int mShapePos = 0;
    int mNumShapes = 0;
    /* segmenttype array */
    int mSegmentPos = 0;
    int mNumSegments = 0;
    int mSegment = 0;
    int mSRSId = 0;

  protected:
    QgsPoint readCoordinates( int iPoint ) const;
    void readCoordinates( int iPoint, int iNextPoint, double *x, double *y, double *z, double *m ) const;
    const QgsPointSequence readPointSequence( int iPoint, int iNextPoint ) const;
    std::unique_ptr< QgsPoint > readPoint( int iShape );
    std::unique_ptr< QgsMultiPoint > readMultiPoint( int iShape );
    std::unique_ptr< QgsLineString > readLineString( int iPoint, int iNextPoint );
    std::unique_ptr< QgsLineString > readLineString( int iFigure );
    std::unique_ptr< QgsCircularString > readCircularString( int iPoint, int iNextPoint );
    std::unique_ptr< QgsCircularString > readCircularString( int iFigure );
    std::unique_ptr< QgsMultiLineString > readMultiLineString( int iShape );
    std::unique_ptr< QgsPolygon > readPolygon( int iShape );
    std::unique_ptr< QgsMultiPolygon > readMultiPolygon( int iShape );
    std::unique_ptr< QgsCompoundCurve > readCompoundCurve( int iFigure );
    std::unique_ptr< QgsCurvePolygon > readCurvePolygon( int iShape );
    std::unique_ptr< QgsGeometryCollection > readGeometryCollection( int iShape );

  public:
    QgsMssqlGeometryParser();
    std::unique_ptr<QgsAbstractGeometry> parseSqlGeometry( unsigned char *pszInput, int nLen );
    int GetSRSId() const { return mSRSId; }
    void DumpMemoryToLog( const char *pszMsg, unsigned char *pszInput, int nLen );
    /* sql geo type */
    bool mIsGeography = false;
};


#endif // QGSMSSQLGEOMETRYPARSER_H
