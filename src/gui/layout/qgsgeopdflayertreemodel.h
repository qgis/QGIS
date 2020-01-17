/***************************************************************************
  qgsgeopdflayertreemodel.h
 ---------------------
 begin                : August 2019
 copyright            : (C) 2019 by Nyall Dawson
 email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSGEOPDFLAYERTREEMODEL_H
#define QGSGEOPDFLAYERTREEMODEL_H

// We don't want to expose this in the public API
#define SIP_NO_FILE

#include <QSortFilterProxyModel>
#include <QItemDelegate>

#include "qgis_gui.h"
#include "qgslayertreemodel.h"

class QgsMapCanvas;
class QgsProject;


class GUI_EXPORT QgsGeoPdfLayerTreeModel : public QgsLayerTreeModel
{
    Q_OBJECT

  public:
    QgsGeoPdfLayerTreeModel( QgsLayerTree *rootNode, QObject *parent = nullptr );

    int columnCount( const QModelIndex &parent ) const override;
    QVariant headerData( int section, Qt::Orientation orientation, int role ) const override;
    Qt::ItemFlags flags( const QModelIndex &idx ) const override;
    QVariant data( const QModelIndex &index, int role ) const override;
    bool setData( const QModelIndex &index, const QVariant &value, int role ) override;

  private:
    enum Columns
    {
      LayerColumn = 0,
      GroupColumn
    };

    QgsVectorLayer *vectorLayer( const QModelIndex &idx ) const;
};

#endif // QGSGEOPDFLAYERTREEMODEL_H
