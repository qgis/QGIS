/***************************************************************************
  qgscustomlayerorderwidget.h
  --------------------------------------
  Date                 : May 2014
  Copyright            : (C) 2014 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCUSTOMLAYERORDERWIDGET_H
#define QGSCUSTOMLAYERORDERWIDGET_H

#include <QWidget>
#include <QAbstractListModel>

class CustomLayerOrderModel;
class QgsLayerTreeMapCanvasBridge;
class QgsLayerTreeNode;

class QCheckBox;
class QListView;

/**
  \ingroup gui
 * The QgsCustomLayerOrderWidget class provides a list box where the user can define
 * custom order for drawing of layers. It also features a checkbox for enabling
 * or disabling the custom order. Any changes made by the user are automatically
 * propagated to the assigned QgsLayerTreeMapCanvasBridge. Also, any updates
 * to the layer tree cause refresh of the list.
 *
 * @see QgsLayerTreeMapCanvasBridge
 * @note added in 2.4
 */
class GUI_EXPORT QgsCustomLayerOrderWidget : public QWidget
{
    Q_OBJECT
  public:
    explicit QgsCustomLayerOrderWidget( QgsLayerTreeMapCanvasBridge* bridge, QWidget *parent = nullptr );

  signals:

  protected slots:
    void bridgeHasCustomLayerOrderChanged( bool state );
    void bridgeCustomLayerOrderChanged( const QStringList& order );
    void nodeVisibilityChanged( QgsLayerTreeNode* node, Qt::CheckState state );

    void modelUpdated();

  protected:
    QgsLayerTreeMapCanvasBridge* mBridge;

    QCheckBox* mChkOverride;
    CustomLayerOrderModel* mModel;
    QListView* mView;
};


///@cond PRIVATE
class CustomLayerOrderModel : public QAbstractListModel
{
    Q_OBJECT

  public:
    CustomLayerOrderModel( QgsLayerTreeMapCanvasBridge* bridge, QObject* parent = nullptr );

    int rowCount( const QModelIndex & ) const override;

    QVariant data( const QModelIndex &index, int role ) const override;

    bool setData( const QModelIndex &index, const QVariant &value, int role ) override;

    Qt::ItemFlags flags( const QModelIndex &index ) const override;

    Qt::DropActions supportedDropActions() const override;

    QStringList mimeTypes() const override;

    QMimeData* mimeData( const QModelIndexList& indexes ) const override;

    bool dropMimeData( const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent ) override;

    bool removeRows( int row, int count, const QModelIndex& parent ) override;

    void refreshModel( const QStringList& order );

    QStringList order() const { return mOrder; }

    void updateLayerVisibility( const QString& layerId );

  protected:
    QgsLayerTreeMapCanvasBridge* mBridge;
    QStringList mOrder;
};
/// @endcond

#endif // QGSCUSTOMLAYERORDERWIDGET_H
