/***************************************************************************
      qgsgeopackageitemguiprovider.h
      -------------------
    begin                : June, 2019
    copyright            : (C) 2019 by Peter Petrik
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

#ifndef QGSGEOPACKAGEITEMGUIPROVIDER_H
#define QGSGEOPACKAGEITEMGUIPROVIDER_H

#include <QObject>
#include "qgsdataitemguiprovider.h"
#include "qgis_sip.h"

///@cond PRIVATE
#define SIP_NO_FILE

class QgsGeoPackageCollectionItem;
class QgsGeoPackageRootItem;

class QgsGeoPackageItemGuiProvider : public QObject, public QgsDataItemGuiProvider
{
    Q_OBJECT

  public:

    QgsGeoPackageItemGuiProvider() = default;

    QString name() override { return QStringLiteral( "geopackage_items" ); }

    void populateContextMenu( QgsDataItem *item, QMenu *menu,
                              const QList<QgsDataItem *> &selectedItems,
                              QgsDataItemGuiContext context ) override;

    bool rename( QgsDataItem *item, const QString &name, QgsDataItemGuiContext context ) override;
    bool deleteLayer( QgsLayerItem *layerItem, QgsDataItemGuiContext context ) override;

    bool acceptDrop( QgsDataItem *item, QgsDataItemGuiContext context ) override;
    bool handleDrop( QgsDataItem *item, QgsDataItemGuiContext context,
                     const QMimeData *data,
                     Qt::DropAction action ) override;
  private:
    bool handleDropGeopackage( QgsGeoPackageCollectionItem *item, const QMimeData *data, QgsDataItemGuiContext context );
    //! Compacts (VACUUM) a geopackage database
    void vacuumGeoPackageDbAction( const QString &path, const QString &name, QgsDataItemGuiContext context );
    void createDatabase( const QPointer< QgsGeoPackageRootItem > &item );

  protected slots:
    void renameVectorLayer( const QString &uri, const QString &key, const QStringList &tableNames,
                            const QPointer< QgsDataItem > &item, QgsDataItemGuiContext context );
};

///@endcond
#endif // QGSGEOPACKAGEITEMGUIPROVIDER_H
