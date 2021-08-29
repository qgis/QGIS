/***************************************************************************
           qgsvirtuallayersourceselect.cpp
      Virtual layer data provider selection widget

begin                : Jan 2016
copyright            : (C) 2016 Hugo Mercier, Oslandia
email                : hugo dot mercier at oslandia dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSVIRTUAL_LAYER_SOURCE_SELECT_H
#define QGSVIRTUAL_LAYER_SOURCE_SELECT_H

#include "ui_qgsvirtuallayersourceselectbase.h"
#include "qgis.h"
#include "qgshelp.h"
#include "qgsguiutils.h"
#include "qgsvirtuallayerdefinition.h"
#include "qgsproviderregistry.h"
#include "qgsabstractdatasourcewidget.h"

class QgsVectorLayer;
class QMainWindow;
class QgsEmbeddedLayerSelectDialog;
class QgsLayerTreeView;

class QgsVirtualLayerSourceSelect : public QgsAbstractDataSourceWidget, private Ui::QgsVirtualLayerSourceSelectBase
{
    Q_OBJECT

  public:
    QgsVirtualLayerSourceSelect( QWidget *parent = nullptr, Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags, QgsProviderRegistry::WidgetMode widgetMode = QgsProviderRegistry::WidgetMode::None );

  public slots:
    //! Triggered when the provider's connections need to be refreshed
    void refresh() override;
    void addButtonClicked() override;


  private slots:
    void testQuery();
    void browseCRS();
    void layerComboChanged( int );
    void addLayer();
    void removeLayer();
    void importLayer();
    void tableRowChanged( const QModelIndex &current, const QModelIndex &previous );
    void updateLayersList();
    void showHelp();


  private:
    QgsVirtualLayerDefinition getVirtualLayerDef();
    long mSrid = 0;
    QStringList mProviderList;
    QgsEmbeddedLayerSelectDialog *mEmbeddedSelectionDialog = nullptr;
    void addEmbeddedLayer( const QString &name, const QString &provider, const QString &encoding, const QString &source );
    QgsLayerTreeView *mTreeView  = nullptr;
    bool preFlight();
};

#endif
