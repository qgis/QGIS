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

class QgsGdalLayerItem : public QgsLayerItem
{
    Q_OBJECT

  private:

    QStringList mSublayers;

  public:
    QgsGdalLayerItem( QgsDataItem *parent,
                      const QString &name, const QString &path, const QString &uri,
                      QStringList *mSublayers = nullptr );

    bool setCrs( const QgsCoordinateReferenceSystem &crs ) override;

    QVector<QgsDataItem *> createChildren() override;

    QString layerName() const override;
};

#endif // QGSGDALDATAITEMS_H
