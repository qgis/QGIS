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
\class QgsMssqlGeometryParser
\brief Geometry parser for SqlGeometry/SqlGeography.
*
*/

class QgsMssqlGeometryParser
{

  protected:
    unsigned char *pszData = nullptr;
    /* version information */
    char chVersion = 0;
    /* serialization properties */
    char chProps = 0;
    /* point array */
    int nPointSize = 0;
    int nPointPos = 0;
    int nNumPoints = 0;
    /* figure array */
    int nFigurePos = 0;
    int nNumFigures = 0;
    /* shape array */
    int nShapePos = 0;
    int nNumShapes = 0;
    /* segmenttype array */
    int nSegmentPos = 0;
    int nNumSegments = 0;
    int iSegment = 0;
    int nSRSId = 0;

  protected:
    QgsPoint readCoordinates( int iPoint );
    const QgsPointSequence readPointSequence( int iPoint, int iNextPoint );
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
    int GetSRSId() { return nSRSId; }
    void DumpMemoryToLog( const char *pszMsg, unsigned char *pszInput, int nLen );
    /* sql geo type */
    bool IsGeography = false;
};


#endif // QGSMSSQLGEOMETRYPARSER_H
