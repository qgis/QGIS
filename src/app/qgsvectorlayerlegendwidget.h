/***************************************************************************
  qgsvectorlayerlegendwidget.h
  ---------------------
  Date                 : April 2018
  Copyright            : (C) 2018 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSVECTORLAYERLEGENDWIDGET_H
#define QGSVECTORLAYERLEGENDWIDGET_H

#include <QWidget>

#include "qgstextrenderer.h"

class QLabel;
class QPushButton;
class QTreeView;

class QgsFontButton;
class QgsCollapsibleGroupBox;
class QgsMapCanvas;
class QgsVectorLayer;

/**
 * A widget for configuration of options specific to vector layer's legend.
 */
class QgsVectorLayerLegendWidget : public QWidget
{
    Q_OBJECT
  public:
    explicit QgsVectorLayerLegendWidget( QWidget *parent = nullptr );

    //! Sets pointer to map canvas
    void setMapCanvas( QgsMapCanvas *canvas );

    //! Returns pointer to map canvas
    QgsMapCanvas *mapCanvas() const { return mCanvas; }

    //! Initialize widget with a map layer
    void setLayer( QgsVectorLayer *layer );

    //! Store changes made in the widget to the layer
    void applyToLayer();

  private slots:
    void labelsFromExpression();

  private:
    void populateLegendTreeView( const QHash<QString, QString> &content );

  private:
    QTreeView *mLegendTreeView = nullptr;
    QgsFontButton *mTextOnSymbolFormatButton = nullptr;
    QPushButton *mTextOnSymbolFromExpressionButton = nullptr;
    QgsCollapsibleGroupBox *mTextOnSymbolGroupBox = nullptr;
    QLabel *mTextOnSymbolLabel = nullptr;

    QgsMapCanvas *mCanvas = nullptr;
    QgsVectorLayer *mLayer = nullptr;
};

#endif // QGSVECTORLAYERLEGENDWIDGET_H
