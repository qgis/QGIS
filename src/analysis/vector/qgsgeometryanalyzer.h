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

#ifndef QGSGEOMETRYANALYZERH
#define QGSGEOMETRYANALYZERH

#include "qgsvectorlayer.h"
#include "qgsfield.h"
#include "qgsfeature.h"
#include "qgsgeometry.h"
#include "qgsdistancearea.h"

class QgsVectorFileWriter;
class QProgressDialog;


/** \ingroup analysis
 * The QGis class provides vector geometry analysis functions
 */

class ANALYSIS_EXPORT QgsGeometryAnalyzer
{
  public:

    /**Simplify vector layer using (a modified) Douglas-Peucker algorithm
     and write it to a new shape file
      @param layer input vector layer
      @param shapefileName path to the output shp
      @param tolerance (level of simplification)
      @param onlySelectedFeatures if true, only selected features are considered, else all the features
      @param p progress dialog (or 0 if no progress dialog is to be shown)
      */
    bool simplify( QgsVectorLayer* layer, const QString& shapefileName, double tolerance,
                   bool onlySelectedFeatures = false, QProgressDialog* p = 0 );

    /**Calculate the true centroids, or 'center of mass' for a vector layer and
       write it to a new shape file
      @param layer input vector layer
      @param shapefileName path to the output shp
      @param onlySelectedFeatures if true, only selected features are considered, else all the features
      @param p progress dialog (or 0 if no progress dialog is to be shown)
      */
    bool centroids( QgsVectorLayer* layer, const QString& shapefileName,
                    bool onlySelectedFeatures = false, QProgressDialog* p = 0 );

    /**Create a polygon based on the extent of all (selected) features and write it to a new shape file
      @param layer input vector layer
      @param shapefileName path to the output shp
      @param onlySelectedFeatures if true, only selected features are considered, else all the features
      @param p progress dialog (or 0 if no progress dialog is to be shown)
      */
    bool extent( QgsVectorLayer* layer, const QString& shapefileName, bool onlySelectedFeatures = false, QProgressDialog* p = 0 );

    /**Create buffers for a vector layer and write it to a new shape file
      @param layer input vector layer
      @param shapefileName path to the output shp
      @param bufferDistance distance for buffering (if no buffer field is specified)
      @param onlySelectedFeatures if true, only selected features are considered, else all the features
      @param dissolve if true, merge all the buffers to a big multipolygon
      @param bufferDistanceField index of the attribute field that contains the buffer distance (or -1 if all features have the same buffer distance)
      @param p progress dialog (or 0 if no progress dialog is to be shown)
      */
    bool buffer( QgsVectorLayer* layer, const QString& shapefileName, double bufferDistance,
                 bool onlySelectedFeatures = false, bool dissolve = false, int bufferDistanceField = -1, QProgressDialog* p = 0 );

    /**Create convex hull(s) of a vector layer and write it to a new shape file
      @param layer input vector layer
      @param shapefileName path to the output shp
      @param onlySelectedFeatures if true, only selected features are considered, else all the features
      @param uniqueIdField index of the attribute field that contains the unique convex hull id (or -1 if
      all features have the same buffer distance)
      @param p progress dialog (or 0 if no progress dialog is to be shown)
      */
    bool convexHull( QgsVectorLayer* layer, const QString& shapefileName, bool onlySelectedFeatures = false,
                     int uniqueIdField = -1, QProgressDialog* p = 0 );

    /**Dissolve a vector layer and write it to a new shape file
      @param layer input vector layer
      @param shapefileName path to the output shp
      @param onlySelectedFeatures if true, only selected features are considered, else all the features
      @param uniqueIdField index of the attribute field that contains the unique id to dissolve on (or -1 if
      all features should be dissolved together)
      @param p progress dialog (or 0 if no progress dialog is to be shown)
      */
    bool dissolve( QgsVectorLayer* layer, const QString& shapefileName, bool onlySelectedFeatures = false,
                   int uniqueIdField = -1, QProgressDialog* p = 0 );

