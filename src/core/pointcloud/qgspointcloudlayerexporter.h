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
#include "qgstaskmanager.h"

/**
 * \class QgsPointCloudLayerExporter
 * \ingroup core
 *
 * \brief Handles exporting point cloud layers to memory layers, OGR supported files and PDAL supported files.
 *
 * \since QGIS 3.28
 */
class CORE_EXPORT QgsPointCloudLayerExporter SIP_NODEFAULTCTORS
{
  public:

    /**
     * Constructor for QgsPointCloudLayerExporter, associated with the specified \a layer.
     *
     * \note The \a layer is safe to be deleted once it's used in the constructor.
     */
    QgsPointCloudLayerExporter( QgsPointCloudLayer *layer );

    /**
     * Exports the point cloud layer to a memory layer.
     *
     * Caller takes ownership of the returned object.
     */
    QgsVectorLayer *exportToMemoryLayer() SIP_FACTORY;

    /**
     * Exports the point cloud layer to a OGR vector file.
     */
    QgsVectorLayer *exportToVectorFile( const QString &filename = QString() ) SIP_FACTORY;

    /**
     * Exports the point cloud layer to a point cloud file, where the file type can be any format supported by PDAL.
     */
    QgsPointCloudLayer *exportToPdalFile( const QString &filename = QString() ) SIP_FACTORY;

    /**
     * Sets the name for the new layer.
     */
    void setFileName( const QString &filename ) { mFilename = filename; };

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
     *
     * \note The \a feedback object must exist for the lifetime of the exporter.
     */
    void setFeedback( QgsFeedback *feedback ) { mFeedback = feedback; };

    /**
     * Sets the list of point cloud \a attributes that will be exported.
     * If never called, all attributes will be exported.
     */
    void setAttributes( const QStringList &attributes );

    /**
     * Sets that no attributes will be exported.
     */
    void setNoAttributes() { mRequestedAttributes.clear(); };

    /**
     * Sets the \a crs for the exported file, and the transfom \a context that will be used for
     * for reprojection if different from the point cloud layer's CRS.
     */
    void setCrs( const QgsCoordinateReferenceSystem &crs, const QgsCoordinateTransformContext &context = QgsCoordinateTransformContext() ) { mCrs = crs; mTransformContext = context; };

    /**
     * Sets the format for the exported file.
     * If never called, the default format is GPKG for vector and COPC for pointcloud files.
     */
    bool setFormat( const QString &format );

    /**
     * Returns the format for the exported file.
     */
    QString format() const { return mFormat; };

    /**
     * Sets the maximum number of points to be exported.
     */
    void setPointsLimit( qint64 limit ) { mPointsLimit = std::max< qint64 >( limit, 0 ); };

  private:

    void exportToSink( QgsFeatureSink * );

    QgsFields outputFields();
    const QgsPointCloudAttributeCollection requestedAttributeCollection();

    const QgsPointCloudAttributeCollection mLayerAttributeCollection;

    QgsPointCloudIndex *mIndex = nullptr;
    QString mName;
    QString mFormat;
    QString mFilename;
    QgsRectangle mExtent = QgsRectangle( std::numeric_limits< double >::lowest(),
                                         std::numeric_limits< double >::lowest(),
                                         std::numeric_limits< double >::max(),
                                         std::numeric_limits< double >::max() );
    QgsDoubleRange mZRange;
    QgsFeedback *mFeedback = nullptr;
    qint64 mPointsLimit = std::numeric_limits<qint64>::max();
    QStringList mRequestedAttributes;
    QgsCoordinateReferenceSystem mCrs;
    QgsCoordinateTransformContext mTransformContext;
};



/**
 * \class QgsPointCloudLayerExporterTask
 * \ingroup core
 * \brief QgsTask task which performs a QgsPointCloudLayerExporter layer export operation as a background
 * task. This can be used to export a point cloud layer out to a provider without blocking the
 * QGIS interface.
 * \see QgsVectorFileWriterTask
 * \see QgsRasterFileWriterTask
 * \since QGIS 3.28
 */
class CORE_EXPORT QgsPointCloudLayerExporterTask : public QgsTask
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsPointCloudLayerExporterTask. Takes a source \a layer, destination \a uri
     * and \a providerKey, and various export related parameters such as destination CRS
     * and export \a options. \a ownsLayer has to be set to TRUE if the task should take ownership
     * of the layer and delete it after export.
    */
    QgsPointCloudLayerExporterTask( QgsPointCloudLayerExporter *exporter );

    void cancel() override;

  signals:

    /**
     * Emitted when exporting the layer is successfully completed.
     */
    void exportComplete( QgsMapLayer * );

    /**
     * Emitted when an error occurs which prevented the layer being exported (or if
     * the task is canceled). The export \a error and \a errorMessage will be reported.
     */
    void errorOccurred( Qgis::VectorExportResult error, const QString &errorMessage );

  protected:

    bool run() override;
    void finished( bool result ) override;

  private:
    QgsMapLayer *mOutputLayer = nullptr;
    QgsPointCloudLayerExporter *mExp = nullptr;
    const QString mFormat;

    QString mDestUri;
    QString mDestProviderKey;
    QgsCoordinateReferenceSystem mDestCrs;
    QMap<QString, QVariant> mOptions;

    std::unique_ptr< QgsFeedback > mOwnedFeedback;

    Qgis::VectorExportResult mError = Qgis::VectorExportResult::Success;
    QString mErrorMessage;

};
#endif // QGSPOINTCLOUDLAYEREXPORTER_H
