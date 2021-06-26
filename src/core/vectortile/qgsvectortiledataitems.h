/***************************************************************************
    qgsvectortiledataitems.h
    ---------------------
    begin                : March 2020
    copyright            : (C) 2020 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSVECTORTILEDATAITEMS_H
#define QGSVECTORTILEDATAITEMS_H

#include "qgsdataitem.h"
#include "qgsdataitemprovider.h"

///@cond PRIVATE
#define SIP_NO_FILE

//! Root item for XYZ tile layers
class CORE_EXPORT QgsVectorTileRootItem : public QgsConnectionsRootItem
{
    Q_OBJECT
  public:
    QgsVectorTileRootItem( QgsDataItem *parent, QString name, QString path );

    QVector<QgsDataItem *> createChildren() override;

    QVariant sortKey() const override { return 8; }

};

//! Item implementation for XYZ tile layers
class CORE_EXPORT QgsVectorTileLayerItem : public QgsLayerItem
{
    Q_OBJECT
  public:
    QgsVectorTileLayerItem( QgsDataItem *parent, QString name, QString path, const QString &encodedUri );

};


//! Provider for XYZ root data item
class QgsVectorTileDataItemProvider : public QgsDataItemProvider
{
  public:
    QString name() override;
    QString dataProviderKey() const override;
    int capabilities() const override;

    QgsDataItem *createDataItem( const QString &path, QgsDataItem *parentItem ) override;
};

///@endcond

#endif // QGSVECTORTILEDATAITEMS_H
