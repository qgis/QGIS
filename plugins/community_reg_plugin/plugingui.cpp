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
//standard includes

PluginGui::PluginGui() : PluginGuiBase()
{

}

    PluginGui::PluginGui( QWidget* parent , const char* name , bool modal , WFlags fl  )
: PluginGuiBase( parent, name, modal, fl )
{

  connect(&mConnection, SIGNAL(done(bool)), this, SLOT(registrationDone(bool)));
}  
PluginGui::~PluginGui()
{
}

void PluginGui::pbnOK_clicked()
{
  QUrl targetURL("http://community.qgis.org";);
  QString host = targetURL.host();
  int port = targetURL.port();
  http = new QHttp(host, port == -1 ? 80 : port);
  connect(http, SIGNAL(requestStarted(int)), this, 
          SLOT(slotRequestStarted(int)));
  connect(http, SIGNAL(requestFinished(int, bool)), this, 
          SLOT(slotRequestFinished(int, bool)));
  connect(http, SIGNAL(done(bool)), this, SLOT(slotDone(bool)));
  connect(http, SIGNAL(dataReadProgress(int,int)), this, 
          SLOT(slotDataReadProgress(int,int)));
  connect(http, SIGNAL(responseHeaderReceived(const 
                  QHttpResponseHeader&)),
          this, SLOT(slotResponseHeaderReceived(const QHttpResponseHeader 
                  &)));
  QByteArray *array=new QByteArray();
  QString tmp = "name=shie";
  array->setRawData(tmp.latin1(), tmp.length());     
  http->post("/qgis_users/index.php", *array);
  done(1);
} 
void PluginGui::pbnCancel_clicked()
{
  close(1);
}
void PluginGui::search()
{
  if (connection.state() == QHttp::HostLookup
          || connection.state() == QHttp::Connecting
          || connection.state() == QHttp::Sending
          || connection.state() == QHttp::Reading) {
    connection.abort();
  }

  if (myLineEdit->text() == "") 
  {
    QMessageBox::critical(this, "Empty query",
            "Please type a search string.",
            QMessageBox::Ok, QMessageBox::NoButton);
  } 
  else 
  {
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    connection.setHost("community.qgis.org");

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
    myPostString += "&fldemail="=myEncodedEmail;
    myPostString += "&fldimage_url="=myEncoded;
    myPostString += "&fldhome_url="=myEncoded;
    myPostString += "&fldcountry="=myEncoded;
    myPostString += "&fldplace_description="=myEncoded;
    myPostString += "&fldlatitude="=myEncoded;
    myPostString += "&fldlongitude="=myEncoded;

    connection.request(myHeader, myPostString.utf8());
  }

}

void PluginGui::searchDone( bool error )
{
  if (error) {
    QMessageBox::critical(this, "Error searching",
            "An error occurred when searching: "
            + connection.errorString(),
            QMessageBox::Ok, QMessageBox::NoButton);
  } else {
    QString result(connection.readAll());

    QRegExp rx("<a href=\"(http://lists\\.trolltech\\.com/qt-interest/.*)\">(.*)</a>");
    rx.setMinimal(TRUE);
    int pos = 0;
    while (pos >= 0) {
      pos = rx.search(result, pos);
      if (pos > -1) {
        pos += rx.matchedLength();
        new QListViewItem(myListView, rx.cap(2), rx.cap(1));
      }
    }
  }

  QApplication::restoreOverrideCursor();

