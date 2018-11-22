/***************************************************************************
    testrenderergui.cpp
    ---------------------
    begin                : January 2012
    copyright            : (C) 2012 by Martin Dobias
    email                : wonder.sk at gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "testrenderergui.h"

#include <qgsapplication.h>
#include <qgsmapcanvas.h>
#include <qgsvectorlayer.h>
#include <qgsproject.h>
#include <qgsrendererpropertiesdialog.h>
#include <qgsstyle.h>

#include <QApplication>
#include <QToolBar>

TestRendererGUI::TestRendererGUI( QWidget *parent )
  : QMainWindow( parent )
{
  resize( 640, 480 );

  QToolBar *toolBar = addToolBar( "Actions" );
  toolBar->addAction( "set renderer", this, SLOT( setRenderer() ) );

  mMapCanvas = new QgsMapCanvas( this );
  mMapCanvas->setCanvasColor( Qt::white );
  setCentralWidget( mMapCanvas );

  connect( QgsProject::instance(), SIGNAL( readProject( QDomDocument ) ), mMapCanvas, SLOT( readProject( QDomDocument ) ) );
}

void TestRendererGUI::loadLayers()
{
  // load just first vector layer
  QList<QgsMapLayer *> canvasLayers;
  foreach ( QgsMapLayer *layer, QgsProject::instance()->mapLayers().values() )
  {
    if ( layer->type() == QgsMapLayer::VectorLayer )
      canvasLayers << layer;
  }

  mMapCanvas->setLayers( canvasLayers );
}

void TestRendererGUI::setRenderer()
{
  QgsMapLayer *layer = mMapCanvas->layer( 0 );
  QVERIFY( layer );
  QVERIFY( layer->type() == QgsMapLayer::VectorLayer );
  QgsVectorLayer *vlayer = static_cast<QgsVectorLayer *>( layer );

  QgsRendererPropertiesDialog dlg( vlayer, QgsStyle::defaultStyle() );
  dlg.exec();

  mMapCanvas->refresh();
}

int main( int argc, char *argv[] )
{
  QApplication app( argc, argv );

  if ( argc < 2 )
  {
    qDebug( "Provide a project file name with at least one vector layer!" );
    return 1;
  }

  QgsApplication::init();
  QgsApplication::initQgis();

  TestRendererGUI gui;

  QString projectFileName( argv[1] );
  QgsProject::instance()->setFileName( projectFileName );
  bool res = QgsProject::instance()->read();
  if ( !res )
  {
    qDebug( "Failed to open project!" );
    return 1;
  }

  // the layers are in the registry - now load them!
  gui.loadLayers();

  gui.show();
  return app.exec();
}
