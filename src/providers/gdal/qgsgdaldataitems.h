/***************************************************************************
    qgsgdaldataitems.h
    ---------------------
    begin                : October 2011
    copyright            : (C) 2011 by Martin Dobias
    email                : wonder.sk at gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSGDALDATAITEMS_H
#define QGSGDALDATAITEMS_H

#include "qgsdataitem.h"

class QgsGdalLayerItem : public QgsLayerItem
{
  private:

    QStringList sublayers;

  public:
    QgsGdalLayerItem( QgsDataItem* parent,
                      QString name, QString path, QString uri,
                      QStringList *theSublayers = NULL,
                      QString fileName = "" );
    ~QgsGdalLayerItem();

    bool setCrs( QgsCoordinateReferenceSystem crs );
    Capability capabilities();
    QString fileName() const { return ( mFileName == "" ) ? QgsLayerItem::fileName() : mFileName; }

    QVector<QgsDataItem*> createChildren();

  protected:
    QString mFileName; // used to identify filename in browser
};


#endif // QGSGDALDATAITEMS_H
