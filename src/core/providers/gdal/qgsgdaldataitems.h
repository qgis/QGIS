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
#include "qgsdataitemprovider.h"
#include "qgsdataprovider.h"
#include "qgis_sip.h"

#include "qgis_sip.h"

///@cond PRIVATE
#define SIP_NO_FILE

Q_NOWARN_DEPRECATED_PUSH  // setCrs is deprecated
class CORE_EXPORT QgsGdalLayerItem : public QgsLayerItem
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
Q_NOWARN_DEPRECATED_POP

//! Provider for GDAL root data item
class QgsGdalDataItemProvider : public QgsDataItemProvider
{
  public:
    QString name() override;

    int capabilities() const override;

    QgsDataItem *createDataItem( const QString &pathIn, QgsDataItem *parentItem ) override;
};

///@endcond
#endif // QGSGDALDATAITEMS_H
