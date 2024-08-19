/***************************************************************************
  qgslayertreefilterproxymodel.h

 ---------------------
 begin                : 05.06.2020
 copyright            : (C) 2020 by Denis Rouzaud
 email                : denis@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLAYERTREEFILTERPROXYMODEL_H
#define QGSLAYERTREEFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>

#include "qgis_core.h"
#include "qgsmaplayer.h"
#include "qgsmaplayerproxymodel.h"

class QgsLayerTreeModel;
class QgsLayerTreeNode;

/**
 * \ingroup core
 * \brief QgsLayerTreeFilterProxyModel is a sort filter proxy model to easily reproduce the legend/layer tree in a tree view.
 *
 * Layers are checkable by default.
 * Symbology nodes will not be shown.
 * Layers can be filtered by their type.
 *
 * For more complex use-cases, the model can be re-implemented to allow a different interaction or to add more columns.
 *
 * \since QGIS 3.14
 */
class CORE_EXPORT QgsLayerTreeFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
  public:
    //! Constructor
    QgsLayerTreeFilterProxyModel( QObject *parent = nullptr );

    /**
     * Initialize the list of checked layers.
     * \note If the model is re-implemented, this method might become useless
     */
    void setCheckedLayers( const QList<QgsMapLayer *> layers );

    //! Returns the checked layers
    QList<QgsMapLayer *> checkedLayers() const {return mCheckedLayers;}

    //! Returns the map layer at a given index
    QgsMapLayer *mapLayer( const QModelIndex &idx ) const;

    //! Rerturns the layer tree model
    QgsLayerTreeModel *layerTreeModel() const;
    //! Sets the layer tree model
    void setLayerTreeModel( QgsLayerTreeModel *layerTreeModel );

    /**
     * Returns if private layers are shown.
     *
     * Defaults to TRUE.
     *
     * \since QGIS 3.40
     */
    bool showPrivateLayers() const;

    /**
     * Determines if private layers are shown.
     *
     * Defaults to TRUE.
     *
     * \since QGIS 3.40
     */
    void setShowPrivateLayers( bool showPrivate );

    /**
     * Defines the type layers (vector, raster, etc) shown in the tree
     * If the list is empty, all types are shown.
     */
    void setFilters( Qgis::LayerFilters filters );

    virtual int columnCount( const QModelIndex &parent ) const override;
    virtual Qt::ItemFlags flags( const QModelIndex &idx ) const override;
    QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const override;
    QModelIndex parent( const QModelIndex &child ) const override;
    QModelIndex sibling( int row, int column, const QModelIndex &idx ) const override;
    virtual QVariant data( const QModelIndex &index, int role ) const override;
    virtual bool setData( const QModelIndex &index, const QVariant &value, int role ) override;

    /**
     * Returns TRUE if the specified \a node will be shown in the model.
     *
     * \since QGIS 3.40
     */
    bool nodeShown( QgsLayerTreeNode *node ) const;

  public slots:
    //! Sets the filter text to search for a layer in the tree
    virtual void setFilterText( const QString &filterText = QString() );

  protected:
    //! Returns if the layer is checked or not
    virtual bool isLayerChecked( QgsMapLayer *layer ) const;

    //! This will set if the layer is checked or not
    virtual void setLayerChecked( QgsMapLayer *layer, bool checked );

    bool filterAcceptsRow( int sourceRow, const QModelIndex &sourceParent ) const override;

  private:

    /**
     * Reimplement to determine which layer are shown in the model
     * \note even when reimplemented, the layer type filter and the filter text will respected.
     */
    virtual bool layerShown( QgsMapLayer *layer ) const;

    //! This will call the virtual method and takes care of emitting dataChanged signal
    void setLayerCheckedPrivate( QgsMapLayer *layer, bool checked );

    QgsLayerTreeModel *mLayerTreeModel = nullptr;
    QList<QgsMapLayer *> mCheckedLayers;
    QString mFilterText;
    // for compatibility this defaults to true
    bool mShowPrivateLayers = true;
    Qgis::LayerFilters mFilters = Qgis::LayerFilter::All;
};

#endif // QGSLAYERTREEFILTERPROXYMODEL_H
