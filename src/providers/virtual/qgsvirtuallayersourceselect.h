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
#include <qgis.h>
#include "qgsguiutils.h"
#include <qgsvirtuallayerdefinition.h>
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

  private slots:
    void on_buttonBox_accepted();
    void onTestQuery();
    void onBrowseCRS();
    void onLayerComboChanged( int );
    void onAddLayer();
    void onRemoveLayer();
    void onImportLayer();
    void onTableRowChanged( const QModelIndex &current, const QModelIndex &previous );
    void updateLayersList();

  signals:
    //! Source, name, provider
    void addVectorLayer( QString, QString, QString );
    //! Old_id, source, name, provider
    void replaceVectorLayer( QString, QString, QString, QString );

  private:
    QgsVirtualLayerDefinition getVirtualLayerDef();
    long mSrid;
    QStringList mProviderList;
    QgsEmbeddedLayerSelectDialog *mEmbeddedSelectionDialog = nullptr;
    void addEmbeddedLayer( const QString &name, const QString &provider, const QString &encoding, const QString &source );
    QgsLayerTreeView *mTreeView  = nullptr;
};

#endif
