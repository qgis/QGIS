/***************************************************************************
                          qgsvectorlayerexporter.h
                             -------------------
    begin                : Thu Aug 25 2011
    copyright            : (C) 2011 by Giuseppe Sucameli
    email                : brush.tyler at gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSVECTORLAYEREXPORTER_H
#define QGSVECTORLAYEREXPORTER_H

#include "qgis_core.h"
#include "qgis.h"
#include "qgsfeature.h"
#include "qgsfeaturesink.h"
#include "qgstaskmanager.h"
#include "qgsfeedback.h"
#include "qgsvectorlayer.h"

class QProgressDialog;
class QgsVectorDataProvider;
class QgsFields;

/**
 * \class QgsVectorLayerExporter
 * \ingroup core
 * A convenience class for exporting vector layers to a destination data provider.
 *
 * QgsVectorLayerExporter can be used in two ways:
 *
 * 1. Using a static call to QgsVectorLayerExporter::exportLayer(...) which exports the
 * entire layer to the destination provider.
 *
 * 2. Create an instance of the class and issue calls to addFeature(...)
 *
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsVectorLayerExporter : public QgsFeatureSink
{
  public:

    //! Error codes
    enum ExportError
    {
      NoError = 0, //!< No errors were encountered
      ErrCreateDataSource, //!< Could not create the destination data source
      ErrCreateLayer, //!< Could not create destination layer
      ErrAttributeTypeUnsupported, //!< Source layer has an attribute type which could not be handled by destination
      ErrAttributeCreationFailed, //!< Destination provider was unable to create an attribute
      ErrProjection, //!< An error occurred while reprojecting features to destination CRS
      ErrFeatureWriteFailed, //!< An error occurred while writing a feature to the destination
      ErrInvalidLayer, //!< Could not access newly created destination layer
      ErrInvalidProvider, //!< Could not find a matching provider key
      ErrProviderUnsupportedFeature, //!< Provider does not support creation of empty layers
      ErrConnectionFailed, //!< Could not connect to destination
      ErrUserCanceled, //!< User canceled the export
    };

    /**
     * Writes the contents of vector layer to a different datasource.
     * \param layer source layer
     * \param uri URI for destination data source
     * \param providerKey string key for destination data provider
     * \param destCRS destination CRS, or an invalid (default constructed) CRS if
     * not available
     * \param onlySelected set to true to export only selected features
     * \param errorMessage if non-null, will be set to any error messages
     * \param options optional provider dataset options
     * \param feedback optional feedback object to show progress and cancelation of export
     * \returns NoError for a successful export, or encountered error
     */
    static ExportError exportLayer( QgsVectorLayer *layer,
                                    const QString &uri,
                                    const QString &providerKey,
                                    const QgsCoordinateReferenceSystem &destCRS,
                                    bool onlySelected = false,
                                    QString *errorMessage SIP_OUT = nullptr,
                                    const QMap<QString, QVariant> &options = QMap<QString, QVariant>(),
                                    QgsFeedback *feedback = nullptr
                                  );

    /**
     * Constructor for QgsVectorLayerExporter.
     * \param uri URI for destination data source
     * \param provider string key for destination data provider
     * \param fields fields to include in created layer
     * \param geometryType destination geometry type
     * \param crs desired CRS, or an invalid (default constructed) CRS if
     * not available
     * \param overwrite set to true to overwrite any existing data source
     * \param options optional provider dataset options
     * \param sinkFlags for how to add features
     */
    QgsVectorLayerExporter( const QString &uri,
                            const QString &provider,
                            const QgsFields &fields,
                            QgsWkbTypes::Type geometryType,
                            const QgsCoordinateReferenceSystem &crs,
                            bool overwrite = false,
                            const QMap<QString, QVariant> &options = QMap<QString, QVariant>(),
                            QgsFeatureSink::SinkFlags sinkFlags = nullptr );

    //! QgsVectorLayerExporter cannot be copied
    QgsVectorLayerExporter( const QgsVectorLayerExporter &rh ) = delete;
    //! QgsVectorLayerExporter cannot be copied
    QgsVectorLayerExporter &operator=( const QgsVectorLayerExporter &rh ) = delete;

    /**
     * Returns any encountered error code, or false if no error was encountered.
     * \see errorMessage()
     * \see errorCount()
     */
    ExportError errorCode() const;

    /**
     * Returns any error message encountered during the export.
     * \see errorCount()
     * \see errorCode()
     */
    QString errorMessage() const;

    /**
     * Returns the number of error messages encountered during the export.
     * \see errorMessage()
     * \see errorCode()
     */
    int errorCount() const { return mErrorCount; }

    bool addFeatures( QgsFeatureList &features, QgsFeatureSink::Flags flags = nullptr ) override;
    bool addFeature( QgsFeature &feature, QgsFeatureSink::Flags flags = nullptr ) override;

    /**
     * Finalizes the export and closes the new created layer.
     */
    ~QgsVectorLayerExporter() override;

    bool flushBuffer() override;

  private:

    //! Create index
    bool createSpatialIndex();

    //! Contains error value
    ExportError mError;
    QString mErrorMessage;

    int mErrorCount;

    QgsVectorDataProvider *mProvider = nullptr;

    //! Map attribute indexes to new field indexes
    QMap<int, int> mOldToNewAttrIdx;
    int mAttributeCount;

    QgsFeatureList mFeatureBuffer;

