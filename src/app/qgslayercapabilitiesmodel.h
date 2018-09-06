/***************************************************************************
                        qgslayercapabilitiesmodel.h
                        ----------------------------
   begin                : August 2018
   copyright            : (C) 2018 by Denis Rouzaud
   email                : denis@opengis.ch
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLAYERCAPABILITIESMODEL_H
#define QGSLAYERCAPABILITIESMODEL_H

#include <QObject>

#include "qgis_app.h"
#include "qgsproject.h"

class QgsLayerTreeModel;
class QgsLayerTreeNode;

class APP_EXPORT QgsLayerCapabilitiesModel : public QSortFilterProxyModel
{
    Q_OBJECT
  public:
    enum Columns
    {
      LayerColumn = 0,
      IdentifiableColumn,
      ReadOnlyColumn,
      SearchableColumn,
      RequiredColumn,
    };

    QgsLayerCapabilitiesModel( QgsProject *project, QObject *parent = nullptr );

    QgsLayerTreeModel *layerTreeModel() const;
    void setLayerTreeModel( QgsLayerTreeModel *layerTreeModel );
    bool identifiable( QgsMapLayer *layer ) const;
    bool removable( QgsMapLayer *layer ) const;
    bool readOnly( QgsMapLayer *layer ) const;
    bool searchable( QgsMapLayer *layer ) const;
    QgsMapLayer *mapLayer( const QModelIndex &idx ) const;
    void setShowSpatialLayersOnly( bool only );

    int columnCount( const QModelIndex &parent ) const override;
    QVariant headerData( int section, Qt::Orientation orientation, int role ) const override;
    Qt::ItemFlags flags( const QModelIndex &idx ) const override;
    QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const override;
    QModelIndex parent( const QModelIndex &child ) const override;
    QModelIndex sibling( int row, int column, const QModelIndex &idx ) const override;
    QVariant data( const QModelIndex &index, int role ) const override;
    bool setData( const QModelIndex &index, const QVariant &value, int role ) override;

  public slots:
    void setFilterText( const QString &filterText = QString() );
    void toggleSelectedItems( const QModelIndexList &checkedIndexes );

  protected:
    bool filterAcceptsRow( int sourceRow, const QModelIndex &sourceParent ) const override;

  private:
    bool nodeShown( QgsLayerTreeNode *node ) const;

    QString mFilterText;
    bool mShowSpatialLayersOnly = false;

    QHash<QgsMapLayer *, bool> mReadOnlyLayers;
    QHash<QgsMapLayer *, bool> mSearchableLayers;
    QHash<QgsMapLayer *, bool> mIdentifiableLayers;
    QHash<QgsMapLayer *, bool> mRemovableLayers;

    QgsLayerTreeModel *mLayerTreeModel = nullptr;
};

#endif // QGSLAYERCAPABILITIESMODEL_H
