#include "waypointtoshape.h"
#include <stdio.h>
#include <qtextstream.h>
#include <iostream>
#include <qfileinfo.h>
#include <qstringlist.h>
WayPointToShape::WayPointToShape(QString theOutputFileName, QString theInputFileName )
{
    DBFHandle myDbfHandle;		/* handle for dBase file */
    SHPHandle myShapeHandle;		/* handle for shape files .shx and .shp */
    /* Open and prepare output files */
    myDbfHandle = createDbf(theOutputFileName);
    myShapeHandle = createShapeFile(theOutputFileName);
    std::cout << "WayPointToShape constructor called with " << theOutputFileName
    << " for output file and " << theInputFileName << " for input file " << std::endl;
    //test the write point routine....
    generatePoints(theInputFileName,myDbfHandle,myShapeHandle);

    DBFClose( myDbfHandle );
    SHPClose( myShapeHandle );
    return;
}

/* DbfName need not include the file extension. */
DBFHandle WayPointToShape::createDbf (QString theDbfName )
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

SHPHandle WayPointToShape::createShapeFile(QString theFileName )
{
    SHPHandle myShapeHandle;
    myShapeHandle = SHPCreate(theFileName, SHPT_POINT );
    return myShapeHandle;
}

void WayPointToShape::writeDbfRecord (DBFHandle theDbfHandle, int theRecordIdInt, QString theLabel)
{

  
    std::cerr << "writeDbfRecord : " << theRecordIdInt << " - " << theLabel;
    if (! DBFWriteIntegerAttribute(theDbfHandle, theRecordIdInt, 0, theRecordIdInt))
    {
        std::cerr <<  "DBFWriteIntegerAttribute failed. : " <<  theRecordIdInt << " - " << theRecordIdInt <<endl;

        //exit(ERR_DBFWRITEINTEGERATTRIBUTE);
    }
    if (theLabel != NULL)
    {
      if (! DBFWriteStringAttribute(theDbfHandle, theRecordIdInt, 1, theLabel))
      {
        std::cerr <<  "DBFWriteStringAttribute failed. : " <<  theRecordIdInt << " - " << theLabel <<endl;
        //exit(ERR_DBFWRITEINTEGERATTRIBUTE);
      }
      std::cerr << " - OK! " << std::endl;
    }
    //DBFWriteIntegerAttribute(theDbfHandle, theRecordIdInt, 0, theRecordIdInt);
    //DBFWriteStringAttribute(theDbfHandle, theRecordIdInt, 1, theLabel);
}

void WayPointToShape::writePoint(SHPHandle theShapeHandle, int theRecordInt, double theXFloat, double theYFloat )
{
    SHPObject * myShapeObject;
    myShapeObject = SHPCreateObject( SHPT_POINT, theRecordInt, 0, NULL, NULL, 1, &theXFloat, &theYFloat, NULL, NULL );
    SHPWriteObject( theShapeHandle, -1, myShapeObject );
    SHPDestroyObject( myShapeObject );
}
/* read from fp and generate point shapefile to theDbfHandle/theShapeHandle */
void WayPointToShape::generatePoints (QString theInputFileName, DBFHandle theDbfHandle, SHPHandle theShapeHandle)
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




