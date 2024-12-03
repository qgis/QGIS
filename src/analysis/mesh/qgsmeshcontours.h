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
 * \brief Exporter of contours lines or polygons from a mesh layer.
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

    /**
     * Constructs the mesh contours exporter directly with triangular mesh and dataset values to populate the cache with them.
     * The cache can't be changed.
     * This instance can be used in a thread safe way.
     * \param triangularMesh triangular mesh of the meshlayer
     * \param nativeMesh the native mesh of the mesh layer
     * \param datasetValues the dataset values to used for creating contours defined on vertices
     * \param scalarActiveFaceFlagValues active face flag values
     *
     * \note for data on faces, the values are averaged on vertices to define contours
     *
     * \since QGIS 3.18
     */
    QgsMeshContours( const QgsTriangularMesh &triangularMesh, const QgsMesh &nativeMesh, const QVector<double> &datasetValues, const QgsMeshDataBlock scalarActiveFaceFlagValues ) SIP_SKIP;


    ~QgsMeshContours();

    /**
     * Exports multi line string containing the contour line for particular dataset and value.
     * \param index dataset index that is used to update the cache
     * \param value value of the contour line
     * \param method for datasets defined on faces, the method will be used to convert data to vertices
     * \param feedback optional feedback object for progress and cancellation support
     * \returns MultiLineString geometry containing contour lines
     *
     * \note not thread safe
     */
    QgsGeometry exportLines( const QgsMeshDatasetIndex &index, double value, QgsMeshRendererScalarSettings::DataResamplingMethod method, QgsFeedback *feedback = nullptr );

    /**
     * Exports multi line string containing the contour line for particular value using the data stored in the cache
     * \param value value of the contour line
     * \param feedback optional feedback object for progress and cancellation support
     * \returns MultiLineString geometry containing contour lines
     *
     * \note thread safe
     *
     * \since QGIS 3.18
     */
    QgsGeometry exportLines( double value, QgsFeedback *feedback = nullptr ) SIP_SKIP;

    /**
     * Exports multi polygons representing the areas with values in range for particular dataset
     * \param index dataset index
     * \param min_value minimum of the value interval for contour polygon
     * \param max_value maximum of the value interval for contour polygon
     * \param method for datasets defined on faces, the method will be used to convert data to vertices
     * \param feedback optional feedback object for progress and cancellation support
     * \returns MultiPolygon geometry containing contour polygons
     *
     * \note not thread safe
     */
    QgsGeometry exportPolygons( const QgsMeshDatasetIndex &index, double min_value, double max_value, QgsMeshRendererScalarSettings::DataResamplingMethod method, QgsFeedback *feedback = nullptr );

    /**
     * Exports multi polygons representing the areas with values in range using the data stored in the cache
     * \param min_value minimum of the value interval for contour polygon
     * \param max_value maximum of the value interval for contour polygon
     * \param feedback optional feedback object for progress and cancellation support
     * \returns MultiPolygon geometry containing contour polygons
     *
     * \note thread safe
     *
     * \since QGIS 3.18
     */
    QgsGeometry exportPolygons( double min_value, double max_value, QgsFeedback *feedback = nullptr ) SIP_SKIP;


  private:
    void populateCache(
      const QgsMeshDatasetIndex &index,
      QgsMeshRendererScalarSettings::DataResamplingMethod method
    );

    QgsMeshLayer *mMeshLayer = nullptr;

    // cached values for particular index & interpolation method & layer
    QgsMeshDatasetIndex mCachedIndex;
    QgsTriangularMesh mTriangularMesh;
    QgsMesh mNativeMesh;
    QVector<double> mDatasetValues;
    QgsMeshDataBlock mScalarActiveFaceFlagValues;
};

#endif // QGSMESHCONTOURS_H
