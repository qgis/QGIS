#ifndef _SHAPEFILEMAKER_H_
#define _SHAPEFILEMAKER_H_

#include "shapefile.h"
#include "utils.h"

//qt includes
#include <qstring.h>
#include <qfile.h>

class ShapefileMaker
{
    public:
        ShapefileMaker(QString theOutputFileName);
        ShapefileMaker(QString theOutputFileName, double theXIntervalDouble, double theYIntervalDouble);
        ~ShapefileMaker() ;
        DBFHandle createDbf (QString theDbfName ) ;
        SHPHandle createShapeFile(QString theFileName ); 
        void writeDbfRecord (DBFHandle theDbfHandle, int theRecordIdInt, QString theLabel) ;
        void writePoint(SHPHandle theShapeHandle, int theRecordInt, double theXDouble, double y ); 
        void writePoint(QString theLabel, double theXDouble, double theYDouble ); 
        static void writeLine(SHPHandle theShapeHandle, 
                int theRecordInt, 
                int theCoordinateCountInt, 
                double * theXArrayDouble, 
                double * theYArrayDouble ); 
        void generateGraticule(DBFHandle theDbfHandle, SHPHandle theShapeHandle,double theXIntervalDouble,double theYIntervalDouble);
        void generatePoints (QString theInputFileName, DBFHandle theDbfHandle, SHPHandle theShapeHandle);

    private:
        DBFHandle mDbfHandle;		/* handle for dBase file */
        SHPHandle mShapeHandle;		/* handle for shape files .shx and .shp */
        int mCurrentRecInt;            /* current record number */

};
#endif
