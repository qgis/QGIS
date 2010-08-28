
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
      ErrCreateLayer
    };

    /** Write contents of vector layer to a shapefile */
    static WriterError writeAsShapefile(QgsVectorLayer* layer,
                                        const QString& shapefileName,
                                        const QString& fileEncoding,
                                        const QgsCoordinateReferenceSystem*,
                                        bool onlySelected = FALSE,
					QString *errorMessage = 0,
					const QStringList &datasourceOptions = QStringList(),
					const QStringList &layerOptions = QStringList() );

    /** Write contents of vector layer to an (OGR supported) vector formt
        @note: this method was added in version 1.5*/
    static WriterError writeAsVectorFormat( QgsVectorLayer* layer,
                                            const QString& fileName,
                                            const QString& fileEncoding,
                                            const QgsCoordinateReferenceSystem *destCRS,
                                            const QString& driverName = "ESRI Shapefile",
                                            bool onlySelected = FALSE,
                                            QString *errorMessage = 0,
					    const QStringList &datasourceOptions = QStringList(),
					    const QStringList &layerOptions = QStringList() );

    /** create shapefile and initialize it */
    QgsVectorFileWriter(const QString& vectorFileName,
                        const QString& fileEncoding,
                        const QMap<int, QgsField>& fields,
                        QGis::WkbType geometryType,
                        const QgsCoordinateReferenceSystem* srs,
			const QString& driverName = "ESRI Shapefile",
			const QStringList &datasourceOptions = QStringList(),
			const QStringList &layerOptions = QStringList() );

    /**Returns map with format filter string as key and OGR format key as value*/
    static QMap< QString, QString> supportedFiltersAndFormats();

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
    bool addFeature(QgsFeature& feature);
    
    /** close opened shapefile for writing */
    ~QgsVectorFileWriter();
    
    /** Delete a shapefile (and its accompanying shx / dbf / prf)
     * @param QString theFileName - /path/to/file.shp
     * @return bool true if the file was deleted successfully
     */ 
    static bool deleteShapeFile(QString theFileName);
};

