/***************************************************************************
 *   Copyright (C) 2010 by Sergey Yakushev                                 *
 *   yakushevs@list.ru                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#include "exportdlg.h"
#include <qgscontexthelp.h>

//qt includes
#include <qlabel.h>
#include <qcombobox.h>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QUuid>
#include <qdialogbuttonbox.h>
#include <qmessagebox.h>


// Qgis includes
#include <qgsvectorlayer.h>
#include <qgsmaplayerregistry.h>
#include <qgsproviderregistry.h>
#include <qgsvectordataprovider.h>

//standard includes

RgExportDlg::RgExportDlg( QWidget* parent, Qt::WindowFlags fl )
    : QDialog( parent, fl )
{
  // create base widgets;
  setWindowTitle( tr( "Export feature" ) );
  QVBoxLayout *v = new QVBoxLayout( this );

  QHBoxLayout *h = new QHBoxLayout();
  QLabel *l = new QLabel( tr( "Select destination layer" ), this );
  h->addWidget( l );
  mcbLayers = new QComboBox( this );
  h->addWidget( mcbLayers );
  v->addLayout( h );

  QDialogButtonBox *bb = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this );
  connect( bb, SIGNAL( accepted() ), this, SLOT( on_buttonBox_accepted() ) );
  connect( bb, SIGNAL( rejected() ), this, SLOT( on_buttonBox_rejected() ) );
  v->addWidget( bb );

  //fill list of layers
  mcbLayers->insertItem( 0, tr( "New temporary layer" ), QVariant( "-1" ) );

  QMap<QString, QgsMapLayer*> mapLayers = QgsMapLayerRegistry::instance()->mapLayers();
  QMap<QString, QgsMapLayer*>::iterator layer_it = mapLayers.begin();

  for ( ; layer_it != mapLayers.end(); ++layer_it )
  {
    QgsVectorLayer* vl = dynamic_cast<QgsVectorLayer*>( layer_it.value() );
    if ( !vl )
      continue;
    if ( vl->geometryType() != QGis::Line )
      continue;
    mcbLayers->insertItem( 0, vl->name(), QVariant( vl->id() ) );
  }

} // RgSettingsDlg::RgSettingsDlg()

RgExportDlg::~RgExportDlg()
{
}

QgsVectorLayer* RgExportDlg::mapLayer() const
{
  QgsVectorLayer* myLayer = nullptr;
  QString layerId = mcbLayers->itemData( mcbLayers->currentIndex() ).toString();

  if ( layerId == "-1" )
  {
    // create a temporary layer
    myLayer = new QgsVectorLayer( QString( "LineString?crs=epsg:4326&memoryid=%1" ).arg( QUuid::createUuid().toString() ), "shortest path", "memory" );

    QgsVectorDataProvider *prov = myLayer->dataProvider();
    if ( !prov )
      return nullptr;

    QList<QgsField> attrList;
    attrList.append( QgsField( "length", QVariant::Double, "", 20, 8 ) );
    attrList.append( QgsField( "time", QVariant::Double, "", 20, 8 ) );
    prov->addAttributes( attrList );
    myLayer->updateFields();
    QList<QgsMapLayer *> myList;
    myList << myLayer;
    QgsMapLayerRegistry::instance()->addMapLayers( myList );
  }
  else
  {
    // return selected layer
    myLayer = dynamic_cast<QgsVectorLayer*>( QgsMapLayerRegistry::instance()->mapLayer( layerId ) );
  }

  return myLayer;
} // QgsVectorLayer* RgExportDlg::vectorLayer() const

void RgExportDlg::on_buttonBox_accepted()
{
  accept();
} // void RgExportDlg::on_buttonBox_accepted()

void RgExportDlg::on_buttonBox_rejected()
{
  reject();
}

void RgExportDlg::on_buttonBox_helpRequested()
{
  QgsContextHelp::run( metaObject()->className() );
}
