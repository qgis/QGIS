
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
                                        bool onlySelected = FALSE);

    /** create shapefile and initialize it */
    QgsVectorFileWriter(const QString& shapefileName,
                        const QString& fileEncoding,
                        const QMap<int, QgsField>& fields,
                        QGis::WkbType geometryType,
                        const QgsCoordinateReferenceSystem* srs,
			const QString& driverName = "ESRI Shapefile" );
    
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

