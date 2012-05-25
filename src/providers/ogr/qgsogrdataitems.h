/***************************************************************************
                             qgsogrdataitems.h
                             -------------------
    begin                : 2011-04-01
    copyright            : (C) 2011 Radim Blazek
    email                : radim dot blazek at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSOGRDATAITEMS_H
#define QGSOGRDATAITEMS_H

#include "qgsdataitem.h"
#include "qgsogrprovider.h"

class QgsOgrLayerItem : public QgsLayerItem
{
    Q_OBJECT
  public:
    QgsOgrLayerItem( QgsDataItem* parent, QString name, QString path, QString uri, LayerType layerType, QString fileName = "" );
    ~QgsOgrLayerItem();

    bool setCrs( QgsCoordinateReferenceSystem crs );
    Capability capabilities();
    QString fileName() const { return ( mFileName == "" ) ? QgsLayerItem::fileName() : mFileName; }

  protected:
    QString mFileName; // used to identify filename in browser
};

class QgsOgrDataCollectionItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsOgrDataCollectionItem( QgsDataItem* parent, QString name, QString path, QString fileName = "" );
    ~QgsOgrDataCollectionItem();

    QVector<QgsDataItem*> createChildren();
    QString fileName() const { return ( mFileName == "" ) ? QgsDataCollectionItem::fileName() : mFileName; }

  protected:
    QString mFileName; // used to identify filename in browser
};

#endif // QGSOGRDATAITEMS_H
