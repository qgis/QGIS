/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsvectorlayerimport.h                                      *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsVectorLayerImport : QgsFeatureSink
{
%Docstring
 A convenience class for writing vector files to disk.
There are two possibilities how to use this class:
1. static call to QgsVectorFileWriter.writeAsShapefile(...) which saves the whole vector layer
2. create an instance of the class and issue calls to addFeature(...)
%End

%TypeHeaderCode
#include "qgsvectorlayerimport.h"
%End
  public:

    enum ImportError
    {
      NoError,
      ErrDriverNotFound,
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

    static ImportError importLayer( QgsVectorLayer *layer,
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
 :rtype: ImportError
%End

    QgsVectorLayerImport( const QString &uri,
                          const QString &provider,
                          const QgsFields &fields,
                          QgsWkbTypes::Type geometryType,
                          const QgsCoordinateReferenceSystem &crs,
                          bool overwrite = false,
                          const QMap<QString, QVariant> *options = 0 );
%Docstring
 Constructor for QgsVectorLayerImport.
 \param uri URI for destination data source
 \param provider string key for destination data provider
 \param fields fields to include in created layer
 \param geometryType destination geometry type
 \param crs desired CRS, or an invalid (default constructed) CRS if
 not available
 \param overwrite set to true to overwrite any existing data source
 \param options optional provider dataset options
%End


    ImportError hasError();
%Docstring
Checks whether there were any errors
 :rtype: ImportError
%End

    QString errorMessage();
%Docstring
Retrieves error message
 :rtype: str
%End

    int errorCount() const;
%Docstring
 :rtype: int
%End

    virtual bool addFeatures( QgsFeatureList &features );

    virtual bool addFeature( QgsFeature &feature );


    ~QgsVectorLayerImport();
%Docstring
Close the new created layer
%End

  private:
    QgsVectorLayerImport( const QgsVectorLayerImport &rh );
};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsvectorlayerimport.h                                      *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
