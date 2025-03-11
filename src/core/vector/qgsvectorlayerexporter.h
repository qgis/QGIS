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
#include "qgis_sip.h"
#include "qgis.h"
#include "qgsfeature.h"
#include "qgsfeaturesink.h"
#include "qgstaskmanager.h"
#include "qgsfeedback.h"
#include "qgsvectorlayer.h"
#include "qgsreferencedgeometry.h"

#include <QPointer>

class QProgressDialog;
class QgsVectorDataProvider;
class QgsFields;

/**
 * \class QgsVectorLayerExporter
 * \ingroup core
 * \brief A convenience class for exporting vector layers to a destination data provider.
 *
 * QgsVectorLayerExporter can be used in two ways:
 *
 * - Using a static call to QgsVectorLayerExporter::exportLayer(...) which exports the
 *   entire layer to the destination provider.
 * - Create an instance of the class and issue calls to addFeature(...)
 *
 */
class CORE_EXPORT QgsVectorLayerExporter : public QgsFeatureSink
{
  public:


    /**
     * Writes the contents of vector layer to a different datasource.
     * \param layer source layer
     * \param uri URI for destination data source
     * \param providerKey string key for destination data provider
     * \param destCRS destination CRS, or an invalid (default constructed) CRS if
     * not available
     * \param onlySelected set to TRUE to export only selected features
     * \param errorMessage if non-null, will be set to any error messages
     * \param options optional provider dataset options
     * \param feedback optional feedback object to show progress and cancellation of export
     * \returns NoError for a successful export, or encountered error
     */
    static Qgis::VectorExportResult exportLayer( QgsVectorLayer *layer,
        const QString &uri,
        const QString &providerKey,
        const QgsCoordinateReferenceSystem &destCRS,
        bool onlySelected = false,
        QString *errorMessage SIP_OUT = nullptr,
        const QMap<QString, QVariant> &options = QMap<QString, QVariant>(),
        QgsFeedback *feedback = nullptr );


    /**
     * \ingroup core
     * \brief Encapsulates output field definition.
     *
     * \since QGIS 3.44
     */
    struct CORE_EXPORT OutputField
    {

      /**
       * Constructor for OutputField, with the specified \a field definition and source \a expression.
       */
      OutputField( const QgsField &field, const QString &expression )
        : field( field )
        , expression( expression )
      {}

      //! Destination field definition
      QgsField field;
      //! The expression for the exported field from the source fields
      QString expression;
    };

    /**
     * \ingroup core
     * \brief Encapsulates options for use with QgsVectorLayerExporter.
     *
     * \since QGIS 3.44
     */
    class CORE_EXPORT ExportOptions
    {
      public:

        /**
         * Sets whether the export should only include selected features.
         *
         * \warning This setting is incompatible with setFilterExpression()
         *
         * \note This option only applies when the QgsVectorLayerExporter::exportLayer() method is used.
         *
         * \see selectedOnly()
         */
        void setSelectedOnly( bool selected ) { mSelectedOnly = selected; }

        /**
         * Returns whether the export will only include selected features.
         *
         * \warning This setting is incompatible with filterExpression()
         *
         * \note This option only applies when the QgsVectorLayerExporter::exportLayer() method is used.
         *
         * \see setSelectedOnly()
         */
        bool selectedOnly() const { return mSelectedOnly; }

        /**
         * Sets the coordinate transform \a context to use when transforming exported features.
         *
         * \see transformContext()
         */
        void setTransformContext( const QgsCoordinateTransformContext &context );

        /**
         * Returns the coordinate transform context used when transforming exported features.
         *
         * \see setTransformContext()
         */
        QgsCoordinateTransformContext transformContext() const;

        /**
         * Sets the destination coordinate reference system to use for exported features.
         *
         * \see destinationCrs()
         */
        void setDestinationCrs( const QgsCoordinateReferenceSystem &crs );

        /**
         * Returns the destination coordinate reference system used for exported features.
         *
         * \see setDestinationCrs()
         */
        QgsCoordinateReferenceSystem destinationCrs() const;

        /**
         * Sets an \a extent filter for the features to export.
         *
         * Only features with a bounding box intersecting \a extent will be exported.
         *
         * \note This option only applies when the QgsVectorLayerExporter::exportLayer() method is used.
         *
         * \see extent()
         */
        void setExtent( const QgsReferencedRectangle &extent );

