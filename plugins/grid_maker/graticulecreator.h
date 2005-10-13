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
        enum ShapeType { POINT, LINE, POLYGON };
        GraticuleCreator(QString theOutputFileName, ShapeType theType);
        ~GraticuleCreator() ;
        void generatePointGraticule(
                               double theXIntervalDouble,
                               double theYIntervalDouble,
                               double theXOriginDouble,
                               double theYOriginDouble,
                               double theXEndPointDouble,
                               double theYEndPointDouble);
        void generateLineGraticule(
                               double theXIntervalDouble,
                               double theYIntervalDouble,
                               double theXOriginDouble,
                               double theYOriginDouble,
                               double theXEndPointDouble,
                               double theYEndPointDouble);
        void generatePolygonGraticule(
                               double theXIntervalDouble,
                               double theYIntervalDouble,
                               double theXOriginDouble,
                               double theYOriginDouble,
                               double theXEndPointDouble,
                               double theYEndPointDouble);
        void generatePoints (QString theInputFileName );

    private:
        DBFHandle mDbfHandle; 
        SHPHandle mShapeHandle;
        void GraticuleCreator::createDbf (QString theDbfName ) ;
        void GraticuleCreator::createShapeFile(QString theFileNamei, ShapeType theType ); 
        void writeDbfRecord ( int theRecordIdInt, QString theLabel) ;
        void writePoint(int theRecordInt, double theXDouble, double y ); 
        //! Writes a WGS 84 .prj file for the generated grid
        void writeProjectionFile(QString theFileName);
        void writePoint(
                int theRecordInt, 
                int theCoordinateCountInt, 
                double * theXArrayDouble, 
                double * theYArrayDouble ); 
        void writeLine(
                int theRecordInt, 
                int theCoordinateCountInt, 
                double * theXArrayDouble, 
                double * theYArrayDouble ); 
        void writePolygon(
                int theRecordInt, 
                int theCoordinateCountInt, 
                double * theXArrayDouble, 
                double * theYArrayDouble ); 
};
#endif
