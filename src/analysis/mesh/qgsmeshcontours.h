/***************************************************************************
                          qgsmeshcontours.h
                          -----------------
    begin                : September 25th, 2019
    copyright            : (C) 2018 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMESHCONTOURS_H
#define QGSMESHCONTOURS_H

#include <QMap>
#include <QString>
#include <QStringList>
#include <QVector>
#include <QPair>
#include <QList>
#include <memory>
#include <limits>

#include "qgsmeshdataprovider.h"
#include "qgsfeature.h"
#include "qgis_analysis.h"
#include "qgis_sip.h"
#include "qgstriangularmesh.h"
#include "qgsmeshrenderersettings.h"

class QgsMeshLayer;
class QgsPoint;
class QgsGeometry;
class QgsFeedback;

/**
 * \ingroup analysis
 * \class QgsMeshContours
 *
 * Exporter of contours lines or polygons from a mesh layer.
 *
 * \since QGIS 3.12
 */
class ANALYSIS_EXPORT QgsMeshContours
{
  public:

    /**
     * Constructs the mesh contours exporter.
     * Caches the native and triangular mesh from data provider
     * \param layer mesh layer to be associated with this exporter
     */
    QgsMeshContours( QgsMeshLayer *layer );
    ~QgsMeshContours();

    /**
     * Exports multi line string containing the contour line for particular dataset and value
     * \param index dataset index
     * \param value value of the contour line
     * \param method for datasets defined on faces, the method will be used to convert data to vertices
     * \param feedback optional feedback object for progress and cancellation support
     * \returns MultiLineString geometry containing contour lines
     */
    QgsGeometry exportLines( const QgsMeshDatasetIndex &index,
                             double value,
                             QgsMeshRendererScalarSettings::DataInterpolationMethod method,
                             QgsFeedback *feedback = nullptr );

    /**
     * Exports multi polygons representing the areas with values in range for particular dataset
     * \param index dataset index
     * \param min_value minimum of the value interval for contour polygon
     * \param max_value maximum of the value interval for contour polygon
     * \param method for datasets defined on faces, the method will be used to convert data to vertices
     * \param feedback optional feedback object for progress and cancellation support
     * \returns MultiPolygon geometry containing contour polygons
     */
    QgsGeometry exportPolygons( const QgsMeshDatasetIndex &index,
                                double min_value,
                                double max_value,
                                QgsMeshRendererScalarSettings::DataInterpolationMethod method,
                                QgsFeedback *feedback = nullptr );

  private:
    void populateCache(
      const QgsMeshDatasetIndex &index,
      QgsMeshRendererScalarSettings::DataInterpolationMethod method );

    QgsMeshLayer *mMeshLayer = nullptr;

    // cached values for particular index & interpolation method & layer
    QgsMeshDatasetIndex mCachedIndex;
    std::shared_ptr<QgsTriangularMesh> mTriangularMesh;
    std::shared_ptr<QgsMesh> mNativeMesh;
    QgsMeshDataBlock mScalarActiveFaceFlagValues;
    QVector<double> mDatasetValues;
};

#endif // QGSMESHCONTOURS_H
