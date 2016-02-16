/***************************************************************************
    qgsgdaldataitems.h
    ---------------------
    begin                : October 2011
    copyright            : (C) 2011 by Martin Dobias
    email                : wonder dot sk at gmail dot com
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

Q_NOWARN_DEPRECATED_PUSH
class QgsGdalLayerItem : public QgsLayerItem
{
    Q_OBJECT

  private:

    QStringList sublayers;

  public:
    QgsGdalLayerItem( QgsDataItem* parent,
                      QString name, QString path, QString uri,
                      QStringList *theSublayers = nullptr );
    ~QgsGdalLayerItem();

    bool setCrs( QgsCoordinateReferenceSystem crs ) override;

    Q_DECL_DEPRECATED Capability capabilities() override;

    QVector<QgsDataItem*> createChildren() override;

    QString layerName() const override;
};
Q_NOWARN_DEPRECATED_POP

#endif // QGSGDALDATAITEMS_H
