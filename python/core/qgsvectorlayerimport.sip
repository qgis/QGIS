
/**
 There are two possibilities how to use this class:
 1. static call to QgsVectorLayerImport::importLayer(...) which saves the whole vector layer
 2. create an instance of the class and issue calls to addFeature(...)
 */
class QgsVectorLayerImport
{
%TypeHeaderCode
#include <qgsvectorlayerimport.h>
#include <qgsfield.h>
%End

public:
      
    enum ImportError
    {
      NoError = 0,
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
      ErrConnectionFailed
    };

    /** Write contents of vector layer to a different datasource */
    static ImportError importLayer( QgsVectorLayer* layer,
                                    const QString& uri,
                                    const QString& providerKey,
                                    const QgsCoordinateReferenceSystem *destCRS,
                                    bool onlySelected = FALSE,
                                    QString *errorMessage /Out/ = 0,
                                    bool skipAttributeCreation = FALSE,
                                    QMap<QString, QVariant> *options = 0
                                  );

    /** create a empty layer and add fields to it */
    QgsVectorLayerImport( const QString &uri,
                          const QString &provider,
                          const QgsFieldMap& fields,
                          QGis::WkbType geometryType,
                          const QgsCoordinateReferenceSystem* crs,
                          bool overwrite = false,
                          const QMap<QString, QVariant> *options = 0
                        );

    /** checks whether there were any errors */
    ImportError hasError();

    /** retrieves error message */
    QString errorMessage();

    /** add feature to the new created layer */
    bool addFeature( QgsFeature& feature );

    /** close the new created layer */
    ~QgsVectorLayerImport();
};

