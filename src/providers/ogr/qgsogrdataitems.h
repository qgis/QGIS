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

Q_NOWARN_DEPRECATED_PUSH
class QgsOgrLayerItem : public QgsLayerItem
{
    Q_OBJECT
  public:
    QgsOgrLayerItem( QgsDataItem* parent, QString name, QString path, QString uri, LayerType layerType );
    ~QgsOgrLayerItem();

    bool setCrs( QgsCoordinateReferenceSystem crs ) override;

    Q_DECL_DEPRECATED Capability capabilities() override;
    QString layerName() const override;
};
Q_NOWARN_DEPRECATED_POP

class QgsOgrDataCollectionItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsOgrDataCollectionItem( QgsDataItem* parent, QString name, QString path );
    ~QgsOgrDataCollectionItem();

    QVector<QgsDataItem*> createChildren() override;
};

#endif // QGSOGRDATAITEMS_H
