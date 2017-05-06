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

class QProgressDialog;
class QgsVectorDataProvider;
class QgsVectorLayer;
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
     * \param skipAttributeCreation set to true to skip exporting feature attributes
     * \param options optional provider dataset options
     * \param progress optional progress dialog to show progress of export
     * \returns NoError for a successful export, or encountered error
     */
    static ExportError exportLayer( QgsVectorLayer *layer,
                                    const QString &uri,
                                    const QString &providerKey,
                                    const QgsCoordinateReferenceSystem &destCRS,
                                    bool onlySelected = false,
                                    QString *errorMessage SIP_OUT = 0,
                                    bool skipAttributeCreation = false,
                                    QMap<QString, QVariant> *options = nullptr,
                                    QProgressDialog *progress = nullptr
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
     */
    QgsVectorLayerExporter( const QString &uri,
                            const QString &provider,
                            const QgsFields &fields,
                            QgsWkbTypes::Type geometryType,
                            const QgsCoordinateReferenceSystem &crs,
                            bool overwrite = false,
                            const QMap<QString, QVariant> *options = nullptr );

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

    bool addFeatures( QgsFeatureList &features ) override;
    bool addFeature( QgsFeature &feature ) override;

    /**
     * Finalizes the export and closes the new created layer.
     */
    ~QgsVectorLayerExporter();

  private:
    //! Flush the buffer writing the features to the new layer
    bool flushBuffer();

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

#endif // QGSVECTORLAYEREXPORTER_H
