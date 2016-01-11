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

class QgsVirtualLayerSourceSelect : public QDialog, private Ui::QgsVirtualLayerSourceSelectBase
{
    Q_OBJECT

  public:
    QgsVirtualLayerSourceSelect( QWidget * parent, Qt::WindowFlags fl = QgisGui::ModalDialogFlags );
    ~QgsVirtualLayerSourceSelect();

  private slots:
    void on_buttonBox_accepted();
    void onTestQuery();
    void onBrowseCRS();

  signals:
    void addVectorLayer( QString, QString, QString );
    void replaceVectorLayer( QString, QString, QString );

  private:
    QgsVirtualLayerDefinition getVirtualLayerDef();
    long mSrid;
};

#endif
