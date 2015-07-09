/***************************************************************************
   qgsmaplayermodel.h
    --------------------------------------
   Date                 : 01.04.2014
   Copyright            : (C) 2014 Denis Rouzaud
   Email                : denis.rouzaud@gmail.com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef QGSMAPLAYERMODEL_H
#define QGSMAPLAYERMODEL_H

#include <QAbstractItemModel>
#include <QSortFilterProxyModel>
#include <QStringList>

class QgsMapLayer;


/**
 * @brief The QgsMapLayerModel class is a model to display layers in widgets.
 * @see QgsMapLayerProxyModel to sort and/filter the layers
 * @see QgsFieldModel to combine in with a field selector.
 * @note added in 2.3
 */
class GUI_EXPORT QgsMapLayerModel : public QAbstractItemModel
{
    Q_OBJECT
  public:
    static const int LayerIdRole;

    /**
     * @brief QgsMapLayerModel creates a model to display layers in widgets.
     */
    explicit QgsMapLayerModel( QObject *parent = 0 );
    /**
     * @brief QgsMapLayerModel creates a model to display a specific list of layers in a widget.
     */
    explicit QgsMapLayerModel( QList<QgsMapLayer*> layers, QObject *parent = 0 );

    /**
     * @brief setItemsCheckable defines if layers should be selectable in the widget
     */
    void setItemsCheckable( bool checkable );
    /**
     * @brief checkAll changes the checkstate for all the layers
     */
    void checkAll( Qt::CheckState checkState );
    /**
     * @brief layersChecked returns the list of layers which are checked (or unchecked)
     */
    QList<QgsMapLayer*> layersChecked( Qt::CheckState checkState = Qt::Checked );
    //! returns if the items can be checked or not
    bool itemsCheckable() const { return mItemCheckable; }

    /**
     * @brief indexFromLayer returns the model index for a given layer
     */
    QModelIndex indexFromLayer( QgsMapLayer* layer ) const;


  protected slots:
    void removeLayers( const QStringList layerIds );
    void addLayers( QList<QgsMapLayer*> layers );

  protected:
    QList<QgsMapLayer*> mLayers;
    QMap<QString, Qt::CheckState> mLayersChecked;
    bool mItemCheckable;

    // QAbstractItemModel interface
  public:
    QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const override;
    QModelIndex parent( const QModelIndex &child ) const override;
    int rowCount( const QModelIndex &parent ) const override;
    int columnCount( const QModelIndex &parent ) const override;
    QVariant data( const QModelIndex &index, int role ) const override;
    bool setData( const QModelIndex &index, const QVariant &value, int role ) override;
    Qt::ItemFlags flags( const QModelIndex &index ) const override;
};

#endif // QGSMAPLAYERMODEL_H
