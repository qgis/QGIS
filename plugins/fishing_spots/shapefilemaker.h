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
        ~ShapefileMaker() {};
        DBFHandle ShapefileMaker::createDbf (QString theDbfName ) ;
        SHPHandle ShapefileMaker::createShapeFile(QString theFileName ); 
        void writeDbfRecord (DBFHandle theDbfHandle, int theRecordIdInt, QString theLabel) ;
        void writePoint(SHPHandle theShapeHandle, int theRecordInt, double theXDouble, double y ); 
        static void writeLine(SHPHandle theShapeHandle, 
                int theRecordInt, 
                int theCoordinateCountInt, 
                double * theXArrayDouble, 
                double * theYArrayDouble ); 
        void generateGraticule(DBFHandle theDbfHandle, SHPHandle theShapeHandle,double theXIntervalDouble,double theYIntervalDouble);
        void generatePoints (QString theInputFileName, DBFHandle theDbfHandle, SHPHandle theShapeHandle);

    private:
};
#endif
