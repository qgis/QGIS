#ifndef _QGSVECTORFILEWRITER_H_
#define _QGSVECTORFILEWRITER_H_

// qgis includes
#include "qgspoint.h"
#include "qgsvectorlayer.h"

//qt includes
#include <qstring.h>
#include <qfile.h>

// OGR Includes
#include "ogr_api.h"

class QgsVectorFileWriter
{
    public:
        QgsVectorFileWriter(QString theOutputFileName, QgsVectorLayer * theVectorLayer);
        QgsVectorFileWriter(QString theOutputFileName, OGRwkbGeometryType theGeometryType);
        ~QgsVectorFileWriter() ;
        bool writePoint(QgsPoint * thePoint); 
        //! Add a new field to the output attribute table
        bool createField(QString theName, OGRFieldType theType, int theWidthInt=0, int thePrecisionInt=0);
        //! creates the output file etc...
        bool initialise();
    private:
        //! current record number
        int mCurrentRecInt;    
        //! file name to be written to 
        QString mOutputFileName;
        //! file type to be written to
        QString mOutputFormat;
        //! Ogr handle to the output datasource
        OGRDataSourceH mDataSourceHandle;
        //! Ogr handle to the spatial layer (e.g. .shp) parrt of the datasource
        OGRLayerH mLayerHandle;
        //! The geometry type for the output file
        OGRwkbGeometryType mGeometryType;
        //! Whether the output gile has been initialised. Some operations require this to be true before they will run
        bool mInitialisedFlag;
};
#endif