        /**
         * Returns the extent filter for the features to export.
         *
         * Only features with a bounding box intersecting this extent will be exported.
         *
         * \note This option only applies when the QgsVectorLayerExporter::exportLayer() method is used.
         *
         * \see setExtent()
         */
        QgsReferencedRectangle extent() const;

        /**
         * Set the filter \a expression for the features to export.
         *
         * Only features matching this expression will be exported.
         *
         * \warning This setting is incompatible with setSelectedOnly()
         *
         * \note This option only applies when the QgsVectorLayerExporter::exportLayer() method is used.
         *
         * \see setExpressionContext()
         * \see filterExpression()
         */
        void setFilterExpression( const QString &expression );

        /**
         * Returns the filter expression for features to export.
         *
         * Only features matching this expression will be exported.
         *
         * \warning This setting is incompatible with selectedOnly()
         *
         * \note This option only applies when the QgsVectorLayerExporter::exportLayer() method is used.
         *
         * \see expressionContext()
         * \see setFilterExpression()
         */
        QString filterExpression() const;

        /**
         * Sets the expression \a context to use when a filterExpression() is set.
         *
         * \see expressionContext()
         */
        void setExpressionContext( const QgsExpressionContext &context );

        /**
         * Returns the expression context used when a filterExpression() is set.
         *
         * \see setExpressionContext()
         */
        const QgsExpressionContext &expressionContext() const;

        /**
         * Returns the output field definitions for the destination table.
         *
         * If empty, all input fields will be copied directly.
         *
         * \note This option only applies when the QgsVectorLayerExporter::exportLayer() method is used.
         *
         * \see setOutputFields()
         */
        QList<QgsVectorLayerExporter::OutputField> outputFields() const;

        /**
         * Sets the output field definitions for the destination table.
         *
         * If empty, all input fields will be copied directly.
         *
         * \note This option only applies when the QgsVectorLayerExporter::exportLayer() method is used.
         *
         * \see outputFields()
         */
        void setOutputFields( const QList<QgsVectorLayerExporter::OutputField> &fields );

      private:

        bool mSelectedOnly = false;

        QgsCoordinateReferenceSystem mDestinationCrs;

        QgsCoordinateTransformContext mTransformContext;

        QgsReferencedRectangle mExtent;

        QString mFilterExpression;
        QgsExpressionContext mExpressionContext;

        QList< QgsVectorLayerExporter::OutputField > mOutputFields;

    };

    /**
     * Writes the contents of vector layer to a different data provider.
     *
     * \param layer source layer
     * \param uri URI for destination data source
     * \param providerKey string key for destination data provider
     * \param exportOptions options controlling the feature export
     * \param errorMessage will be set to any error messages which occur during the export
     * \param providerOptions optional provider dataset options
     * \param feedback optional feedback object to show progress and cancellation of export
     * \returns Qgis::VectorExportResult::NoError for a successful export, or encountered error
     *
     * \since QGIS 3.44
     */
    static Qgis::VectorExportResult exportLayer( QgsVectorLayer *layer,
        const QString &uri,
        const QString &providerKey,
        const QgsVectorLayerExporter::ExportOptions &exportOptions,
        QString *errorMessage SIP_OUT = nullptr,
        const QMap<QString, QVariant> &providerOptions = QMap<QString, QVariant>(),
        QgsFeedback *feedback = nullptr );

    /**
     * Constructor for QgsVectorLayerExporter.
     * \param uri URI for destination data source
     * \param provider string key for destination data provider
     * \param fields fields to include in created layer
     * \param geometryType destination geometry type
     * \param crs desired CRS, or an invalid (default constructed) CRS if
     * not available
     * \param overwrite set to TRUE to overwrite any existing data source
     * \param options optional provider dataset options
     * \param sinkFlags for how to add features
     */
    QgsVectorLayerExporter( const QString &uri,
                            const QString &provider,
                            const QgsFields &fields,
                            Qgis::WkbType geometryType,
                            const QgsCoordinateReferenceSystem &crs,
                            bool overwrite = false,
                            const QMap<QString, QVariant> &options = QMap<QString, QVariant>(),
                            QgsFeatureSink::SinkFlags sinkFlags = QgsFeatureSink::SinkFlags() );

