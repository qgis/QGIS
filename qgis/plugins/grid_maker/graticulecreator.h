#ifndef _GRATICULECREATOR_H_
#define _GRATICULECREATOR_H_

#include "shapefile.h"
#include "utils.h"

//qt includes
#include <qstring.h>
#include <qfile.h>

class GraticuleCreator
{
    public:
        GraticuleCreator::GraticuleCreator(QString theOutputFileName, QString theInputFileName ); 
        ~GraticuleCreator() {};
        DBFHandle GraticuleCreator::createDbf (QString theDbfName ) ;
        SHPHandle GraticuleCreator::createShapeFile(QString theFileName ); 
        void writeDbfRecord (DBFHandle theDbfHandle, int theRecordIdInt, QString theLabel) ;
        void writePoint(SHPHandle theShapeHandle, int theRecordInt, double theXFloat, double y ); 
        static void WriteLine(SHPHandle theShapeHandle, 
                int theRecordInt, 
                int theCoordinateCountInt, 
                double * theXArrayDouble, 
                double * theYArrayDouble ); 
        void generateGraticule(DBFHandle theDbfHandle, SHPHandle theShapeHandle,float theXIntervalFloat,float theYIntervalFloat);

    private:
};
#endif
