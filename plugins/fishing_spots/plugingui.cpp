/***************************************************************************
 *   Copyright (C) 2003 by Tim Sutton                                      *
 *   tim@linfiniti.com                                                     *
 *                                                                         *
 *   This is a plugin generated from the QGIS plugin template              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#include "plugingui.h"

//qt includes
#include <qtimer.h>
#include <qregexp.h>
#include <qlineedit.h>
#include <qtextedit.h>
#include <qmessagebox.h>
#include <qtabwidget.h>
#include <qwidget.h>
#include <qstring.h>
#include <qfiledialog.h>
#include <qfileinfo.h>
//standard includes
#include <iostream>
#include <assert.h>
//shapefile making stuff
#include "shapefilemaker.h"
/** Alternate constructor for use when this plugin is going to be an app main widget. */
QgsFishingSpotsPluginGui::QgsFishingSpotsPluginGui() : QgsFishingSpotsPluginGuiBase()
{
  
  //Comment out the next line if you are debuggin stuff
  tabMain->hide();
  //tabMain->removePage(tabRegex);
}

QgsFishingSpotsPluginGui::QgsFishingSpotsPluginGui( QWidget* parent , const char* name , bool modal , WFlags fl  )
: QgsFishingSpotsPluginGuiBase( parent, name, modal, fl )
{
   
  //Comment out the next line if you are debuggin stuff
  tabMain->hide();
  //tabMain->removePage(tabRegex);
}  
QgsFishingSpotsPluginGui::~QgsFishingSpotsPluginGui()
{
}

void QgsFishingSpotsPluginGui::pbnOK_clicked()
{
  
  if (leOutputFileName->text()=="") 
  {
    QMessageBox::warning( this, "Fishing spots",
                "Output filename is invalid.\n"
                    "Fix it and try again" );
  }
  //
  // If you have a produced a raster layer using your plugin, you can ask qgis to 
  // add it to the view using:
  // emit drawRasterLayer(QString("layername"));
  // or for a vector layer
  // emit drawVectorLayer(QString("pathname"),QString("layername"),QString("provider name (either ogr or postgres"));
  //
  
  
  //Test the url snarfer
  mQUrl = QUrl("http://www.marlinnut.com/latlon.shtml");
  check(); 
  
  
} 

void QgsFishingSpotsPluginGui::pbnCancel_clicked()
{
 close(1);
}


//
// The following methods are adapted from some sample code I found here
//


void QgsFishingSpotsPluginGui::check()
{
  
  connect(&mQhttp, SIGNAL(stateChanged(int)), 
          this, SLOT(slotStateChanged(int)));
  connect(&mQhttp, SIGNAL(responseHeaderReceived(const QHttpResponseHeader &)),
          this, SLOT(slotResponseHeaderReceived(const QHttpResponseHeader &)));
  connect(&mQhttp, SIGNAL(requestFinished(int, bool)),
          this, SLOT(slotRequestFinished(int, bool)));

  std::cerr << "-------------------------------------------" << std::endl;
  std::cerr << "Web Snarfer running " <<  std::endl;
  std::cerr << "Setting host to: " << mQUrl.host() << std::endl;
  std::cerr << "Path in url is:  " << mQUrl.path() << std::endl;
  //std::cerr << "Query in url is:  " << mQUrl.query() << std::endl;
  mSetHostIdInt = mQhttp.setHost(mQUrl.host());
  std::cerr << "ID sethost: " << mSetHostIdInt << std::endl;

  if(!mQUrl.query().isNull() || mQUrl.hasRef()) 
    mRequestQString = mQUrl.toString();
  else mRequestQString = mQUrl.path();

  mHeaderIdInt = mQhttp.head(mRequestQString);
  std::cerr << "ID head: " << mHeaderIdInt << std::endl;
  
  //QTimer::singleShot( mTimeOutInt * 1000, this, SLOT(slotTimeOut()) );
}

void QgsFishingSpotsPluginGui::slotRequestFinished(int id, bool error)
{
  std::cerr << "slotRequestFinished: " << id << std::endl;
  if(error)
    {
      std::cerr << "Error: " << mQhttp.errorString() << std::endl;
      finish();
    }
  
  else if(id == mHeaderIdInt)
    {
      requestHeadFinished(id);
    }

  else if(id == mGetIdInt) 
    {
      requestGetFinished(id);
    }
}

