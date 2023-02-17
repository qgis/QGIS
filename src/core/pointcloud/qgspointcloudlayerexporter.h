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
#include "qgsvectorfilewriter.h"

#ifdef HAVE_PDAL_QGIS
#include <pdal/PointView.hpp>
#include <pdal/PointTable.hpp>
#include <pdal/Options.hpp>
#endif

class QgsCoordinateTransform;
class QgsGeos;

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
     * Supported export formats for point clouds
     */
    enum class ExportFormat : int
    {
      Memory = 0, //!< Memory layer
      Las = 1, //!< LAS/LAZ point cloud
      Gpkg = 2, //!< Geopackage
      Shp = 3, //!< ESRI ShapeFile
      Dxf = 4, //!< AutoCAD dxf
      Csv = 5, //!< Comma separated values
    };

    /**
     * Gets a list of the supported export formats.
     *
     * \see setFormat()
     */
    static QList< ExportFormat > supportedFormats() SIP_SKIP
    {
      QList< ExportFormat > formats;
      formats << ExportFormat::Memory
#ifdef HAVE_PDAL_QGIS
              << ExportFormat::Las
#endif
              << ExportFormat::Gpkg
              << ExportFormat::Shp
              << ExportFormat::Dxf
              << ExportFormat::Csv;
      return formats;
    }

    /**
     * Gets the OGR driver name for the specified \a format
     * \note Not available in python bindings
     */
    static QString getOgrDriverName( ExportFormat format ) SIP_SKIP;

    /**
     * Constructor for QgsPointCloudLayerExporter, associated with the specified \a layer.
     *
     * \note The \a layer is safe to be deleted once it's used in the constructor.
     */
    QgsPointCloudLayerExporter( QgsPointCloudLayer *layer );

    ~QgsPointCloudLayerExporter();

    /**
     * Sets the \a filename for the new layer.
     */
    void setFileName( const QString &filename ) { mFilename = filename; }

    /**
     * Gets the filename for the new layer.
     */
    QString fileName() const { return mFilename; }

    /**
     * Sets the \a name for the new layer.
     */
    void setLayerName( const QString &name ) { mName = name; }

    /**
     * Gets the name for the new layer.
     */
    QString layerName() const { return mName; }

    /**
     * Sets a filter extent for points to be exported in the target CRS
     * Points that fall outside the extent will be skipped.
     * \see setCrs()
     */
    void setFilterExtent( const QgsRectangle extent ) { mExtent = extent; }

    /**
     * Gets the filter extent for points to be exported.
     */
    QgsRectangle filterExtent() const { return mExtent; }

    /**
     * Sets a spatial filter for points to be exported based on \a geom in the point cloud's CRS.
     * Points that do not intersect \a geometry will be skipped.
     */
    void setFilterGeometry( const QgsAbstractGeometry *geometry );

    /**
     * Sets a spatial filter for points to be exported based on the features of \a layer.
     * Points that do not intersect the \a layer's features will be skipped.
     */
    void setFilterGeometry( QgsMapLayer *layer, bool selectedFeaturesOnly = false );

    /**
     * Sets an inclusive range for Z values to be exported.
     * Points with Z values outside the range will be skipped.
     */
    void setZRange( const QgsDoubleRange zRange ) { mZRange = zRange; }

    /**
     * Gets the inclusive range for Z values to be exported.
     */
    QgsDoubleRange zRange() const { return mZRange; }

    /**
     * Sets a QgsFeedback object to allow cancellation / progress reporting.
     *
     * \note The \a feedback object must exist for the lifetime of the exporter.
     */
    void setFeedback( QgsFeedback *feedback ) { mFeedback = feedback; }

    /**
     * Gets a pointer to the QgsFeedback object used for cancellation / progress reporting, or nullptr if not set.
     */
    QgsFeedback *feedback() const { return mFeedback; }

    /**
     * Sets the list of point cloud \a attributes that will be exported.
     * If never called, all attributes will be exported.
     * \note This has no effect when exporting to LAS/LAZ format.
     */
    void setAttributes( const QStringList &attributes );

    /**
     * Sets that no attributes will be exported.
     * \note This has no effect when exporting to LAS/LAZ format.
     */
    void setNoAttributes() { mRequestedAttributes.clear(); }

    /**
     * Sets that all attributes will be exported.
     */
    void setAllAttributes();

    /**
     * Gets the list of point cloud attributes that will be exported.
     */
    QStringList attributes() const { return mRequestedAttributes; }

    /**
     * Sets the \a crs for the exported file, and the transform \a context that will be used
     * for reprojection if different from the point cloud layer's CRS.
     */
    void setCrs( const QgsCoordinateReferenceSystem &crs, const QgsCoordinateTransformContext &context = QgsCoordinateTransformContext() ) { mTargetCrs = crs; mTransformContext = context; }

    /**
     * Gets the \a crs for the exported file.
     */
    QgsCoordinateReferenceSystem crs() const { return mTargetCrs; }

    /**
     * Sets the \a format for the exported file.
     * \returns true if the \a format is supported, false otherwise.
     * \see ExportFormat
     */
    bool setFormat( const ExportFormat format );

    /**
     * Returns the format for the exported file or layer.
     */
    ExportFormat format() const { return mFormat; }

    /**
     * Sets the maximum number of points to be exported. Default value is 0.
     * \note Any \a limit value <= 0 means no limit.
     */
    void setPointsLimit( qint64 limit ) { mPointsLimit = std::max< qint64 >( limit, 0 ); }

    /**
     * Gets the maximum number of points to be exported. 0 means no limit.
     */
    qint64 pointsLimit() { return mPointsLimit; }

    /**
     * Sets whether an existing output vector file should be overwritten on appended to.
     * \note Only applies to vector formats
     */
    void setActionOnExistingFile( const QgsVectorFileWriter::ActionOnExistingFile action ) { mActionOnExistingFile = action; }

    /**
     * Creates the QgsVectorLayer for exporting to a memory layer, if necessary.
     * This method allows the exported memory layer to be created in the main thread. If not explicitly called, this method
     * will be implicitly called by doExport().
     */
    void prepareExport();

    /**
     * Performs the actual exporting operation.
     */
    void doExport();

    /**
     * Gets a pointer to the exported layer.
     * Caller takes ownership of the returned object.
     */
    QgsMapLayer *takeExportedLayer() SIP_FACTORY;

    /**
     * Gets the last error that occurred during the exporting operation.
     * If no error occurred an empty string is returned.
     */
    QString lastError() const { return mLastError; }

  private:

    void exportToSink( QgsFeatureSink * );

    QgsFields outputFields();
    const QgsPointCloudAttributeCollection requestedAttributeCollection();
    void setLastError( QString error ) { mLastError = error; }

    const QgsPointCloudAttributeCollection mLayerAttributeCollection;

    QgsPointCloudIndex *mIndex = nullptr;
    QString mName = QObject::tr( "Exported" );
    ExportFormat mFormat = ExportFormat::Memory;
    QString mFilename;
    QString mLastError;
    QgsRectangle mExtent = QgsRectangle( -std::numeric_limits<double>::infinity(),
                                         -std::numeric_limits<double>::infinity(),
                                         std::numeric_limits<double>::infinity(),
                                         std::numeric_limits<double>::infinity(),
                                         false );
    std::unique_ptr< QgsGeos > mFilterGeometryEngine;
    QgsDoubleRange mZRange;
    QgsFeedback *mFeedback = nullptr;
    qint64 mPointsLimit = 0;
    QStringList mRequestedAttributes;
    QgsCoordinateReferenceSystem mSourceCrs;
    QgsCoordinateReferenceSystem mTargetCrs;
    QgsCoordinateTransformContext mTransformContext;
    int mPointRecordFormat;
    QgsVectorFileWriter::ActionOnExistingFile mActionOnExistingFile = QgsVectorFileWriter::ActionOnExistingFile::CreateOrOverwriteFile;

    QgsMapLayer *mMemoryLayer = nullptr;
    QgsFeatureSink *mVectorSink = nullptr;
    QgsCoordinateTransform *mTransform = nullptr;


    class ExporterBase
    {
      public:
        ExporterBase() = default;
        virtual ~ExporterBase() = default;
        void run();
      protected:
        virtual void handlePoint( double x, double y, double z, const QVariantMap &map, const qint64 pointNumber ) = 0;
        virtual void handleNode() = 0;
        virtual void handleAll() = 0;
        QgsPointCloudLayerExporter *mParent = nullptr;
    };

    class ExporterMemory : public ExporterBase
    {
      public:
        ExporterMemory( QgsPointCloudLayerExporter *exp );
        ~ExporterMemory() override;

      private:
        void handlePoint( double x, double y, double z, const QVariantMap &map, const qint64 pointNumber ) override;
        void handleNode() override;
        void handleAll() override;
        QgsFeatureList mFeatures;
    };

    class ExporterVector : public ExporterBase
    {
      public:
        ExporterVector( QgsPointCloudLayerExporter *exp );
        ~ExporterVector() override;

      private:
        void handlePoint( double x, double y, double z, const QVariantMap &map, const qint64 pointNumber ) override;
        void handleNode() override;
        void handleAll() override;
        QgsFeatureList mFeatures;
    };

#ifdef HAVE_PDAL_QGIS
    class ExporterPdal : public ExporterBase
    {
      public:
        ExporterPdal( QgsPointCloudLayerExporter *exp );

      private:
        void handlePoint( double x, double y, double z, const QVariantMap &map, const qint64 pointNumber ) override;
        void handleNode() override;
        void handleAll() override;
        const int mPointFormat;
        pdal::Options mOptions;
        pdal::PointTable mTable;
        pdal::PointViewPtr mView;
    };
#endif

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

    std::unique_ptr< QgsFeedback > mOwnedFeedback;

};
#endif // QGSPOINTCLOUDLAYEREXPORTER_H
