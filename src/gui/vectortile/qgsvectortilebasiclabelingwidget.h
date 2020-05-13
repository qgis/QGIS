/***************************************************************************
  qgsvectortilebasiclabelingwidget.h
  --------------------------------------
  Date                 : May 2020
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

#ifndef QGSVECTORTILEBASICLABELINGWIDGET_H
#define QGSVECTORTILEBASICLABELINGWIDGET_H

#include "qgsmaplayerconfigwidget.h"

#include "ui_qgsvectortilebasiclabelingwidget.h"

#include "qgswkbtypes.h"

#include <memory>

///@cond PRIVATE
#define SIP_NO_FILE

class QgsVectorTileBasicLabeling;
class QgsVectorTileBasicLabelingListModel;
class QgsVectorTileLayer;
class QgsMapCanvas;
class QgsMessageBar;

/**
 * \ingroup gui
 * Styling widget for basic labling of vector tile layer
 *
 * \since QGIS 3.14
 */
class GUI_EXPORT QgsVectorTileBasicLabelingWidget : public QgsMapLayerConfigWidget, private Ui::QgsVectorTileBasicLabelingWidget
{
    Q_OBJECT
  public:
    QgsVectorTileBasicLabelingWidget( QgsVectorTileLayer *layer, QgsMapCanvas *canvas, QgsMessageBar *messageBar, QWidget *parent = nullptr );
    ~QgsVectorTileBasicLabelingWidget() override;

  public slots:
    //! Applies the settings made in the dialog
    void apply() override;

  private slots:
    void addStyle( QgsWkbTypes::GeometryType geomType );
    //void addStyle();
    void editStyle();
    void editStyleAtIndex( const QModelIndex &index );
    void removeStyle();

    void updateLabelingFromWidget();

  private:
    QgsVectorTileLayer *mVTLayer = nullptr;
    std::unique_ptr<QgsVectorTileBasicLabeling> mLabeling;
    QgsVectorTileBasicLabelingListModel *mModel = nullptr;
    QgsMessageBar *mMessageBar = nullptr;
};


class QgsPalLayerSettings;
class QgsVectorLayer;
class QgsSymbolWidgetContext;
class QgsLabelingGui;

/**
 * \ingroup gui
 * Helper widget class that wraps QgsLabelingGui into a QgsPanelWidget
 *
 * \since QGIS 3.14
 */
class QgsLabelingPanelWidget : public QgsPanelWidget
{
    Q_OBJECT
  public:
    QgsLabelingPanelWidget( const QgsPalLayerSettings &labelSettings, QgsVectorLayer *vectorLayer, QgsMapCanvas *mapCanvas, QWidget *parent = nullptr );

    void setDockMode( bool dockMode ) override;

    void setContext( const QgsSymbolWidgetContext &context );
    QgsPalLayerSettings labelSettings();

  private:
    QgsLabelingGui *mLabelingGui = nullptr;
};

///@endcond

#endif // QGSVECTORTILEBASICLABELINGWIDGET_H