#ifdef SIP_RUN
    QgsVectorLayerExporter( const QgsVectorLayerExporter &rh );
#endif

};


/**
 * \class QgsVectorLayerExporterTask
 * \ingroup core
 * QgsTask task which performs a QgsVectorLayerExporter layer export operation as a background
 * task. This can be used to export a vector layer out to a provider without blocking the
 * QGIS interface.
 * \see QgsVectorFileWriterTask
 * \see QgsRasterFileWriterTask
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsVectorLayerExporterTask : public QgsTask
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsVectorLayerExporterTask. Takes a source \a layer, destination \a uri
     * and \a providerKey, and various export related parameters such as destination CRS
     * and export \a options. \a ownsLayer has to be set to true if the task should take ownership
     * of the layer and delete it after export.
    */
    QgsVectorLayerExporterTask( QgsVectorLayer *layer,
                                const QString &uri,
                                const QString &providerKey,
                                const QgsCoordinateReferenceSystem &destinationCrs,
                                const QMap<QString, QVariant> &options = QMap<QString, QVariant>(),
                                bool ownsLayer = false );

    /**
     * Creates a new QgsVectorLayerExporterTask which has ownership over a source \a layer.
     * When the export task has completed (successfully or otherwise) \a layer will be
     * deleted. The destination \a uri and \a providerKey, and various export related parameters such as destination CRS
     * and export \a options must be specified.
    */
    static QgsVectorLayerExporterTask *withLayerOwnership( QgsVectorLayer *layer SIP_TRANSFER,
        const QString &uri,
        const QString &providerKey,
        const QgsCoordinateReferenceSystem &destinationCrs,
        const QMap<QString, QVariant> &options = QMap<QString, QVariant>() ) SIP_FACTORY;

    void cancel() override;

  signals:

    /**
     * Emitted when exporting the layer is successfully completed.
     */
    void exportComplete();

    /**
     * Emitted when an error occurs which prevented the layer being exported (or if
     * the task is canceled). The export \a error and \a errorMessage will be reported.
     */
    void errorOccurred( int error, const QString &errorMessage );

  protected:

    bool run() override;
    void finished( bool result ) override;

  private:

    QPointer< QgsVectorLayer > mLayer = nullptr;
    bool mOwnsLayer = false;

    QString mDestUri;
    QString mDestProviderKey;
    QgsCoordinateReferenceSystem mDestCrs;
    QMap<QString, QVariant> mOptions;

    std::unique_ptr< QgsFeedback > mOwnedFeedback;

    QgsVectorLayerExporter::ExportError mError = QgsVectorLayerExporter::NoError;
    QString mErrorMessage;

};

#endif // QGSVECTORLAYEREXPORTER_H
