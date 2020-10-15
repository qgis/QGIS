/***************************************************************************
                         qgspointclouddataitems.h
                         --------------------
    begin                : October 2020
    copyright            : (C) 2020 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPOINTCLOUDDATAITEMS_H
#define QGSPOINTCLOUDDATAITEMS_H

#include "qgsdataitem.h"
#include "qgsdataitemprovider.h"

///@cond PRIVATE
#define SIP_NO_FILE

class CORE_EXPORT QgsPointCloudLayerItem : public QgsLayerItem
{
    Q_OBJECT
  public:
    QgsPointCloudLayerItem( QgsDataItem *parent, const QString &name, const QString &path, const QString &uri );
    QString layerName() const override;

};

//! Provider for MDAL data items
class QgsPointCloudDataItemProvider : public QgsDataItemProvider
{
  public:
    QString name() override;

    int capabilities() const override;

    QgsDataItem *createDataItem( const QString &pathIn, QgsDataItem *parentItem ) override;
};

///@endcond

#endif // QGSPOINTCLOUDDATAITEMS_H



