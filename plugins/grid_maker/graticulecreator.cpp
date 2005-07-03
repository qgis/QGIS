#include "graticulecreator.h"
#include <stdio.h>
#include <cassert>
#include <qtextstream.h>
#include <iostream>
#include <fstream>
#include <qfileinfo.h>
#include <qstringlist.h>
#include <qgis.h>
GraticuleCreator::GraticuleCreator(QString theOutputFileName, 
                                   double theXIntervalDouble, 
                                   double theYIntervalDouble,
                                   double theXOriginDouble,
                                   double theYOriginDouble,
                                   double theXEndPointDouble,
                                   double theYEndPointDouble
                                   )
{
    std::cout << "GraticuleCreator constructor called with " << theOutputFileName
    << " for output file and " << theXIntervalDouble << "," << theYIntervalDouble << " for x,y interval " << std::endl;
    DBFHandle myDbfHandle;    /* handle for dBase file */
    SHPHandle myShapeHandle;    /* handle for shape files .shx and .shp */
    /* Open and prepare output files */
    myDbfHandle = createDbf(theOutputFileName);
    myShapeHandle = createShapeFile(theOutputFileName);
    writeProjectionFile(theOutputFileName);
    //test the write point routine....
    //generatePoints(theInputFileName,myDbfHandle,myShapeHandle);
    generateGraticule(myDbfHandle,
                      myShapeHandle,
                      theXIntervalDouble,
                      theYIntervalDouble,
                      theXOriginDouble,
                      theYOriginDouble,
                      theXEndPointDouble,
                      theYEndPointDouble);
    DBFClose( myDbfHandle );
    SHPClose( myShapeHandle );
    return;
}

/* DbfName need not include the file extension. */
DBFHandle GraticuleCreator::createDbf (QString theDbfName )
{
    DBFHandle myDbfHandle;
    //remove the path part of the dbf name
    QFileInfo myFileInfo( theDbfName );
    QString myBaseString = myFileInfo.dirPath()+QString("/")+myFileInfo.baseName();  // excludes any extension
    //create the dbf
    myDbfHandle = DBFCreate( myBaseString+".dbf" );
    //create an index field named after the base part of the file name

    DBFAddField( myDbfHandle, myBaseString+"_id", FTInteger, 11, 0 );
    //create a second arbitary attribute field
    DBFAddField( myDbfHandle, "Date", FTString, 12, 0 );
    //close the dbf
    DBFClose( myDbfHandle );
    //reopen
    myDbfHandle = DBFOpen( myBaseString+".dbf", "r+b" );
    //exit this fn giving
    return myDbfHandle;
}

SHPHandle GraticuleCreator::createShapeFile(QString theFileName )
{
    SHPHandle myShapeHandle;
    //myShapeHandle = SHPCreate(theFileName, SHPT_POINT );
    myShapeHandle = SHPCreate(theFileName, SHPT_ARC );
    return myShapeHandle;
}

void GraticuleCreator::writeDbfRecord (DBFHandle theDbfHandle, int theRecordIdInt, QString theLabel)
{

  
    std::cerr << "writeDbfRecord : " << theRecordIdInt << " - " << theLabel;
    if (! DBFWriteIntegerAttribute(theDbfHandle, theRecordIdInt, 0, theRecordIdInt))
    {
        std::cerr <<  "DBFWriteIntegerAttribute failed. : " <<  theRecordIdInt << " - " << theRecordIdInt << std::endl;

        //exit(ERR_DBFWRITEINTEGERATTRIBUTE);
    }
    if (theLabel != NULL)
    {
      if (! DBFWriteStringAttribute(theDbfHandle, theRecordIdInt, 1, theLabel))
      {
        std::cerr <<  "DBFWriteStringAttribute failed. : " <<  theRecordIdInt << " - " << theLabel <<std::endl;
        //exit(ERR_DBFWRITEINTEGERATTRIBUTE);
      }
      std::cerr << " - OK! " << std::endl;
    }
    //DBFWriteIntegerAttribute(theDbfHandle, theRecordIdInt, 0, theRecordIdInt);
    //DBFWriteStringAttribute(theDbfHandle, theRecordIdInt, 1, theLabel);
}

void  GraticuleCreator::writeProjectionFile(QString theFileName )
{
  // Write a WGS 84 projection file for the shapefile
  theFileName = theFileName.replace(".shp",".prj");
  std::ofstream of(theFileName);
  if(!of.fail())
  {
    of << GEOWKT
      << std::endl; 
    of.close();

  }
}
void GraticuleCreator::writePoint(SHPHandle theShapeHandle, int theRecordInt, double theXDouble, double theYDouble )
{
    SHPObject * myShapeObject;
    myShapeObject = SHPCreateObject( SHPT_POINT, theRecordInt, 0, NULL, NULL, 1, &theXDouble, &theYDouble, NULL, NULL );
    SHPWriteObject( theShapeHandle, -1, myShapeObject );
    SHPDestroyObject( myShapeObject );
}