    /**Creates an event layer (multipoint or multiline) by locating features from a (non-spatial) event table along the features of a line layer.
        Note that currently (until QgsGeometry supports m-values) the z-coordinate of the line layer is used for linear referencing
      @param lineLayer layer with the line geometry
      @param eventLayer layer with features and location field
      @param lineField join index in line layer
      @param eventField join index in event layer
      @param outputLayer name of output file (can be empty if a memory layer is used)
      @param outputFormat name of output format (can be empty if a memory provider is used to store the results)
      @param unlocatedFeatureIds out: ids of event features where linear referencing was not successful
      @param locationField1 attribute index of location field in event layer
      @param locationField2 attribute index of location end field (or -1 for point layer)
      @param offsetField attribute index for offset field. Negative offset value = offset to left side, positive value = offset to right side
      @param offsetScale factor to scale offset
      @param forceSingleGeometry force layer to single point/line type. Feature attributes are copied in case of multiple matches
      @param memoryProvider memory provider to write output to (can be 0 if output is written to a file)
      @param p progress dialog or 0 if no progress dialog should be shown
    */
    bool eventLayer( QgsVectorLayer* lineLayer, QgsVectorLayer* eventLayer, int lineField, int eventField, QgsFeatureIds &unlocatedFeatureIds, const QString& outputLayer,
                     const QString& outputFormat, int locationField1, int locationField2 = -1, int offsetField = -1, double offsetScale = 1.0,
                     bool forceSingleGeometry = false, QgsVectorDataProvider* memoryProvider = 0, QProgressDialog* p = 0 );

    /**Returns linear reference geometry as a multiline (or 0 if no match). Currently, the z-coordinates are considered to be the measures (no support for m-values in QGIS)*/
    QgsGeometry* locateBetweenMeasures( double fromMeasure, double toMeasure, QgsGeometry* lineGeom );
    /**Returns linear reference geometry. Unlike the PostGIS function, this method always returns multipoint or 0 if no match (not geometry collection).
      Currently, the z-coordinates are considered to be the measures (no support for m-values in QGIS)*/
    QgsGeometry* locateAlongMeasure( double measure, QgsGeometry* lineGeom );

  private:

    QList<double> simpleMeasure( QgsGeometry* geometry );
    double perimeterMeasure( QgsGeometry* geometry, QgsDistanceArea& measure );
    /**Helper function to simplify an individual feature*/
    void simplifyFeature( QgsFeature& f, QgsVectorFileWriter* vfw, double tolerance );
    /**Helper function to get the cetroid of an individual feature*/
    void centroidFeature( QgsFeature& f, QgsVectorFileWriter* vfw );
    /**Helper function to buffer an individual feature*/
    void bufferFeature( QgsFeature& f, int nProcessedFeatures, QgsVectorFileWriter* vfw, bool dissolve, QgsGeometry** dissolveGeometry,
                        double bufferDistance, int bufferDistanceField );
    /**Helper function to get the convex hull of feature(s)*/
    void convexFeature( QgsFeature& f, int nProcessedFeatures, QgsGeometry** dissolveGeometry );
    /**Helper function to dissolve feature(s)*/
    void dissolveFeature( QgsFeature& f, int nProcessedFeatures, QgsGeometry** dissolveGeometry );

    //helper functions for event layer
    void addEventLayerFeature( QgsFeature& feature, QgsGeometry* geom, QgsGeometry* lineGeom, QgsVectorFileWriter* fileWriter, QgsFeatureList& memoryFeatures, int offsetField = -1, double offsetScale = 1.0,
                               bool forceSingleType = false );
    /**Create geometry offset relative to line geometry.
        @param geom the geometry to modify
        @param lineGeom the line geometry to which the feature is referenced
        @param offset the offset value in layer unit. Negative values mean offset towards left, positive values offset to the right side*/
    bool createOffsetGeometry( QgsGeometry* geom, QgsGeometry* lineGeom, double offset );
    QgsPoint createPointOffset( double x, double y, double dist, QgsGeometry* lineGeom ) const;
    const unsigned char* locateBetweenWkbString( const unsigned char* ptr, QgsMultiPolyline& result, double fromMeasure, double toMeasure );
    const unsigned char* locateAlongWkbString( const unsigned char* ptr, QgsMultiPoint& result, double measure );
    static bool clipSegmentByRange( double x1, double y1, double m1, double x2, double y2, double m2, double range1, double range2, QgsPoint& pt1, QgsPoint& pt2, bool& secondPointClipped );
    static void locateAlongSegment( double x1, double y1, double m1, double x2, double y2, double m2, double measure, bool& pt1Ok, QgsPoint& pt1, bool& pt2Ok, QgsPoint& pt2 );
};
#endif //QGSVECTORANALYZER
