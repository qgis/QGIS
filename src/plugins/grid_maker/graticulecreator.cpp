#include "graticulecreator.h"
#include <stdio.h>
#include <cassert>
#include <qtextstream.h>
#include <iostream>
#include <fstream>
#include <qfileinfo.h>
#include <qstringlist.h>
#include <qgis.h>
GraticuleCreator::GraticuleCreator(QString theOutputFileName,ShapeType theType)
{
    std::cout << "GraticuleCreator constructor called with " << theOutputFileName.toLocal8Bit().data()
    << " for output file " << std::endl;
    /* Open and prepare output files */
    createDbf(theOutputFileName);
    createShapeFile(theOutputFileName,theType);
    writeProjectionFile(theOutputFileName);

}
GraticuleCreator::~GraticuleCreator()
{
    DBFClose( mDbfHandle );
    SHPClose( mShapeHandle );
}
    
/* DbfName need not include the file extension. */
void GraticuleCreator::createDbf (QString mDbfName )
{
    //remove the path part of the dbf name
    QFileInfo myFileInfo( mDbfName );
    QString myBaseString = myFileInfo.dirPath()+QString("/")+myFileInfo.baseName();  // excludes any extension
    //create the dbf
    mDbfHandle = DBFCreate( myBaseString+".dbf" );
    //create an index field named after the base part of the file name

    DBFAddField( mDbfHandle, myBaseString+"_id", FTInteger, 11, 0 );
    //create a second arbitary attribute field
    DBFAddField( mDbfHandle, "Date", FTString, 12, 0 );
    //close the dbf
    DBFClose( mDbfHandle );
    //reopen
    mDbfHandle = DBFOpen( myBaseString+".dbf", "r+b" );
    //exit this fn giving
    return;
}

void GraticuleCreator::createShapeFile(QString theFileName , ShapeType theType)
{
  switch (theType)
  {
      case POINT:
          mShapeHandle = SHPCreate(theFileName, SHPT_POINT );
          break;

      case LINE:
          mShapeHandle = SHPCreate(theFileName, SHPT_ARC );
          break;

      default: //POLYGON
          mShapeHandle = SHPCreate(theFileName, SHPT_POLYGON);
  }
  return;
}

void GraticuleCreator::writeDbfRecord ( int theRecordIdInt, QString theLabel)
{

  
    //std::cerr << "writeDbfRecord : " << theRecordIdInt << " - " << theLabel.toLocal8Bit().data();
    if (! DBFWriteIntegerAttribute(mDbfHandle, theRecordIdInt, 0, theRecordIdInt))
    {
        std::cerr <<  "DBFWriteIntegerAttribute failed. : " <<  theRecordIdInt << " - " << theRecordIdInt << std::endl;

        //exit(ERR_DBFWRITEINTEGERATTRIBUTE);
    }
    if (!theLabel.isNull())
    {
      if (! DBFWriteStringAttribute(mDbfHandle, theRecordIdInt, 1, theLabel))
      {
        std::cerr <<  "DBFWriteStringAttribute failed. : " <<  theRecordIdInt << " - " << theLabel.toLocal8Bit().data() <<std::endl;
        //exit(ERR_DBFWRITEINTEGERATTRIBUTE);
      }
      std::cerr << " - OK! " << std::endl;
    }
    //DBFWriteIntegerAttribute(mDbfHandle, theRecordIdInt, 0, theRecordIdInt);
    //DBFWriteStringAttribute(mDbfHandle, theRecordIdInt, 1, theLabel);
}

void  GraticuleCreator::writeProjectionFile(QString theFileName )
{
  // Write a WGS 84 projection file for the shapefile
  theFileName = theFileName.replace(".shp",".prj");
  std::ofstream of(theFileName);
  if(!of.fail())
  {
    of << GEOWKT.toLocal8Bit().data()
      << std::endl; 
    of.close();

  }
}
void GraticuleCreator::writePoint( int theRecordInt, double theXDouble, double theYDouble )
{
    SHPObject * myShapeObject;
    myShapeObject = SHPCreateObject( SHPT_POINT, theRecordInt, 0, NULL, NULL, 1, &theXDouble, &theYDouble, NULL, NULL );
    SHPWriteObject( mShapeHandle, -1, myShapeObject );
    SHPDestroyObject( myShapeObject );
}
void GraticuleCreator::writePoint(
        int theRecordInt, 
        int theCoordinateCountInt, 
        double * theXArrayDouble, 
        double * theYArrayDouble ) 
{
  SHPObject * myShapeObject;
  myShapeObject = SHPCreateObject( SHPT_POINT, 
                                   theRecordInt, 
                                   0, 
                                   NULL, 
                                   NULL,
                                   theCoordinateCountInt, 
                                   theXArrayDouble, 
                                   theYArrayDouble,
                                   NULL, 
                                   NULL );
  SHPWriteObject( mShapeHandle, -1, myShapeObject );
  SHPDestroyObject( myShapeObject );
}

