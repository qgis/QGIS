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

class QgsCoordinateTransform;

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

    ~QgsPointCloudLayerExporter();

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
     * Sets the \a filename for the new layer.
     */
    void setFileName( const QString &filename ) { mFilename = filename; };

    /**
     * Gets the filename for the new layer.
     */
    QString fileName() const { return mFilename; };

    /**
     * Sets the \a name for the new layer.
     */
    void setLayerName( const QString &name ) { mName = name; };

    /**
     * Gets the name for the new layer.
     */
    QString layerName() const { return mFilename; };

    /**
     * Sets a filter extent for points to be exported.
     * Points that fall outside the extent will be skipped.
     */
    void setFilterExtent( const QgsRectangle extent ) { mExtent = extent; };

    /**
     * Gets the filter extent for points to be exported.
     */
    QgsRectangle filterExtent() const { return mExtent; };

    /**
     * Sets an inclusive range for Z values to be exported.
     * Points with Z values outside the range will be skipped.
     */
    void setZRange( const QgsDoubleRange zRange ) { mZRange = zRange; };

    /**
     * Gets the inclusive range for Z values to be exported.
     */
    QgsDoubleRange zRange() const { return mZRange; };

    /**
     * Sets a QgsFeedback object to allow cancellation / progress reporting.
     *
     * \note The \a feedback object must exist for the lifetime of the exporter.
     */
    void setFeedback( QgsFeedback *feedback ) { mFeedback = feedback; };

    /**
     * Gets a pointer to the QgsFeedback object used for cancellation / progress reporting, or nullptr if not set.
     */
    QgsFeedback *feedback() const { return mFeedback; };

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
     * Gets the list of point cloud attributes that will be exported.
     */
    QStringList attributes() const { return mRequestedAttributes; };

    /**
     * Sets the \a crs for the exported file, and the transfom \a context that will be used for
     * for reprojection if different from the point cloud layer's CRS.
     */
    void setCrs( const QgsCoordinateReferenceSystem &crs, const QgsCoordinateTransformContext &context = QgsCoordinateTransformContext() ) { mCrs = crs; mTransformContext = context; };

    /**
     * Gets the \a crs for the exported file.
     */
    QgsCoordinateReferenceSystem crs() const { return mCrs; };

    /**
     * Sets the \a format for the exported file. Possible values are OGR driver names and PDAL reader names.
     * If never called, the default format is GPKG for vector and COPC for pointcloud files.
     * \returns true if the \a format is supported, false otherwise.
     */
    bool setFormat( const QString &format );

    /**
     * Returns the format for the exported file or layer.
     */
    QString format() const { return mFormat; };

    /**
     * Sets the maximum number of points to be exported.
     */
    void setPointsLimit( qint64 limit ) { mPointsLimit = std::max< qint64 >( limit, 0 ); };

    /**
     * Gets the maximum number of points to be exported.
     */
    qint64 pointsLimit() { return mPointsLimit; };

    void doExport();

    QgsMapLayer *getLayer();

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

    QgsMapLayer *mOutputLayer = nullptr;



    class ExporterBase
    {
      public:
        ExporterBase();
        void run();
        virtual ~ExporterBase() = default;
      protected:
        virtual void handlePoint( double x, double y, double z, const QVariantMap &map ) = 0;
        virtual void handleNode() = 0;
        QgsPointCloudLayerExporter *mParent = nullptr;
        QgsCoordinateTransform *mCt = nullptr;
    };

    class ExporterMemory : public ExporterBase
    {
      public:
        ExporterMemory( QgsPointCloudLayerExporter *exp );
        ~ExporterMemory() override;
      private:
        void handlePoint( double x, double y, double z, const QVariantMap &map ) override;
        void handleNode() override;
        QgsFeatureList mFeatures;
    };

    class ExporterVector : public ExporterBase
    {
      public:
        ExporterVector( QgsPointCloudLayerExporter *exp );
        ~ExporterVector() override;
      private:
        void handlePoint( double x, double y, double z, const QVariantMap &map ) override;
        void handleNode() override;
        QgsFeatureList mFeatures;
    };

    class ExporterPdal : public ExporterBase
    {
      public:
        ExporterPdal( QgsPointCloudLayerExporter *exp );
        ~ExporterPdal() override;
      private:
        void handlePoint( double x, double y, double z, const QVariantMap &map ) override;
        void handleNode() override;
    };

};



/**
 * \class QgsPointCloudLayerExporterTask
 * \ingroup core
 * \brief QgsTask task which performs a QgsPointCloudLayerExporter layer export operation as a background
 * task. This can be used to export a point cloud layer out to a provider without blocking the
 * QGIS interface.
 * \since QGIS 3.28
 */
class CORE_EXPORT QgsPointCloudLayerExporterTask : public QgsTask
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsPointCloudLayerExporterTask. Takes ownership of \a exporter.
    */
    QgsPointCloudLayerExporterTask( QgsPointCloudLayerExporter *exporter );

    void cancel() override;

  signals:

    /**
     * Emitted when exporting the layer is successfully completed.
     */
    void exportComplete();

  protected:

    bool run() override;
    void finished( bool result ) override;

  private:
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