void QgsFishingSpotsPluginGui::slotResponseHeaderReceived(const QHttpResponseHeader& resp)
{
  std::cerr << "slotResponseHeaderReceived " << std::endl;
  std::cerr << resp.toString() << std::endl;
  mQHttpResponseHeader = resp;
}

void QgsFishingSpotsPluginGui::slotTimeOut()
{
  std::cerr << "slotTimeout " << std::endl;
  if(mQhttp.state() == QHttp::Connecting) 
    {
      assert(mQhttp.currentId() == mHeaderIdInt || 
             mQhttp.currentId() == mGetIdInt);
      
      std::cerr << "Error: Time out" << std::endl;
      
      finish();
    }
}

void QgsFishingSpotsPluginGui::slotStateChanged(int state) 
{
  std::cerr << "slotStateChanged " << std::endl;
  std::cerr << "Current id: " << mQhttp.currentId() << std::endl;
  std::cerr << "State: " << state << std::endl;

  switch (state)
  {
      case QHttp::Unconnected: std::cout <<"Unconnected" << std::endl; break;
      case QHttp::HostLookup:  std::cout <<"Host Lookup" << std::endl; break;
      case QHttp::Connecting:  std::cout <<"Connecting"  << std::endl; break;
      case QHttp::Sending:     std::cout <<"Sending"     << std::endl; break;
      case QHttp::Reading:     std::cout <<"Reading"     << std::endl; break;
      case QHttp::Connected:   std::cout <<"Connected"   << std::endl; break;
      case QHttp::Closing:     std::cout <<"Closing"     << std::endl; break;
      default :  std::cout <<"Illegal state"             << std::endl; break;
  }

}

void QgsFishingSpotsPluginGui::requestHeadFinished(int id)
{
  std::cerr << "requestHeadFinished: " << id << std::endl;

  assert(id = mHeaderIdInt);
  assert(!mQhttp.hasPendingRequests());

  // 405 -> it might not support HEAD request's...
  if( (mQHttpResponseHeader.statusCode() == 200 || mQHttpResponseHeader.statusCode() == 405) && 
      (mQHttpResponseHeader.contentType() == "text/html" || 
       mQHttpResponseHeader.contentType() == "text/plain") )
    {
      mGetIdInt = mQhttp.get(mRequestQString);
      std::cerr << "ID GET: " << mGetIdInt << std::endl;
    }
  else if(mQHttpResponseHeader.statusCode() == 301 || 
          mQHttpResponseHeader.statusCode() == 302)
    {
      // ...
    }
  else
    {
      finish();
    }
}

//
// This is run once the web page has been retrieved.
//

