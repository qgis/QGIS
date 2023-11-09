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

// We don't want to expose this in the public API
#define SIP_NO_FILE

#include <QWidget>

#include "qgstextformat.h"
#include "qgis_gui.h"

class QgsImageSourceLineEdit;
class QLabel;
class QPushButton;
class QTreeView;
class QTreeWidget;
class QTreeWidgetItem;

class QgsFontButton;
class QgsCollapsibleGroupBox;
class QgsMapCanvas;
class QgsVectorLayer;

/**
 * \ingroup gui
 * \class QgsVectorLayerLegendWidget
 *
 * \brief A widget for configuration of options specific to vector layer's legend.
 */
class GUI_EXPORT QgsVectorLayerLegendWidget : public QWidget
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
    void labelLegendTreeWidgetItemDoubleClicked( QTreeWidgetItem *item, int column );

  private:
    void populateLegendTreeView( const QHash<QString, QString> &content );
    void populateLabelLegendTreeWidget();
    void applyLabelLegend();

  private:
    QTreeView *mLegendTreeView = nullptr;
    QgsFontButton *mTextOnSymbolFormatButton = nullptr;
    QPushButton *mTextOnSymbolFromExpressionButton = nullptr;
    QgsCollapsibleGroupBox *mTextOnSymbolGroupBox = nullptr;
    QLabel *mTextOnSymbolLabel = nullptr;
    QgsCollapsibleGroupBox *mLabelLegendGroupBox = nullptr;
    QTreeWidget *mLabelLegendTreeWidget = nullptr;
    QLabel *mPlaceholderImageLabel = nullptr;
    QgsImageSourceLineEdit *mImageSourceLineEdit = nullptr;

    QgsMapCanvas *mCanvas = nullptr;
    QgsVectorLayer *mLayer = nullptr;
};

#endif // QGSVECTORLAYERLEGENDWIDGET_H
