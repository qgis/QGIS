#ifndef _WAYPOINTTOSHAPE_H_
#define _WAYPOINTTOSHAPE_H_

#include "shapefile.h"
#include "utils.h"

//qt includes
#include <qstring.h>
#include <qfile.h>

class WayPointToShape
{
    public:
        WayPointToShape::WayPointToShape(QString theOutputFileName, QString theInputFileName ); 
        ~WayPointToShape() {};
        DBFHandle WayPointToShape::createDbf (QString theDbfName ) ;
        SHPHandle WayPointToShape::createShapeFile(QString theFileName ); 
        void writeDbfRecord (DBFHandle theDbfHandle, int theRecordIdInt, QString theLabel) ;
        void writePoint(SHPHandle theShapeHandle, int theRecordInt, double theXFloat, double y ); 
        void generatePoints (QString theInputFileName, DBFHandle theDbfHandle, SHPHandle theShapeHandle) ;

    private:
};
#endif