void GraticuleCreator::writeLine(SHPHandle theShapeHandle, 
        int theRecordInt, 
        int theCoordinateCountInt, 
        double * theXArrayDouble, 
        double * theYArrayDouble ) 
{
  SHPObject * myShapeObject;
  myShapeObject = SHPCreateObject( SHPT_ARC, 
                                   theRecordInt, 
                                   0, 
                                   NULL, 
                                   NULL,
                                   theCoordinateCountInt, 
                                   theXArrayDouble, 
                                   theYArrayDouble,
                                   NULL, 
                                   NULL );
  SHPWriteObject( theShapeHandle, -1, myShapeObject );
  SHPDestroyObject( myShapeObject );
}

//TODO: check for rediculous intervals!
void GraticuleCreator::generateGraticule(DBFHandle theDbfHandle, 
                                         SHPHandle theShapeHandle,
                                         double theXIntervalDouble,
                                         double theYIntervalDouble,
                                         double theXOriginDouble,
                                         double theYOriginDouble,
                                         double theXEndPointDouble,
                                         double theYEndPointDouble)
{
  
  int myRecordInt=0;
  //create the arrays for storing the coordinates
  double * myXArrayDouble;
  double * myYArrayDouble;
  //we want out graticule to be made of short line segments rather tban
  //long ones that imply span xmin <-> xmax
  //so that when reprojecting the graticule will warp properly
  //so first we need to work out how many intersections there are...
  //
  
  long myXIntersectionCount = ((theXEndPointDouble - theXOriginDouble) / theXIntervalDouble)+1;
  long myYIntersectionCount = ((theYEndPointDouble - theYOriginDouble) / theYIntervalDouble)+1;
  
  
  //
  //Longitude loop
  //
  myXArrayDouble = (double *)malloc(myYIntersectionCount * sizeof(double));
  myYArrayDouble = (double *)malloc(myYIntersectionCount * sizeof(double));
  for (double myXDouble = theXOriginDouble;myXDouble <=theXEndPointDouble;myXDouble+=theXIntervalDouble)
  {
    long myVertexNo=0;
    for (double myYDouble=theYOriginDouble;myYDouble<=theYEndPointDouble;myYDouble+=theYIntervalDouble)
    {
      myXArrayDouble[myVertexNo]=myXDouble;
      myYArrayDouble[myVertexNo]=myYDouble;
      ++myVertexNo;
    }
    writeDbfRecord(theDbfHandle,myRecordInt,"testing");
    writeLine(theShapeHandle, myRecordInt, myYIntersectionCount, myXArrayDouble, myYArrayDouble); 

    ++myRecordInt;
  }
  delete myXArrayDouble;
  delete myYArrayDouble;

  //
  //Latitude loop
  //
  myXArrayDouble = (double *)malloc(myXIntersectionCount * sizeof(double));
  myYArrayDouble = (double *)malloc(myXIntersectionCount * sizeof(double));
  for (double myYDouble=theYOriginDouble;myYDouble<=theYEndPointDouble;myYDouble+=theYIntervalDouble)
  {
    long myVertexNo=0;
    for (double myXDouble=theXOriginDouble;myXDouble<=theXEndPointDouble;myXDouble+=theXIntervalDouble)
    {
      myXArrayDouble[myVertexNo]=myXDouble;
      myYArrayDouble[myVertexNo]=myYDouble;
      ++myVertexNo;
    }
    
    writeDbfRecord(theDbfHandle,myRecordInt,"testing");
    writeLine(theShapeHandle, myRecordInt,myXIntersectionCount, myXArrayDouble, myYArrayDouble); 

    ++myRecordInt;
  }
  
  delete myXArrayDouble;
  delete myYArrayDouble;
  // write a proj file for the graticule
  
}

/* read from fp and generate point shapefile to theDbfHandle/theShapeHandle */
void GraticuleCreator::generatePoints (QString theInputFileName, DBFHandle theDbfHandle, SHPHandle theShapeHandle)
{
    QFile myFile( theInputFileName );
    if ( myFile.open( IO_ReadOnly ) )
    {
        QTextStream myStream( &myFile );
        QString myLineString;
        int myRecordInt = 0;
        while ( !myStream.atEnd() )
        {
            // line of text excluding '\n'
            myLineString = myStream.readLine();
            //tokenise the line so we can get coords and records
            QStringList myQStringList = QStringList::split("\t",myLineString,true);

            if (myQStringList.size()==4)
            {
                QString myDateQString = myQStringList[1];
                QString myLatQString = myQStringList[2];
                QString myLongQString = myQStringList[3];

                //convert items 3 and 4 to lat and long...
                //TODO - continue here...
                double x=myLongQString.toDouble();
                double y=myLatQString.toDouble();
                //create the dbf and shape recs
                std::cerr << "Writing record: " << myDateQString << " - " << x << " - " << y << std::endl;
                writeDbfRecord(theDbfHandle, myRecordInt, myDateQString);
                writePoint(theShapeHandle, myRecordInt, x, y);
                myRecordInt++;
            }
        }
        myFile.close();
    }
}




