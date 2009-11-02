/***************************************************************************
    qgsgeometryanalyzer.h - QGIS Tools for vector geometry analysis
                             -------------------
    begin                : 19 March 2009
    copyright            : (C) Carson Farmer
    email                : carson.farmer@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id: qgis.h 9774 2008-12-12 05:41:24Z timlinux $ */

#ifndef QGSGEOMETRYANALYZERH
#define QGSGEOMETRYANALYZERH

#include "qgsvectorlayer.h"
#include "qgsfield.h"
#include "qgsfeature.h"
#include "qgsgeometry.h"
#include "qgsfield.h"
#include "qgsdistancearea.h"


/** \ingroup analysis
 * The QGis class provides vector geometry analysis functions
 */

class ANALYSIS_EXPORT QgsGeometryAnalyzer
{
  public:

    /** 
     * Convert a vector layer from single part geometry 
     * to multipart geometry for a given field 
     *
     * */
    bool singlepartsToMultipart( QgsVectorLayer* layer,
        const QString& shapefileName,
        const QString& fileEncoding,
        const int fieldIndex );

    /** 
     * Convert multipart features to multiple singlepart features. Creates
     * simple polygons and lines.
     */
    bool multipartToSingleparts( QgsVectorLayer* layer,
        const QString& shapefileName,
        const QString& fileEncoding );

    /** 
     * Extract nodes from line and polygon vector layers and output them as
     * points.
     * */
    bool extractNodes( QgsVectorLayer* layer,
        const QString& shapefileName,
        const QString& fileEncoding );

    /**
     * Convert polygon features to line features. Multipart polygons are
     * converted to multiple singlepart lines.
     */
    bool polygonsToLines( QgsVectorLayer* layer,
        const QString& shapefileName,
        const QString& fileEncoding );
        
    /**
     * Add vector layer geometry info to point (XCOORD, YCOORD), line (LENGTH),
     * or polygon (AREA, PERIMETER) layer.
     */
    bool exportGeometryInformation( QgsVectorLayer* layer,
        const QString& shapefileName,
        const QString& fileEncoding );
    
    /**
     * Simplify (generalise) line or polygon vector layers using (a modified)
     * Douglas-Peucker algorithm.
     */
    bool simplifyGeometry( QgsVectorLayer* layer,
        const QString shapefileName,
        const QString fileEncoding,
        const double tolerance );

    /**
     * Calculate the true centroids, or 'center of mass' for each polygon in an
     * input polygon layer.
     */
    bool polygonCentroids( QgsVectorLayer* layer,
        const QString& shapefileName,
        const QString& fileEncoding );

    /**
     * Create a polygon based on the extents of all features (or all 
     * selected features if applicable) and write it out to a shp.
     */
    bool layerExtent( QgsVectorLayer* layer,
        const QString& shapefileName,
        const QString& fileEncoding );
                             
  private:

    QList<double> simpleMeasure( QgsGeometry* geometry );
    double perimeterMeasure( QgsGeometry* geometry, QgsDistanceArea& measure );
    QgsFieldMap checkGeometryFields( QgsVectorLayer* layer, int& index1, int& index2 );
    QgsGeometry* extractLines( QgsGeometry* geometry );
    QgsGeometry* extractAsSingle( QgsGeometry* geometry );
    QgsGeometry* extractAsMulti( QgsGeometry* geometry );
    QgsGeometry* convertGeometry( QgsGeometry* geometry );
    QList<QgsPoint> extractPoints( QgsGeometry* geometry );

};
#endif //QGSVECTORANALYZER