void QgsFishingSpotsPluginGui::requestGetFinished(int id)
{
  std::cerr << "************************************* " << std::endl;
  std::cerr << "requestGetFinished: " << id << std::endl;

  assert(id == mGetIdInt);

  QString myPageQString(mQhttp.readAll());
  std::cerr << "Getting document contents : " << std::endl;
  //std::cerr << myPageQString << std::endl;
  assert(!myPageQString.isEmpty());

  //now we parse the file looking for lat long occurrences
  QRegExp myRecordQRegExp( leRecordRegex->text(),false,false ); // match using regex in our dialog
  QRegExp myLabelQRegExp( leLabelRegex->text(),false,false ); // match using regex in our dialog
  QRegExp myLatitudeQRegExp( leLatitudeRegex->text(),false,false ); // match using regex in our dialog
  QRegExp myLongitudeQRegExp( leLongitudeRegex->text(),false,false ); // match using regex in our dialog
  //
  //check each regex is valid and if not bail out
  //
  //
  // Record
  //
  if (!myRecordQRegExp.isValid()) 
  {
    QMessageBox::warning( this, "Fishing spots",
                "The record regex " + leRecordRegex->text() + " is invalid.\n"
                    "Fix it and try again" );
  }
  else
  {
    //make sure greedy matches are off so if you have a string like
    // <b>blah</b><b>blahblah</b>
    // and your regex is <b>.*</b>
    // non greedy match will return <b>blah</b>
    myRecordQRegExp.setMinimal(true);
    std::cerr << "Using record regex : " << leRecordRegex->text() << std::endl;
  }
  //
  // Label
  //
  if (!myLabelQRegExp.isValid()) 
  {
    QMessageBox::warning( this, "Fishing spots",
                "The label regex " + leRecordRegex->text() + " is invalid.\n"
                    "Fix it and try again" );
  }
  else
  {
    //make sure greedy matches are off so if you have a string like
    // <b>blah</b><b>blahblah</b>
    // and your regex is <b>.*</b>
    // non greedy match will return <b>blah</b>
    myLabelQRegExp.setMinimal(true);
    std::cerr << "Using label regex : " << leLabelRegex->text() << std::endl;
  }
  //
  // Latitude
  //
  if (!myLatitudeQRegExp.isValid()) 
  {
    QMessageBox::warning( this, "Fishing spots",
                "The latidude regex " + leLatitudeRegex->text() + " is invalid.\n"
                    "Fix it and try again" );
  }
  else
  {
    //make sure greedy matches are off so if you have a string like
    // <b>blah</b><b>blahblah</b>
    // and your regex is <b>.*</b>
    // non greedy match will return <b>blah</b>
    myLatitudeQRegExp.setMinimal(true);
    std::cerr << "Using latitude regex : " << leLatitudeRegex->text() << std::endl;
  }
  //
  // Longitude
  //
  if (!myLongitudeQRegExp.isValid()) 
  {
    QMessageBox::warning( this, "Fishing spots",
                "The regex " + leLongitudeRegex->text() + " is invalid.\n"
                    "Fix it and try again" );
  }
  else
  {
    //make sure greedy matches are off so if you have a string like
    // <b>blah</b><b>blahblah</b>
    // and your regex is <b>.*</b>
    // non greedy match will return <b>blah</b>
    myLongitudeQRegExp.setMinimal(true);
    std::cerr << "Using Longitude regex : " << leLongitudeRegex->text() << std::endl;
  }
  
  //
  // Main parsing loop
  //
  int myPosInt = 0;    // where we are in the string
  int myCountInt = 0;  // how many matches we find
  while ( myPosInt >= 0 ) 
  {
    myPosInt = myRecordQRegExp.search( myPageQString, myPosInt );
    if ( myPosInt >= 0 ) 
    {
      std::cerr << "************************************* " << std::endl;
      std::cerr << "Match found from pos " << myPosInt << " to " << myPosInt + myRecordQRegExp.matchedLength() << std::endl;
      QString myRecordQString =  myPageQString.mid(myPosInt,myRecordQRegExp.matchedLength());
      myPosInt += myRecordQRegExp.matchedLength(); //skip the length of the matched string
      myCountInt++;    // count the number of matches
      int myLastPosInt=0; //for keeping track of where we have searched up to within the record
      
      // now extract the location name
      
      myLastPosInt = myLabelQRegExp.search(myRecordQString, myLastPosInt);
      QString myLabelQString = myRecordQString.mid(myLastPosInt,myLabelQRegExp.matchedLength());
      myLabelQString.replace("<P>","");
      myLabelQString.replace("\n","");
      std::cerr << "\t label match from " << myLastPosInt << " : " << myLabelQRegExp.matchedLength() << std::endl; 
      myLastPosInt += myLabelQRegExp.matchedLength();
      
      // now extract the lat
      
      myLastPosInt = myLatitudeQRegExp.search(myRecordQString, myLastPosInt);
      QString myLatitudeQString = myRecordQString.mid(myLastPosInt,myLatitudeQRegExp.matchedLength()); 
      myLatitudeQString.replace("&#176; ",","); //replace the html degree symbol with a comma
      myLatitudeQString.replace("'",""); //replace the minutes symbol with nothing
      myLatitudeQString.replace("\n","");
      QStringList myQStringList = QStringList::split(QString(","),myLatitudeQString);
      float myDegreesFloat = myQStringList[0].toFloat();
      float myDecimalMinutesFloat = myQStringList[1].toFloat();
      float myLatitudeFloat = myDegreesFloat + (myDecimalMinutesFloat/60); //convert from degrees and decimal minutes to decimal degrees
      std::cerr << "\t long match from " << myLastPosInt << " : " << myLatitudeQRegExp.matchedLength() << std::endl; 
      myLastPosInt += myLatitudeQRegExp.matchedLength();
      
      // now extract the long
      
      myLastPosInt = myLongitudeQRegExp.search(myRecordQString, myLastPosInt);
      QString myLongitudeQString = myRecordQString.mid(myLastPosInt,myLongitudeQRegExp.matchedLength()); 
      myLongitudeQString.replace("&#176; ",","); //replace the html degree symbol with a comma
      myLongitudeQString.replace("'",""); //replace the minutes symbol with nothing
      myLongitudeQString.replace("\n","");
      myQStringList = QStringList::split(QString(","),myLongitudeQString);
      myDegreesFloat = myQStringList[0].toFloat();
      myDecimalMinutesFloat = myQStringList[1].toFloat();
      float myLongitudeFloat = myDegreesFloat + (myDecimalMinutesFloat/60); //convert from degrees and decimal minutes to decimal degrees
      std::cerr << "\t lat match from " << myLastPosInt << " : " << myLongitudeQRegExp.matchedLength() << std::endl; 
      myLastPosInt += myLongitudeQRegExp.matchedLength();
      
      // Show some info to stdout

      teResults->append( myLabelQString + ", " + QString::number(myLatitudeFloat)  + ", " + QString::number(myLongitudeFloat) + "\n");;

      //create a FishingSpot struct and add it to the vector
      FishingSpot myFishingSpot;
      myFishingSpot.label=myLabelQString;
      myFishingSpot.latitude=myLatitudeFloat;
      myFishingSpot.longitude=myLongitudeFloat;
      mFishingSpotsVector.push_back(myFishingSpot);
    }
  }
  std::cerr << "************************************* " << std::endl;
  //std::cerr << myPageQString << std::endl;
  std::cout << myCountInt << " records found" << std::endl;
  //std::cerr << myPageQString << std::endl;
  std::cerr << "************************************* " << std::endl;

  //
  // Now build the shapefile
  //
  createShapefile(leOutputFileName->text());
  finish();
}