void GraticuleCreator::writeLine(
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
  SHPWriteObject( mShapeHandle, -1, myShapeObject );
  SHPDestroyObject( myShapeObject );
}

void GraticuleCreator::writePolygon(
        int theRecordInt, 
        int theCoordinateCountInt, 
        double * theXArrayDouble, 
        double * theYArrayDouble ) 
{
  SHPObject * myShapeObject;
  myShapeObject = SHPCreateObject( SHPT_POLYGON, 
                                   theRecordInt, 
                                   0, 
                                   NULL, 
                                   NULL,
                                   theCoordinateCountInt, 
                                   theXArrayDouble, 
                                   theYArrayDouble,
                                   NULL, 
                                   NULL );
  SHPWriteObject( mShapeHandle, -1, myShapeObject );
  SHPDestroyObject( myShapeObject );
}
//TODO: check for rediculous intervals!
void GraticuleCreator::generatePointGraticule(
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
  long myXIntersectionCount = 1;
  long myYIntersectionCount = 1;
  myXArrayDouble = (double *)malloc(myYIntersectionCount * sizeof(double));
  myYArrayDouble = (double *)malloc(myYIntersectionCount * sizeof(double));
  for (double myXDouble = theXOriginDouble;myXDouble <=theXEndPointDouble;myXDouble+=theXIntervalDouble)
  {
    for (double myYDouble=theYOriginDouble;myYDouble<=theYEndPointDouble;myYDouble+=theYIntervalDouble)
    {
      myXArrayDouble[0]=myXDouble;
      myYArrayDouble[0]=myYDouble;
      writeDbfRecord(myRecordInt,"testing");
      writePoint( myRecordInt, myYIntersectionCount, myXArrayDouble, myYArrayDouble); 
    }
    ++myRecordInt;
  }
  delete myXArrayDouble;
  delete myYArrayDouble;

  // write a proj file for the graticule
  
}

//TODO: check for rediculous intervals!
void GraticuleCreator::generateLineGraticule(double theXIntervalDouble,
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
  
  long myXIntersectionCount = static_cast<long>((theXEndPointDouble - theXOriginDouble) / theXIntervalDouble)+1;
  long myYIntersectionCount = static_cast<long>((theYEndPointDouble - theYOriginDouble) / theYIntervalDouble)+1;
  
  
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
    writeDbfRecord(myRecordInt,"testing");
    writeLine( myRecordInt, myYIntersectionCount, myXArrayDouble, myYArrayDouble); 

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
    
    writeDbfRecord(myRecordInt,"testing");
    writeLine( myRecordInt,myXIntersectionCount, myXArrayDouble, myYArrayDouble); 

    ++myRecordInt;
  }
  
  delete myXArrayDouble;
  delete myYArrayDouble;
  // write a proj file for the graticule
  
}
//TODO: check for rediculous intervals!
void GraticuleCreator::generatePolygonGraticule(
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
  
  long myXVertexCount = 5; // four vertices of the sqaure for each cell, plus end at start
  long myYVertexCount = 5;
  
  
  //
  //Longitude loop
  //
  myXArrayDouble = (double *)malloc(myYVertexCount * sizeof(double));
  myYArrayDouble = (double *)malloc(myYVertexCount * sizeof(double));
  for (double myXDouble = theXOriginDouble ; myXDouble < theXEndPointDouble ; myXDouble+=theXIntervalDouble)
  {
    for (double myYDouble=theYOriginDouble;myYDouble<=theYEndPointDouble;myYDouble+=theYIntervalDouble)
    {
      //top left of rect
      myXArrayDouble[0]=myXDouble;
      myYArrayDouble[0]=myYDouble;
      //top right
      myXArrayDouble[1]=myXDouble+theXIntervalDouble;
      myYArrayDouble[1]=myYDouble;
      //bottom right
      myXArrayDouble[2]=myXDouble+theXIntervalDouble;
      myYArrayDouble[2]=myYDouble+theYIntervalDouble;
      //bottom left
      myXArrayDouble[3]=myXDouble;
      myYArrayDouble[3]=myYDouble+theYIntervalDouble;
      //return to top left
      myXArrayDouble[4]=myXDouble;
      myYArrayDouble[4]=myYDouble;
      writeDbfRecord(myRecordInt,"testing");
      writePolygon( myRecordInt, myYVertexCount, myXArrayDouble, myYArrayDouble); 

      ++myRecordInt;
    }
  }
  delete myXArrayDouble;
  delete myYArrayDouble;

  // write a proj file for the graticule
  
}

/* read from fp and generate point shapefile to mDbfHandle/mShapeHandle */
void GraticuleCreator::generatePoints (QString theInputFileName)
{
    QFile myFile( theInputFileName );
    if ( myFile.open( QIODevice::ReadOnly ) )
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
                std::cerr << "Writing record: " << myDateQString.toLocal8Bit().data() << " - " << x << " - " << y << std::endl;
                writeDbfRecord( myRecordInt, myDateQString);
                writePoint( myRecordInt, x, y);
                myRecordInt++;
            }
        }
        myFile.close();
    }
}




