#include "graticulecreator.h"
#include <stdio.h>
#include <qtextstream.h>
#include <iostream>
#include <qfileinfo.h>
#include <qstringlist.h>
GraticuleCreator::GraticuleCreator(QString theOutputFileName, float theXIntervalFloat, float theYIntervalFloat)
{
    std::cout << "GraticuleCreator constructor called with " << theOutputFileName
    << " for output file and " << theInputFileName << " for input file " << std::endl;
    DBFHandle myDbfHandle;		/* handle for dBase file */
    SHPHandle myShapeHandle;		/* handle for shape files .shx and .shp */
    /* Open and prepare output files */
    myDbfHandle = createDbf(theOutputFileName);
    myShapeHandle = createShapeFile(theOutputFileName);
    //test the write point routine....
    //generatePoints(theInputFileName,myDbfHandle,myShapeHandle);
    generateGraticule(myDbfHandle,myShapeHandle,theXIntervalFloat,theYIntervalFloat);
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
    QString myBaseString = myFileInfo.baseName();  // excludes any extension
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
    myShapeHandle = SHPCreate(theFileName, SHPT_POINT );
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

void GraticuleCreator::writePoint(SHPHandle theShapeHandle, int theRecordInt, double theXFloat, double theYFloat )
{
    SHPObject * myShapeObject;
    myShapeObject = SHPCreateObject( SHPT_POINT, theRecordInt, 0, NULL, NULL, 1, &theXFloat, &theYFloat, NULL, NULL );
    SHPWriteObject( theShapeHandle, -1, myShapeObject );
    SHPDestroyObject( myShapeObject );
}

void GraticuleCreator::WriteLine(SHPHandle theShapeHandle, 
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
void GraticuleCreator::generateGraticule(DBFHandle theDbfHandle, SHPHandle theShapeHandle,float theXIntervalFloat,float theYIntervalFloat)
{
  
  int myRecordInt=0;
  //
  //Longitude loop
  //
  for (float myXFloat=-180.0;myXFloat <=180.0;myXFloat+=theXIntervalFloat)
  {
    //create the arrays for storing the coordinates
    double * myXArrayDouble = NULL;
    double * myYArrayDouble = NULL;
    
    myXArrayDouble = realloc(myXArrayDouble, 2 * sizeof(double));
    myYArrayDouble = realloc(myYArrayDouble, 2 * sizeof(double));
    
    myXArrayDouble[0]=myXFloat;
    myXArrayDouble[1]=myXFloat;
    myYArrayDouble[0]=-90.0;
    myYArrayDouble[1]=90.0;

    WriteDbf(theDbfHandle,myRecordInt,myRecordInt;"testing");
    WriteLine(theShpHandle, myRecordInt, 2, myXArrayDouble, myYArrayDouble); //2=no vertices
    
    delete myXArrayDouble;
    delete myYArrayDouble;
  }

  //
  //Latitude loop
  //
  for (float myYFloat=-90.0;myYFloat<=90.0;myYFloat+=theYIntervalFloat)
  {
    //create the arrays for storing the coordinates
    double * myXArrayDouble = NULL;
    double * myYArrayDouble = NULL;
    
    myXArrayDouble = realloc(myXArrayDouble, 2 * sizeof(double));
    myYArrayDouble = realloc(myYArrayDouble, 2 * sizeof(double));

    myXArrayDouble[0]=-180.0;
    myXArrayDouble[1]=180.0;
    myYArrayDouble[0]=myYFloat;
    myYArrayDouble[1]=myYFloat;
    
    WriteDbf(theDbfHandle,myRecordInt,myRecordInt;"testing");
    WriteLine(theShpHandle, myRecordInt, 2, myXArrayDouble, myYArrayDouble); //2=no vertices

    delete myXArrayDouble;
    delete myYArrayDouble;
  }
  
  ++myRecordInt;
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
                float x=myLongQString.toFloat();
                float y=myLatQString.toFloat();
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




