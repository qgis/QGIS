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
#include <qmessagebox.h>
#include <qurl.h>
#include <qlineedit.h>
#include <qhttp.h>
#include <qregexp.h>
#include <qcursor.h> 
#include <qapplication.h> 

//plugin includes
#include "qgslocationcapturewidget.h"

//standard includes

PluginGui::PluginGui() : PluginGuiBase()
{

}

    PluginGui::PluginGui( QWidget* parent , const char* name , bool modal , WFlags fl  )
: PluginGuiBase( parent, name, modal, fl )
{
  mConnection = new QHttp();
  connect(mConnection, SIGNAL(done(bool)), this, SLOT(registrationDone(bool)));
}  
PluginGui::~PluginGui()
{
  delete mConnection;
  delete mHttp;
}

void PluginGui::pbnOK_clicked()
{
  QUrl myTargetUrl("http://community.qgis.org");
  QString myHost = myTargetUrl.host();
  int myPort = myTargetUrl.port();
  mHttp = new QHttp(myHost, myPort == -1 ? 80 : myPort);
  connect(mHttp, SIGNAL(requestStarted(int)), this, 
          SLOT(slotRequestStarted(int)));
  connect(mHttp, SIGNAL(requestFinished(int, bool)), this, 
          SLOT(slotRequestFinished(int, bool)));
  connect(mHttp, SIGNAL(done(bool)), this, SLOT(slotDone(bool)));
  connect(mHttp, SIGNAL(dataReadProgress(int,int)), this, 
          SLOT(slotDataReadProgress(int,int)));
  connect(mHttp, SIGNAL(responseHeaderReceived(const 
                  QHttpResponseHeader&)),
          this, SLOT(slotResponseHeaderReceived(const QHttpResponseHeader 
                  &)));
  QByteArray *myByteArray=new QByteArray();
  QString tmp = "name=shie";
  myByteArray->setRawData(tmp.latin1(), tmp.length());     
  mHttp->post("/qgis_users/index.php", *myByteArray);
  done(1);
} 
void PluginGui::pbnCancel_clicked()
{
  close(1);
}

void PluginGui::pbnGetCoords_clicked()
{
  QgsLocationCaptureWidget * myWidget = new QgsLocationCaptureWidget();
  myWidget->show();
  delete myWidget;
}



void PluginGui::submit()
{
  if (mConnection->state() == QHttp::HostLookup
          || mConnection->state() == QHttp::Connecting
          || mConnection->state() == QHttp::Sending
          || mConnection->state() == QHttp::Reading) {
    mConnection->abort();
  }

  if (leName->text().isEmpty() | 
      leEmail->text().isEmpty() |
      leLatitude->text().isEmpty() |
      leLongitude->text().isEmpty()   
      ) 
  {
    QMessageBox::critical(this, "Empty query",
            "Please type a submit string.",
            QMessageBox::Ok, QMessageBox::NoButton);
  } 
  else 
  {
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    mConnection->setHost("community.qgis.org");

    QHttpRequestHeader myHeader("POST", "/qgis_users/index.php");
    myHeader.setValue("Host", "community.qgis.org");
    myHeader.setContentType("application/x-www-form-urlmyEncoded");

    QString myEncodedName = leName->text();
    QUrl::encode(myEncodedName);
    QString myEncodedEmail = leEmail->text();
    QUrl::encode(myEncodedEmail);
    QString myEncodedImageUrl = leImageUrl->text();
    QUrl::encode(myEncodedImageUrl);
    QString myEncodedHomeUrl = leHomeUrl->text();
    QUrl::encode(myEncodedHomeUrl);
    QString myEncodedCountry = leCountry->text();
    QUrl::encode(myEncodedCountry);
    QString myEncodedPlaceDescription = lePlaceDescription->text();
    QUrl::encode(myEncodedPlaceDescription);
    QString myEncodedLatitude = leLatitude->text();
    QUrl::encode(myEncodedLatitude);
    QString myEncodedLongitude = leLongitude->text();
    QUrl::encode(myEncodedLongitude);
    QString myPostString = "formAction=save";
    myPostString += "&fldname=" + myEncodedName;
    myPostString += "&fldemail="+myEncodedEmail;
    myPostString += "&fldimage_url="+myEncodedImageUrl;
    myPostString += "&fldhome_url="+myEncodedHomeUrl;
    myPostString += "&fldcountry="+myEncodedCountry;
    myPostString += "&fldplace_description="+myEncodedPlaceDescription;
    myPostString += "&fldlatitude="+myEncodedLatitude;
    myPostString += "&fldlongitude="+myEncodedLongitude;

    mConnection->request(myHeader, myPostString.utf8());
  }

}

void PluginGui::submitDone( bool error )
{
  if (error) {
    QMessageBox::critical(this, "Error submiting",
            "An error occurred when submiting: "
            + mConnection->errorString(),
            QMessageBox::Ok, QMessageBox::NoButton);
  } else {
    QString result(mConnection->readAll());
    /*
     * TODO Implement checking of returned submission results page
     * 
     QRegExp rx("<a href=\"(http://lists\\.trolltech\\.com/qt-interest/.*)\">(.*)</a>");
     rx.setMinimal(TRUE);
     int pos = 0;
     while (pos >= 0) 
     {
     pos = rx.search(result, pos);
     if (pos > -1) 
     {
     pos += rx.matchedLength();
     new QListViewItem(myListView, rx.cap(2), rx.cap(1));
     }
     }
     */
    QApplication::restoreOverrideCursor();

  }
}
