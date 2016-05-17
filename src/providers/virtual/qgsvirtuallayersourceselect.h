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
#include <qgisgui.h>
#include <qgsvirtuallayerdefinition.h>

class QgsVectorLayer;
class QMainWindow;
class QgsEmbeddedLayerSelectDialog;

class QgsVirtualLayerSourceSelect : public QDialog, private Ui::QgsVirtualLayerSourceSelectBase
{
    Q_OBJECT

  public:
    QgsVirtualLayerSourceSelect( QWidget * parent, Qt::WindowFlags fl = QgisGui::ModalDialogFlags );

  private slots:
    void on_buttonBox_accepted();
    void onTestQuery();
    void onBrowseCRS();
    void onLayerComboChanged( int );
    void onAddLayer();
    void onRemoveLayer();
    void onImportLayer();
    void onTableRowChanged( const QModelIndex& current, const QModelIndex& previous );

  signals:
    /** Source, name, provider */
    void addVectorLayer( QString, QString, QString );
    /** Old_id, source, name, provider */
    void replaceVectorLayer( QString, QString, QString, QString );

  private:
    QgsVirtualLayerDefinition getVirtualLayerDef();
    long mSrid;
    QStringList mProviderList;
    QgsEmbeddedLayerSelectDialog* mEmbeddedSelectionDialog;
    void addEmbeddedLayer( QString name, QString provider, QString encoding, QString source );
};

#endif
