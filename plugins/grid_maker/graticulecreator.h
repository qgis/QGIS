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
        GraticuleCreator(QString theOutputFileName, 
                         double theXIntervalDouble, 
                         double theYIntervalDouble,
                         double theXOriginDouble,
                         double theYOriginDouble,
                         double theXEndPointDouble,
                         double theYEndPointDouble);
        ~GraticuleCreator() {};
        DBFHandle GraticuleCreator::createDbf (QString theDbfName ) ;
        SHPHandle GraticuleCreator::createShapeFile(QString theFileName ); 
        void writeDbfRecord (DBFHandle theDbfHandle, int theRecordIdInt, QString theLabel) ;
        void writePoint(SHPHandle theShapeHandle, int theRecordInt, double theXDouble, double y ); 
        static void writeLine(SHPHandle theShapeHandle, 
                int theRecordInt, 
                int theCoordinateCountInt, 
                double * theXArrayDouble, 
                double * theYArrayDouble ); 
        void generateGraticule(DBFHandle theDbfHandle, 
                               SHPHandle theShapeHandle,
                               double theXIntervalDouble,
                               double theYIntervalDouble,
                               double theXOriginDouble,
                               double theYOriginDouble,
                               double theXEndPointDouble,
                               double theYEndPointDouble);
        void generatePoints (QString theInputFileName, DBFHandle theDbfHandle, SHPHandle theShapeHandle);

    private:
};
#endif
