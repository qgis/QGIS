
/**
 There are two possibilities how to use this class:
 1. static call to QgsVectorFileWriter::writeAsShapefile(...) which saves the whole vector layer
 2. create an instance of the class and issue calls to addFeature(...)

 Currently supports only writing to shapefiles, but shouldn't be a problem to add capability
 to support other OGR-writable formats.
 */
class QgsVectorFileWriter
{
%TypeHeaderCode
#include <qgsvectorfilewriter.h>
#include <qgsfield.h>
%End

  public:
    enum WriterError
    {
      NoError = 0,
      ErrDriverNotFound,
      ErrCreateDataSource,
      ErrCreateLayer,
      ErrAttributeTypeUnsupported,
      ErrAttributeCreationFailed,
      ErrProjection,  // added in 1.5
      ErrFeatureWriteFailed, // added in 1.6
      ErrInvalidLayer, // added in 2.0
    };

    /** Write contents of vector layer to an (OGR supported) vector formt
        @note: this method was added in version 1.5*/
    static WriterError writeAsVectorFormat( QgsVectorLayer* layer,
                                            const QString& fileName,
                                            const QString& fileEncoding,
                                            const QgsCoordinateReferenceSystem *destCRS,
                                            const QString& driverName = "ESRI Shapefile",
                                            bool onlySelected = false,
                                            QString *errorMessage = 0,
                                            const QStringList &datasourceOptions = QStringList(),  // added in 1.6
                                            const QStringList &layerOptions = QStringList(),  // added in 1.6
                                            bool skipAttributeCreation = false, // added in 1.6
                                            QString *newFilename = 0 // added in 1.9
                                          );

    /** create shapefile and initialize it */
    QgsVectorFileWriter( const QString& vectorFileName,
                         const QString& fileEncoding,
                         const QgsFields& fields,
                         QGis::WkbType geometryType,
                         const QgsCoordinateReferenceSystem* srs,
                         const QString& driverName = "ESRI Shapefile",
                         const QStringList &datasourceOptions = QStringList(), // added in 1.6
                         const QStringList &layerOptions = QStringList(), // added in 1.6
                         QString *newFilename = 0 // added in 1.9
                       );

    /**Returns map with format filter string as key and OGR format key as value*/
    static QMap< QString, QString> supportedFiltersAndFormats();

    /**Returns driver list that can be used for dialogs*/
    static QMap< QString, QString> ogrDriverList();

    /**Returns filter string that can be used for dialogs*/
    static QString fileFilterString();

    /**Creates a filter for an OGR driver key*/
    static QString filterForDriver( const QString& driverName );

    /** checks whether there were any errors in constructor */
    WriterError hasError();

    /** retrieves error message
     * @note added in 1.5
     */
    QString errorMessage();

    /** add feature to the currently opened shapefile */
    bool addFeature( QgsFeature& feature );

    // QMap<int, int> attrIdxToOgrIdx();

    /** close opened shapefile for writing */
    ~QgsVectorFileWriter();

    /** Delete a shapefile (and its accompanying shx / dbf / prf)
     * @param theFileName /path/to/file.shp
     * @return bool true if the file was deleted successfully
     */
    static bool deleteShapeFile( QString theFileName );

  protected:

    // OGRGeometryH createEmptyGeometry( QGis::WkbType wkbType );
};

