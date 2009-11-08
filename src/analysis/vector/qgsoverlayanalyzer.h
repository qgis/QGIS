/***************************************************************************
    qgsoverlayanalyzer.h - QGIS Tools for vector geometry analysis
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

#ifndef QGSOVERLAYANALYZERH
#define QGSOVERLAYANALYZERH

#include "qgsvectorlayer.h"
#include "qgsfield.h"
#include "qgsspatialindex.h"
#include "qgsfeature.h"
#include "qgsgeometry.h"
#include "qgsfield.h"
#include "qgsdistancearea.h"

class QgsVectorFileWriter;
class QProgressDialog;


/** \ingroup analysis
 * The QGis class provides vector overlay analysis functions
 */

class ANALYSIS_EXPORT QgsOverlayAnalyzer
{
  public:

    /**Perform an intersection on two input vector layers and write output to a new shape file
      @param layerA input vector layer
      @param layerB input vector layer
      @param shapefileName path to the output shp
      @param onlySelectedFeatures if true, only selected features are considered, else all the features
      @param p progress dialog (or 0 if no progress dialog is to be shown)
      @note: added in version 1.4*/
    bool intersection( QgsVectorLayer* layerA, QgsVectorLayer* layerB, \
                       const QString& shapefileName, bool onlySelectedFeatures = false, \
                       QProgressDialog* p = 0 );

//    /**Perform a union of two input vector layers and write output to a new shape file
//      @param layerA input vector layer
//      @param layerB input vector layer
//      @param shapefileName path to the output shp
//      @param onlySelectedFeatures if true, only selected features are considered, else all the features
//      @param p progress dialog (or 0 if no progress dialog is to be shown)
//      @note: added in version 1.4*/
//    bool combine( QgsVectorLayer* layerA, QgsVectorLayer* layerB,
//                       const QString& shapefileName, bool onlySelectedFeatures = false,
//                       QProgressDialog* p = 0 );
//
//    /**Clip a vector layer based on the boundary of another vector layer and
//       write output to a new shape file
//      @param layerA input vector layer
//      @param layerB input vector layer
//      @param shapefileName path to the output shp
//      @param onlySelectedFeatures if true, only selected features are considered, else all the features
//      @param p progress dialog (or 0 if no progress dialog is to be shown)
//      @note: added in version 1.4*/
//    bool clip( QgsVectorLayer* layerA, QgsVectorLayer* layerB,
//                       const QString& shapefileName, bool onlySelectedFeatures = false,
//                       QProgressDialog* p = 0 );
//
//    /**Difference a vector layer based on the geometries of another vector layer
//       and write the output to a new shape file
//      @param layerA input vector layer
//      @param layerB input vector layer
//      @param shapefileName path to the output shp
//      @param onlySelectedFeatures if true, only selected features are considered, else all the features
//      @param p progress dialog (or 0 if no progress dialog is to be shown)
//      @note: added in version 1.4*/
//    bool difference( QgsVectorLayer* layerA, QgsVectorLayer* layerB,
//                       const QString& shapefileName, bool onlySelectedFeatures = false,
//                       QProgressDialog* p = 0 );
//
//    /**Intersect two vector layers and write the geometries of each layer that
//       do not intersect with the other layer to a new shape file (Symmetrical difference)
//      @param layerA input vector layer
//      @param layerB input vector layer
//      @param shapefileName path to the output shp
//      @param onlySelectedFeatures if true, only selected features are considered, else all the features
//      @param p progress dialog (or 0 if no progress dialog is to be shown)
//      @note: added in version 1.4*/
//    bool symDifference( QgsVectorLayer* layerA, QgsVectorLayer* layerB,
//                       const QString& shapefileName, bool onlySelectedFeatures = false,
//                       QProgressDialog* p = 0 );

  private:

    void combineFieldLists( QgsFieldMap fieldListA, QgsFieldMap fieldListB );
    void intersectFeature( QgsFeature& f, QgsVectorFileWriter* vfw, QgsVectorLayer* dp, QgsSpatialIndex* index );
};
#endif //QGSVECTORANALYZER