void QgsFishingSpotsPluginGui::finish()
{
  std::cerr << "finish: " <<  std::endl;
/*
  Don't go back to slotRequestFinished
*/
  disconnect(&mQhttp, SIGNAL(requestFinished(int, bool)), 
             this, SLOT(slotRequestFinished(int, bool)));

  mQhttp.closeConnection();
  //close the dialog
  done(1);
}




void QgsFishingSpotsPluginGui::createShapefile(QString theShapefileName)
{

  ShapefileMaker * myShapefileMaker = new ShapefileMaker(theShapefileName);
  for (int myIteratorInt = 0; myIteratorInt < mFishingSpotsVector.size(); ++myIteratorInt)
  {
        FishingSpot myFishingSpot = mFishingSpotsVector[myIteratorInt];
        myShapefileMaker->writePoint(theShapefileName, static_cast<double>(0-myFishingSpot.longitude), static_cast<double>(myFishingSpot.latitude) );
        std::cerr << myFishingSpot.label << " :: " << myFishingSpot.latitude << " :: " << myFishingSpot.longitude << std::endl;
  }
  delete myShapefileMaker;
  //clear the spots list
  mFishingSpotsVector.clear(); 
  QFileInfo myFileInfo( theShapefileName );
  emit drawVectorLayer(myFileInfo.dirPath(),myFileInfo.baseName(),QString("ogr"));
}

void QgsFishingSpotsPluginGui::pbnSelectFileName_clicked()
{
  std::cout << " Gps File Importer Gui::pbnSelectOutputFile_clicked() " << std::endl;
  QString myOutputFileNameQString = QFileDialog::getSaveFileName(
          ".",
          "ESRI Shapefile (*.shp)",
          this,
          "save file dialog"
          "Choose a filename to save under" );
  leOutputFileName->setText(myOutputFileNameQString);

}
