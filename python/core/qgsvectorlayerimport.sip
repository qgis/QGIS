
/**
 There are two possibilities how to use this class:
 1. static call to QgsVectorFileWriter::writeAsShapefile(...) which saves the whole vector layer
 2. create an instance of the class and issue calls to addFeature(...)
 */
class QgsVectorLayerImport
{
%TypeHeaderCode
#include <qgsvectorlayerimport.h>
#include <qgsfield.h>
class QProgressDialog;
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
      ErrUserCancelled, /*!< User cancelled the import*/
    };

     /**
     * Writes the contents of vector layer to a different datasource.
     * @param layer source layer
     * @param uri URI for destination data source
     * @param providerKey string key for destination data provider
     * @param destCRS destination CRS, or an invalid (default constructed) CRS if
     * not available
     * @param onlySelected set to true to export only selected features
     * @param errorMessage if non-null, will be set to any error messages
     * @param skipAttributeCreation set to true to skip exporting feature attributes
     * @param options optional provider dataset options
     * @param progress optional progress dialog to show progress of export
     * @returns NoError for a successful export, or encountered error
     */
    static ImportError importLayer( QgsVectorLayer* layer,
                                    const QString& uri,
                                    const QString& providerKey,
                                    const QgsCoordinateReferenceSystem& destCRS,
                                    bool onlySelected = false,
                                    QString *errorMessage /Out/ = nullptr,
                                    bool skipAttributeCreation = false,
                                    QMap<QString, QVariant> *options = nullptr,
                                    QProgressDialog *progress = nullptr
                                  );

    /** Constructor for QgsVectorLayerImport.
     * @param uri URI for destination data source
     * @param provider string key for destination data provider
     * @param fields fields to include in created layer
     * @param geometryType destination geometry type
     * @param crs desired CRS, or an invalid (default constructed) CRS if
     * not available
     * @param overwrite set to true to overwrite any existing data source
     * @param options optional provider dataset options
     * @param progress optional progress dialog to show progress of export
     */
    QgsVectorLayerImport( const QString &uri,
                          const QString &provider,
                          const QgsFields &fields,
                          QgsWkbTypes::Type geometryType,
                          const QgsCoordinateReferenceSystem& crs,
                          bool overwrite = false,
                          const QMap<QString, QVariant> *options = nullptr,
                          QProgressDialog *progress = nullptr
                        );

    /** Checks whether there were any errors */
    ImportError hasError();

    /** Retrieves error message */
    QString errorMessage();

    int errorCount() const;

    /** Add feature to the new created layer */
    bool addFeature( QgsFeature& feature );

    /** Close the new created layer */
    ~QgsVectorLayerImport();

  protected:
    /** Flush the buffer writing the features to the new layer */
    bool flushBuffer();

    /** Create index */
    bool createSpatialIndex();

  private:

    QgsVectorLayerImport( const QgsVectorLayerImport& rh );
};
