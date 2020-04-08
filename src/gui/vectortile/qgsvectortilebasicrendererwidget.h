/***************************************************************************
  qgsvectortilebasicrendererwidget.h
  --------------------------------------
  Date                 : April 2020
  Copyright            : (C) 2020 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSVECTORTILEBASICRENDERERWIDGET_H
#define QGSVECTORTILEBASICRENDERERWIDGET_H

#include "qgsmaplayerconfigwidget.h"

#include "ui_qgsvectortilebasicrendererwidget.h"

#include "qgswkbtypes.h"

#include <memory>

///@cond PRIVATE
#define SIP_NO_FILE

class QgsVectorTileBasicRenderer;
class QgsVectorTileBasicRendererListModel;
class QgsVectorTileLayer;
class QgsMapCanvas;
class QgsMessageBar;

/**
 * \ingroup gui
 * Styling widget for basic renderer of vector tile layer
 *
 * \since QGIS 3.14
 */
class GUI_EXPORT QgsVectorTileBasicRendererWidget : public QgsMapLayerConfigWidget, private Ui::QgsVectorTileBasicRendererWidget
{
    Q_OBJECT
  public:
    QgsVectorTileBasicRendererWidget( QgsVectorTileLayer *layer, QgsMapCanvas *canvas, QgsMessageBar *messageBar, QWidget *parent = nullptr );
    ~QgsVectorTileBasicRendererWidget() override;

  public slots:
    //! Applies the settings made in the dialog
    void apply() override;

  private slots:
    void addStyle( QgsWkbTypes::GeometryType geomType );
    void editStyle();
    void editStyleAtIndex( const QModelIndex &index );
    void removeStyle();

    void updateSymbolsFromWidget();
    void cleanUpSymbolSelector( QgsPanelWidget *container );

  private:
    QgsVectorTileLayer *mVTLayer = nullptr;
    std::unique_ptr<QgsVectorTileBasicRenderer> mRenderer;
    QgsVectorTileBasicRendererListModel *mModel = nullptr;
    QgsMessageBar *mMessageBar = nullptr;
};

///@endcond

#endif // QGSVECTORTILEBASICRENDERERWIDGET_H
