#include "shapefilemaker.h"
#include <stdio.h>
#include <qtextstream.h>
#include <iostream>
#include <qfileinfo.h>
#include <qstringlist.h>
ShapefileMaker::ShapefileMaker(QString theOutputFileName)
{
    std::cout << "ShapefileMaker constructor called with " << theOutputFileName << std::endl;
    /* Open and prepare output files */
    mDbfHandle = createDbf(theOutputFileName);
    mShapeHandle = createShapeFile(theOutputFileName);
    mCurrentRecInt=0;
    //test the write point routine....
    //generatePoints(theInputFileName,mDbfHandle,mShapeHandle);
    //generateGraticule(mDbfHandle,mShapeHandle,theXIntervalDouble,theYIntervalDouble);
}

ShapefileMaker::ShapefileMaker(QString theOutputFileName, double theXIntervalDouble, double theYIntervalDouble)
{
    std::cout << "ShapefileMaker constructor called with " << theOutputFileName
    << " for output file and " << theXIntervalDouble << "," << theYIntervalDouble << " for x,y interval " << std::endl;
    DBFHandle mDbfHandle;		/* handle for dBase file */
    SHPHandle mShapeHandle;		/* handle for shape files .shx and .shp */
    /* Open and prepare output files */
    mDbfHandle = createDbf(theOutputFileName);
    mShapeHandle = createShapeFile(theOutputFileName);
    //test the write point routine....
    //generatePoints(theInputFileName,mDbfHandle,mShapeHandle);
    generateGraticule(mDbfHandle,mShapeHandle,theXIntervalDouble,theYIntervalDouble);
    return;
}

ShapefileMaker::~ShapefileMaker()
{
    DBFClose( mDbfHandle );
    SHPClose( mShapeHandle );
}

/* DbfName need not include the file extension. */
DBFHandle ShapefileMaker::createDbf (QString theDbfName )
{
    DBFHandle mDbfHandle;
    //remove the path part of the dbf name
    QFileInfo myFileInfo( theDbfName );
    QString myBaseString = myFileInfo.dirPath()+QString("/")+myFileInfo.baseName();  // excludes any extension
    //create the dbf
    mDbfHandle = DBFCreate( myBaseString+".dbf" );
    //create an index field named after the base part of the file name

    DBFAddField( mDbfHandle, myBaseString+"_id", FTInteger, 11, 0 );
    //create a second arbitary attribute field
    DBFAddField( mDbfHandle, "Label", FTString, 255, 0 );
    //close the dbf
    DBFClose( mDbfHandle );
    //reopen
    mDbfHandle = DBFOpen( myBaseString+".dbf", "r+b" );
    //exit this fn giving
    return mDbfHandle;
}

SHPHandle ShapefileMaker::createShapeFile(QString theFileName )
{
    SHPHandle mShapeHandle;
    mShapeHandle = SHPCreate(theFileName, SHPT_POINT );
    //mShapeHandle = SHPCreate(theFileName, SHPT_ARC );
    return mShapeHandle;
}

void ShapefileMaker::writeDbfRecord (DBFHandle theDbfHandle, int theRecordIdInt, QString theLabel)
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

void ShapefileMaker::writePoint(SHPHandle theShapeHandle, int theRecordInt, double theXDouble, double theYDouble )
{
    SHPObject * myShapeObject;
    myShapeObject = SHPCreateObject( SHPT_POINT, theRecordInt, 0, NULL, NULL, 1, &theXDouble, &theYDouble, NULL, NULL );
    SHPWriteObject( theShapeHandle, -1, myShapeObject );
    SHPDestroyObject( myShapeObject );
}

void ShapefileMaker::writePoint(QString theLabel, double theXDouble, double theYDouble )
{
  writeDbfRecord (mDbfHandle, mCurrentRecInt,  theLabel);
  writePoint(mShapeHandle,mCurrentRecInt,theXDouble,theYDouble);
  mCurrentRecInt++;

}

void ShapefileMaker::writeLine(SHPHandle theShapeHandle, 
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
void ShapefileMaker::generateGraticule(DBFHandle theDbfHandle, SHPHandle theShapeHandle,double theXIntervalDouble,double theYIntervalDouble)
{
  
  int myRecordInt=0;
  //create the arrays for storing the coordinates
  double * myXArrayDouble;
  double * myYArrayDouble;
  myXArrayDouble = (double *)malloc(2 * sizeof(double));
  myYArrayDouble = (double *)malloc(2 * sizeof(double));
  
  //
  //Longitude loop
  //
  for (double myXDouble=-180.0;myXDouble <=180.0;myXDouble+=theXIntervalDouble)
  {
    
    myXArrayDouble[0]=myXDouble;
    myXArrayDouble[1]=myXDouble;
    myYArrayDouble[0]=-90.0;
    myYArrayDouble[1]=90.0;

    writeDbfRecord(theDbfHandle,myRecordInt,"testing");
    writeLine(theShapeHandle, myRecordInt, 2, myXArrayDouble, myYArrayDouble); //2=no vertices
    
    ++myRecordInt;
  }

  //
  //Latitude loop
  //
  for (double myYDouble=-90.0;myYDouble<=90.0;myYDouble+=theYIntervalDouble)
  {
    
    myXArrayDouble[0]=-180.0;
    myXArrayDouble[1]=180.0;
    myYArrayDouble[0]=myYDouble;
    myYArrayDouble[1]=myYDouble;
    
    writeDbfRecord(theDbfHandle,myRecordInt,"testing");
    writeLine(theShapeHandle, myRecordInt, 2, myXArrayDouble, myYArrayDouble); //2=no vertices

    ++myRecordInt;
  }
  
  delete myXArrayDouble;
  delete myYArrayDouble;
}

/* read from fp and generate point shapefile to theDbfHandle/theShapeHandle */
void ShapefileMaker::generatePoints (QString theInputFileName, DBFHandle theDbfHandle, SHPHandle theShapeHandle)
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




