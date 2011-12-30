/***************************************************************************
 *   Copyright (C) 2003 by Tim Sutton                                      *
 *   tim@linfiniti.com                                                     *
 *                                                                         *
 *   This is a plugin generated from the QGIS plugin template              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#include "heatmapgui.h"
#include "qgscontexthelp.h"
#include "qgsmaplayerregistry.h"
#include "qgsmaplayer.h"
#include "qgsvectorlayer.h"


//qt includes
#include <QComboBox>
#include <QFileDialog>
#include <QMap>
#include <QMapIterator>
#include <QSettings>


HeatmapGui::HeatmapGui( QWidget* parent, Qt::WFlags fl )
    : QDialog( parent, fl )
{
  setupUi( this );

  // Load all the available vector layers to mInputVectorCombo
  QMap<QString, QgsMapLayer*> mapLayers = QgsMapLayerRegistry::instance() -> mapLayers();
  QMapIterator<QString, QgsMapLayer*> layers(mapLayers);

  while(layers.hasNext())
  {
      layers.next();
      QgsVectorLayer* vl = qobject_cast<QgsVectorLayer *>(layers.value());
      if( vl )
      {
          mInputVectorCombo->insertItem(0, vl->name());
      }
  }

  enableOrDisableOkButton();


}

HeatmapGui::~HeatmapGui()
{
}

// Close the Dialog
void HeatmapGui::on_mButtonBox_rejected()
{
    reject();
}

// Browse button
void HeatmapGui::on_mBrowseButton_clicked()
{
    QSettings s;
    QString lastDir = s.value( "/Interpolation/lastOutputDir", "" ).toString();

    QString rasterFileName = QFileDialog::getSaveFileName( 0, tr( "Save Heatmap raster as ..."), lastDir );
    if( !rasterFileName.isEmpty() )
    {
        mOutputRasterLineEdit->setText( rasterFileName );
        QFileInfo rasterFileInfo( rasterFileName );
        QDir fileDir = rasterFileInfo.absoluteDir();
        if ( fileDir.exists() )
        {
            s.setValue( "/Interpolation/lastOutputDir", rasterFileInfo.absolutePath() );
        }
    }
    enableOrDisableOkButton();
}

// Ok button
void HeatmapGui::enableOrDisableOkButton()
{
    bool enabled = true;

    QString filename = mOutputRasterLineEdit->text();
    QFileInfo theFileInfo( filename );
    if( filename.isEmpty() || !theFileInfo.dir().exists() )
    {
        enabled = false;
    }

    mButtonBox->button( QDialogButtonBox::Ok )->setEnabled( enabled );
}

void HeatmapGui::on_mButtonBox_accepted()
{
    //emit signal to start working logic


    // and finally
    accept();
}

