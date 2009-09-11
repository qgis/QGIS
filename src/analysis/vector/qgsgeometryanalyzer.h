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

class QgsVectorFileWriter;
class QProgressDialog;


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

    /**Create buffers for a vector layer and write it to a new shape file
      @param layer input vector layer
      @param shapefileName path to the output shp
      @param fileEncoding encoding of the output file
      @param bufferDistance distance for buffering (if no buffer field is specified)
      @param onlySelectedFeatures if true, only selected features are considered, else all the features
      @param dissolve if true, merge all the buffers to a big multipolygon
      @param bufferDistanceField index of the attribute field that contains the buffer distance (or -1 if all features have the same buffer distance)
      @param p progress dialog (or 0 if no progress dialog is to be shown)
      @note: added in version 1.3*/
    bool buffer( QgsVectorLayer* layer, const QString& shapefileName, double bufferDistance, \
                 bool onlySelectedFeatures = false, bool dissolve = false, int bufferDistanceField = -1, QProgressDialog* p = 0 );

  private:

    QList<double> simpleMeasure( QgsGeometry* geometry );
    double perimeterMeasure( QgsGeometry* geometry, QgsDistanceArea& measure );
    QgsFieldMap checkGeometryFields( QgsVectorLayer* layer, int& index1, int& index2 );
    QgsGeometry* extractLines( QgsGeometry* geometry );
    QgsGeometry* extractAsSingle( QgsGeometry* geometry );
    QgsGeometry* extractAsMulti( QgsGeometry* geometry );
    QgsGeometry* convertGeometry( QgsGeometry* geometry );
    QList<QgsPoint> extractPoints( QgsGeometry* geometry );
    /**Helper function to buffer an individual feature*/
    void bufferFeature( QgsFeature& f, int nProcessedFeatures, QgsVectorFileWriter* vfw, bool dissolve, QgsGeometry** dissolveGeometry, \
                        double bufferDistance, int bufferDistanceField );

};
#endif //QGSVECTORANALYZER
