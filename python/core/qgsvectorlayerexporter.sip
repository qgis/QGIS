/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsvectorlayerexporter.h                                    *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsVectorLayerExporter : QgsFeatureSink
{
%Docstring
 A convenience class for exporting vector layers to a destination data provider.

 QgsVectorLayerExporter can be used in two ways:

 1. Using a static call to QgsVectorLayerExporter.exportLayer(...) which exports the
 entire layer to the destination provider.

 2. Create an instance of the class and issue calls to addFeature(...)

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsvectorlayerexporter.h"
%End
  public:

    enum ExportError
    {
      NoError,
      ErrCreateDataSource,
      ErrCreateLayer,
      ErrAttributeTypeUnsupported,
      ErrAttributeCreationFailed,
      ErrProjection,
      ErrFeatureWriteFailed,
      ErrInvalidLayer,
      ErrInvalidProvider,
      ErrProviderUnsupportedFeature,
      ErrConnectionFailed,
      ErrUserCanceled,
    };

    static ExportError exportLayer( QgsVectorLayer *layer,
                                    const QString &uri,
                                    const QString &providerKey,
                                    const QgsCoordinateReferenceSystem &destCRS,
                                    bool onlySelected = false,
                                    QString *errorMessage /Out/ = 0,
                                    bool skipAttributeCreation = false,
                                    QMap<QString, QVariant> *options = 0,
                                    QProgressDialog *progress = 0
                                  );
%Docstring
 Writes the contents of vector layer to a different datasource.
 \param layer source layer
 \param uri URI for destination data source
 \param providerKey string key for destination data provider
 \param destCRS destination CRS, or an invalid (default constructed) CRS if
 not available
 \param onlySelected set to true to export only selected features
 \param errorMessage if non-null, will be set to any error messages
 \param skipAttributeCreation set to true to skip exporting feature attributes
 \param options optional provider dataset options
 \param progress optional progress dialog to show progress of export
 :return: NoError for a successful export, or encountered error
 :rtype: ExportError
%End

    QgsVectorLayerExporter( const QString &uri,
                            const QString &provider,
                            const QgsFields &fields,
                            QgsWkbTypes::Type geometryType,
                            const QgsCoordinateReferenceSystem &crs,
                            bool overwrite = false,
                            const QMap<QString, QVariant> *options = 0 );
%Docstring
 Constructor for QgsVectorLayerExporter.
 \param uri URI for destination data source
 \param provider string key for destination data provider
 \param fields fields to include in created layer
 \param geometryType destination geometry type
 \param crs desired CRS, or an invalid (default constructed) CRS if
 not available
 \param overwrite set to true to overwrite any existing data source
 \param options optional provider dataset options
%End


    ExportError errorCode() const;
%Docstring
 Returns any encountered error code, or false if no error was encountered.
.. seealso:: errorMessage()
.. seealso:: errorCount()
 :rtype: ExportError
%End

    QString errorMessage() const;
%Docstring
 Returns any error message encountered during the export.
.. seealso:: errorCount()
.. seealso:: errorCode()
 :rtype: str
%End

    int errorCount() const;
%Docstring
 Returns the number of error messages encountered during the export.
.. seealso:: errorMessage()
.. seealso:: errorCode()
 :rtype: int
%End

    virtual bool addFeatures( QgsFeatureList &features );

    virtual bool addFeature( QgsFeature &feature );


    ~QgsVectorLayerExporter();
%Docstring
 Finalizes the export and closes the new created layer.
%End

  private:
    QgsVectorLayerExporter( const QgsVectorLayerExporter &rh );
};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsvectorlayerexporter.h                                    *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
