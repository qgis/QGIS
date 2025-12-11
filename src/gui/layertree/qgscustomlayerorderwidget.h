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

#include "qgis.h"
#include "qgis_gui.h"

#include <QAbstractListModel>
#include <QWidget>

class CustomLayerOrderModel;
class QgsLayerTreeMapCanvasBridge;
class QgsLayerTreeNode;
class QgsMapLayer;

class QCheckBox;
class QListView;

/**
 * \ingroup gui
 * \brief A list box where the user can define custom order for drawing of layers.
 *
 * It also features a checkbox for enabling
 * or disabling the custom order. Any changes made by the user are automatically
 * propagated to the assigned QgsLayerTreeMapCanvasBridge. Also, any updates
 * to the layer tree cause refresh of the list.
 *
 * \see QgsLayerTreeMapCanvasBridge
 */
class GUI_EXPORT QgsCustomLayerOrderWidget : public QWidget
{
    Q_OBJECT
  public:
    //! Constructor for QgsCustomLayerOrderWidget
    explicit QgsCustomLayerOrderWidget( QgsLayerTreeMapCanvasBridge *bridge, QWidget *parent SIP_TRANSFERTHIS = nullptr );

  signals:

  private slots:
    void bridgeHasCustomLayerOrderChanged( bool state );
    void bridgeCustomLayerOrderChanged();
    //! Slot triggered when the visibility of a node changes
    void nodeVisibilityChanged( QgsLayerTreeNode *node );

    void modelUpdated();

  private:
    QgsLayerTreeMapCanvasBridge *mBridge = nullptr;

    QCheckBox *mChkOverride = nullptr;
    CustomLayerOrderModel *mModel = nullptr;
    QListView *mView = nullptr;
};


#ifndef SIP_RUN
///@cond PRIVATE
class CustomLayerOrderModel : public QAbstractListModel
{
    Q_OBJECT

  public:
    CustomLayerOrderModel( QgsLayerTreeMapCanvasBridge *bridge, QObject *parent = nullptr );

    [[nodiscard]] int rowCount( const QModelIndex & ) const override;

    [[nodiscard]] QVariant data( const QModelIndex &index, int role ) const override;

    bool setData( const QModelIndex &index, const QVariant &value, int role ) override;

    [[nodiscard]] Qt::ItemFlags flags( const QModelIndex &index ) const override;

    [[nodiscard]] Qt::DropActions supportedDropActions() const override;

    [[nodiscard]] QStringList mimeTypes() const override;

    [[nodiscard]] QMimeData *mimeData( const QModelIndexList &indexes ) const override;

    bool dropMimeData( const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent ) override;

    bool removeRows( int row, int count, const QModelIndex &parent ) override;

    void refreshModel( const QList<QgsMapLayer *> &order );

    [[nodiscard]] QStringList order() const { return mOrder; }

    void updateLayerVisibility( const QString &layerId );

  protected:
    QgsLayerTreeMapCanvasBridge *mBridge = nullptr;
    QStringList mOrder;
};
/// @endcond
#endif

#endif // QGSCUSTOMLAYERORDERWIDGET_H
