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
#ifndef QGSFISHINGSPOTSPLUGINGUI_H
#define QGSFISHINGSPOTSPLUGINGUI_H

#include <pluginguibase.uic.h>
//
// QT includes
//
#include <qstring.h>
#include <qobject.h>
#include <qurl.h>
#include <qhttp.h>
#include <qvaluevector.h>

/**
@author Tim Sutton
*/

//
//// Structs
//

/** \brief The Fishing spot struct stores data for one location listed on the http://www.marlinnut.com/latlon.shtml site.
  */
struct FishingSpot
{
  /** \brief The name of the fishing spot. */
  QString label;
  /** \brief The Latitude*/
  float latitude;
  /** \brief The Longitude */
  float longitude;

};

//
// Typedefs
//

/** \brief  A vector type for storing multiple typedefs.*/
typedef QValueVector<FishingSpot> FishingSpotsVector;

class QgsFishingSpotsPluginGui : public QgsFishingSpotsPluginGuiBase
{
Q_OBJECT
public:
    QgsFishingSpotsPluginGui();
    QgsFishingSpotsPluginGui( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~QgsFishingSpotsPluginGui();
    void pbnOK_clicked();
    void pbnCancel_clicked();
    void check();
    void pbnSelectFileName_clicked();
    
protected slots:

  void slotStateChanged(int state);
  void slotResponseHeaderReceived ( const QHttpResponseHeader & resp );
  void slotRequestFinished ( int id, bool error );
  void slotTimeOut();
  
signals:

  void finished();

signals:
   void drawRasterLayer(QString);
   void drawVectorLayer(QString,QString,QString);

private:

  QUrl mQUrl;
  int mTimeOutInt;
  QHttp mQhttp;
  int mSetHostIdInt;
  int mHeaderIdInt;
  int mGetIdInt;
  QString mRequestQString;
  QHttpResponseHeader mQHttpResponseHeader;
  FishingSpotsVector mFishingSpotsVector;

  void createShapefile(QString theShapefileName);
  void requestHeadFinished(int id);
  void requestGetFinished(int id);
  void finish();
};

#endif