    QgsVectorLayerExporter( const QgsVectorLayerExporter &rh ) = delete;
    QgsVectorLayerExporter &operator=( const QgsVectorLayerExporter &rh ) = delete;

    /**
     * Returns any encountered error code, or FALSE if no error was encountered.
     * \see errorMessage()
     * \see errorCount()
     */
    Qgis::VectorExportResult errorCode() const;

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

    /**
     * Returns the attribute capabilities of the exporter.
     *
     * \since QGIS 3.32
     */
    Qgis::VectorDataProviderAttributeEditCapabilities attributeEditCapabilities() const;

    bool addFeatures( QgsFeatureList &features, QgsFeatureSink::Flags flags = QgsFeatureSink::Flags() ) override;
    bool addFeature( QgsFeature &feature, QgsFeatureSink::Flags flags = QgsFeatureSink::Flags() ) override;
    QString lastError() const override;

    /**
     * Finalizes the export and closes the new created layer.
     */
    ~QgsVectorLayerExporter() override;

    bool flushBuffer() override;

  private:

    //! Create index
    bool createSpatialIndex();

    //! Contains error value
    Qgis::VectorExportResult mError;
    QString mErrorMessage;

    int mErrorCount;

    QgsVectorDataProvider *mProvider = nullptr;

    //! Map attribute indexes to new field indexes
    QMap<int, int> mOldToNewAttrIdx;
    int mAttributeCount;

    QgsFeatureList mFeatureBuffer;
    int mFeatureBufferMemoryUsage = 0;

    bool mCreateSpatialIndex = true;

#ifdef SIP_RUN
    QgsVectorLayerExporter( const QgsVectorLayerExporter &rh );
#endif

};


/**
 * \class QgsVectorLayerExporterTask
 * \ingroup core
 * \brief QgsTask task which performs a QgsVectorLayerExporter layer export operation as a background
 * task. This can be used to export a vector layer out to a provider without blocking the
 * QGIS interface.
 * \see QgsVectorFileWriterTask
 * \see QgsRasterFileWriterTask
 */
class CORE_EXPORT QgsVectorLayerExporterTask : public QgsTask
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsVectorLayerExporterTask. Takes a source \a layer, destination \a uri
     * and \a providerKey, and various export related parameters such as destination CRS
     * and export \a options. \a ownsLayer has to be set to TRUE if the task should take ownership
     * of the layer and delete it after export.
    */
    QgsVectorLayerExporterTask( QgsVectorLayer *layer,
                                const QString &uri,
                                const QString &providerKey,
                                const QgsCoordinateReferenceSystem &destinationCrs,
                                const QMap<QString, QVariant> &options = QMap<QString, QVariant>(),
                                bool ownsLayer = false );

    /**
     * Constructor for QgsVectorLayerExporterTask. Takes a source \a layer, destination \a uri
     * and \a providerKey, and various export related parameters via the \a exportOptions argument.
     *
     * \a ownsLayer has to be set to TRUE if the task should take ownership
     * of the layer and delete it after export.
     *
     * \since QGIS 3.44
    */
    QgsVectorLayerExporterTask( QgsVectorLayer *layer,
                                const QString &uri,
                                const QString &providerKey,
                                const QgsVectorLayerExporter::ExportOptions &exportOptions,
                                const QMap<QString, QVariant> &providerOptions = QMap<QString, QVariant>(),
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
    void errorOccurred( Qgis::VectorExportResult error, const QString &errorMessage );

  protected:

    bool run() override;
    void finished( bool result ) override;

  private:

    QPointer< QgsVectorLayer > mLayer = nullptr;
    bool mOwnsLayer = false;

    QString mDestUri;
    QString mDestProviderKey;
    QgsVectorLayerExporter::ExportOptions mExportOptions;
    QMap<QString, QVariant> mOptions;

    std::unique_ptr< QgsFeedback > mOwnedFeedback;

    Qgis::VectorExportResult mError = Qgis::VectorExportResult::Success;
    QString mErrorMessage;

};

#endif // QGSVECTORLAYEREXPORTER_H
