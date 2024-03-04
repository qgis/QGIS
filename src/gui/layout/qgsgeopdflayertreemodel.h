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
#include "qgsmaplayermodel.h"

class QgsMapCanvas;
class QgsProject;
class QgsVectorLayer;


/**
 * \ingroup gui
 * \brief Layer tree model for Geo-PDF layers
 *
 * \note This class is not a part of public API
 * \since QGIS 3.12
 */
class GUI_EXPORT QgsGeoPdfLayerTreeModel : public QgsMapLayerModel
{
    Q_OBJECT

  public:

    //! Model columns
    enum Columns
    {
      LayerColumn = 0, //!< Layer name
      GroupColumn, //!< PDF group
      InitiallyVisible, //!< Initial visibility state
      IncludeVectorAttributes //!< Vector attribute
    };

    //! constructor
    QgsGeoPdfLayerTreeModel( const QList< QgsMapLayer * > &layers, QObject *parent = nullptr );

    int columnCount( const QModelIndex &parent ) const override;
    QVariant headerData( int section, Qt::Orientation orientation, int role ) const override;
    Qt::ItemFlags flags( const QModelIndex &idx ) const override;
    QVariant data( const QModelIndex &index, int role ) const override;
    bool setData( const QModelIndex &index, const QVariant &value, int role ) override;

    /**
     * Checks (or unchecks) all rows and children from the specified \a parent index.
     */
    void checkAll( bool checked, const QModelIndex &parent = QModelIndex(), int column = IncludeVectorAttributes );

  private:

    QgsMapLayer *mapLayer( const QModelIndex &idx ) const;
    QgsVectorLayer *vectorLayer( const QModelIndex &idx ) const;
};


///@cond PRIVATE
class GUI_EXPORT QgsGeoPdfLayerFilteredTreeModel : public QSortFilterProxyModel
{
    Q_OBJECT
  public:

    QgsGeoPdfLayerFilteredTreeModel( QgsGeoPdfLayerTreeModel *sourceModel, QObject *parent = nullptr );

    bool filterAcceptsRow( int source_row, const QModelIndex &source_parent ) const override;

  private:
    QgsGeoPdfLayerTreeModel *mLayerTreeModel = nullptr;
};
///@endcond

#endif // QGSGEOPDFLAYERTREEMODEL_H
