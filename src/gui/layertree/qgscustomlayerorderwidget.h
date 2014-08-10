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

class CustomLayerOrderModel;
class QgsLayerTreeMapCanvasBridge;
class QgsLayerTreeNode;

class QCheckBox;
class QListView;

/**
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
    explicit QgsCustomLayerOrderWidget( QgsLayerTreeMapCanvasBridge* bridge, QWidget *parent = 0 );

  signals:

  protected slots:
    void bridgeHasCustomLayerOrderChanged( bool override );
    void bridgeCustomLayerOrderChanged( const QStringList& order );
    void nodeVisibilityChanged( QgsLayerTreeNode* node, Qt::CheckState state );

    void modelUpdated();

  protected:
    QgsLayerTreeMapCanvasBridge* mBridge;

    QCheckBox* mChkOverride;
    CustomLayerOrderModel* mModel;
    QListView* mView;
};

#endif // QGSCUSTOMLAYERORDERWIDGET_H
