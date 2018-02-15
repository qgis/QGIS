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
                                    const QMap<QString, QVariant> &options = QMap<QString, QVariant>(),
                                    QgsFeedback *feedback = 0
                                  );
%Docstring
Writes the contents of vector layer to a different datasource.

:param layer: source layer
:param uri: URI for destination data source
:param providerKey: string key for destination data provider
:param destCRS: destination CRS, or an invalid (default constructed) CRS if
not available
:param onlySelected: set to true to export only selected features
:param errorMessage: if non-null, will be set to any error messages
:param options: optional provider dataset options
:param feedback: optional feedback object to show progress and cancelation of export

:return: NoError for a successful export, or encountered error
%End

    QgsVectorLayerExporter( const QString &uri,
                            const QString &provider,
                            const QgsFields &fields,
                            QgsWkbTypes::Type geometryType,
                            const QgsCoordinateReferenceSystem &crs,
                            bool overwrite = false,
                            const QMap<QString, QVariant> &options = QMap<QString, QVariant>() );
%Docstring
Constructor for QgsVectorLayerExporter.

:param uri: URI for destination data source
:param provider: string key for destination data provider
:param fields: fields to include in created layer
:param geometryType: destination geometry type
:param crs: desired CRS, or an invalid (default constructed) CRS if
not available
:param overwrite: set to true to overwrite any existing data source
:param options: optional provider dataset options
%End


    ExportError errorCode() const;
%Docstring
Returns any encountered error code, or false if no error was encountered.

.. seealso:: :py:func:`errorMessage`

.. seealso:: :py:func:`errorCount`
%End

    QString errorMessage() const;
%Docstring
Returns any error message encountered during the export.

.. seealso:: :py:func:`errorCount`

.. seealso:: :py:func:`errorCode`
%End

    int errorCount() const;
%Docstring
Returns the number of error messages encountered during the export.

.. seealso:: :py:func:`errorMessage`

.. seealso:: :py:func:`errorCode`
%End

    virtual bool addFeatures( QgsFeatureList &features, QgsFeatureSink::Flags flags = 0 );

    virtual bool addFeature( QgsFeature &feature, QgsFeatureSink::Flags flags = 0 );


    ~QgsVectorLayerExporter();

    virtual bool flushBuffer();


  private:
    QgsVectorLayerExporter( const QgsVectorLayerExporter &rh );
};


class QgsVectorLayerExporterTask : QgsTask
{
%Docstring
QgsTask task which performs a QgsVectorLayerExporter layer export operation as a background
task. This can be used to export a vector layer out to a provider without blocking the
QGIS interface.

.. versionadded:: 3.0

.. seealso:: :py:class:`QgsVectorFileWriterTask`

.. seealso:: :py:class:`QgsRasterFileWriterTask`
%End

%TypeHeaderCode
#include "qgsvectorlayerexporter.h"
%End
  public:

    QgsVectorLayerExporterTask( QgsVectorLayer *layer,
                                const QString &uri,
                                const QString &providerKey,
                                const QgsCoordinateReferenceSystem &destinationCrs,
                                const QMap<QString, QVariant> &options = QMap<QString, QVariant>(),
                                bool ownsLayer = false );
%Docstring
Constructor for QgsVectorLayerExporterTask. Takes a source ``layer``, destination ``uri``
and ``providerKey``, and various export related parameters such as destination CRS
and export ``options``. ``ownsLayer`` has to be set to true if the task should take ownership
of the layer and delete it after export.
%End

    static QgsVectorLayerExporterTask *withLayerOwnership( QgsVectorLayer *layer /Transfer/,
        const QString &uri,
        const QString &providerKey,
        const QgsCoordinateReferenceSystem &destinationCrs,
        const QMap<QString, QVariant> &options = QMap<QString, QVariant>() ) /Factory/;
%Docstring
Creates a new QgsVectorLayerExporterTask which has ownership over a source ``layer``.
When the export task has completed (successfully or otherwise) ``layer`` will be
deleted. The destination ``uri`` and ``providerKey``, and various export related parameters such as destination CRS
and export ``options`` must be specified.
%End

    virtual void cancel();


  signals:

    void exportComplete();
%Docstring
Emitted when exporting the layer is successfully completed.
%End

    void errorOccurred( int error, const QString &errorMessage );
%Docstring
Emitted when an error occurs which prevented the layer being exported (or if
the task is canceled). The export ``error`` and ``errorMessage`` will be reported.
%End

  protected:

    virtual bool run();

    virtual void finished( bool result );


};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsvectorlayerexporter.h                                    *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
