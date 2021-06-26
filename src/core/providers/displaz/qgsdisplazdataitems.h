/***************************************************************************
    QgsDisplazdataitems.h
    ------------------
    begin                : April 2018
    copyright            : (C) 2018 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef HSLLCPDALDATAITEMS_H
#define HSLLCPDALDATAITEMS_H

#include "qgsdataitem.h"
#include "qgsproviderregistry.h"
#include "qgsdataitemprovider.h"
#include <QString>

class QgsDisplazLayerItem : public QgsLayerItem
{
    Q_OBJECT
  public:
    //! Ctor
    QgsDisplazLayerItem( QgsDataItem *parent, const QString &name, const QString &path, const QString &uri );
    QString layerName() const override;
};

//! Provider for MDAL data items
class QgsDisplazDataItemProvider : public QgsDataItemProvider
{
public:
	QString name() override;

	int capabilities() const override;

	QgsDataItem *createDataItem(const QString &pathIn, QgsDataItem *parentItem) override;
};

#endif // HSLLCPDALDATAITEMS_H
