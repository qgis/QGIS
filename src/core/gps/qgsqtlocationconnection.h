/***************************************************************************
                          QgsQtLocationConnection.h  -  description
                          -------------------
    begin                : December 7th, 2011
    copyright            : (C) 2011 by Marco Bernasocchi, Bernawebdesign.ch
    email                : marco at bernawebdesign dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSQTLOCATIONCONNECTION_H
#define QGSQTLOCATIONCONNECTION_H

#include "qgsgpsconnection.h"
#include <QtLocation/QGeoPositionInfoSource>
using namespace QtMobility;

class CORE_EXPORT QgsQtLocationConnection: public QgsGPSConnection
{
    Q_OBJECT
  public:
    QgsQtLocationConnection();
    ~QgsQtLocationConnection();

  protected slots:
    /**Parse available data source content*/
    void parseData( );

  private slots:
    void connected();
    void error();

  private:
    QString mDevice;
    QGeoPositionInfoSource *source;

};

#endif // QGSQTLOCATIONCONNECTION_H
