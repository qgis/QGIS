/***************************************************************************
                         qgspointcloudlayerexporter.h
                         ---------------------
    begin                : July 2022
    copyright            : (C) 2022 by Stefanos Natsis
    email                : uclaros at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPOINTCLOUDLAYEREXPORTER_H
#define QGSPOINTCLOUDLAYEREXPORTER_H

#include "qgis_core.h"
#include "qgsvectorlayer.h"
#include "qgspointcloudlayer.h"

/**
 * \class QgsPointCloudLayerExporter
 * \ingroup core
 *
 * \brief Handles exporting PointCloud layers to Temporary scratch layers, OGR supported files and PDAL supported files
 *
 * \since QGIS 3.28
 */
class CORE_EXPORT QgsPointCloudLayerExporter
{
  public:

    /**
     * Constructor for QgsPointCloudLayerExporter, associated with the specified \a layer.
     */
    QgsPointCloudLayerExporter( QgsPointCloudLayer *layer );

    /**
     * Exports the pointcloud layer to a Temporary Scratch Layer (memory layer).
     *
     * Caller takes ownership of the returned object.
     */
    QgsVectorLayer *exportToMemoryLayer() SIP_FACTORY;

    /**
     * Exports the pointcloud layer to a OGR vector file.
     */
    void exportToVectorFile( const QString &filename );

    /**
     * Exports the pointcloud layer to a PDAL pointcloud file.
     */
    void exportToPdalFile( const QString &filename );

    /**
     * Sets the name for the new layer.
     */
    void setLayerName( const QString &name ) { mName = name; };

    /**
     * Sets a filter extent for points to be exported.
     * Points that fall outside the extent will be skipped.
     */
    void setFilterExtent( const QgsRectangle extent ) { mExtent = extent; };

    /**
     * Sets an inclusive range for Z values to be exported.
     * Points with Z values outside the range will be skipped.
     */
    void setZRange( const QgsDoubleRange zRange ) { mZRange = zRange; };

    /**
     * Sets a QgsFeedback object to allow cancellation / progress reporting.
     */
    void setFeedback( QgsFeedback *feedback ) { mFeedback = feedback; };

    /**
     * Sets the list of pointcloud attributes that will be exported.
     * If never called, all attributes will be exported.
     */
    void setAttributes( const QStringList &attributes );

    /**
     * Sets the CRS for the exported file.
     * If different from the pointcloud layer's CRS, points will be reprojected.
     */
    void setCrs( const QgsCoordinateReferenceSystem &crs ) { mCrs = crs; };

    /**
     * Sets the format for the exported file.
     * If never called, the default format is GPKG for vector and COPC for pointcloud files.
     */
    bool setFormat( const QString &format );

  private:

    void exportToSink( QgsFeatureSink * );

    QgsFields outputFields();
    const QgsPointCloudAttributeCollection requestedAttributeCollection();

    QgsPointCloudLayer *mLayer;
    QString mName;
    QString mFormat;
    QgsRectangle mExtent = QgsRectangle( std::numeric_limits< double >::lowest(),
                                         std::numeric_limits< double >::lowest(),
                                         std::numeric_limits< double >::max(),
                                         std::numeric_limits< double >::max() );
    QgsDoubleRange mZRange;
    QgsFeedback *mFeedback = nullptr;
    qint64 mPointsLimit = std::numeric_limits<qint64>::max();
    QStringList mRequestedAttributes;
    QgsCoordinateReferenceSystem mCrs;
};

#endif // QGSPOINTCLOUDLAYEREXPORTER_H
