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
