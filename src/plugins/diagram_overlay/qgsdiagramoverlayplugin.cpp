/***************************************************************************
                         qgsdiagramoverlayplugin.cpp  -  description
                         ---------------------------
    begin                : January 2007
    copyright            : (C) 2007 by Marco Hugentobler
    email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdiagramoverlayplugin.h"
#include "qgisinterface.h"
#include "qgslegendinterface.h"
#include "qgsdiagramdialog.h"
#include "qgsdiagramoverlay.h"
#include "qgsmaplayerregistry.h"
#include "qgsproject.h"
#include "qgsvectorlayer.h"
#include <QDomDocument>
#include <QFile>

#ifdef WIN32
#define QGISEXTERN extern "C" __declspec( dllexport )
#else
#define QGISEXTERN extern "C"
#endif

static const QString pluginName = QObject::tr( "Diagram Overlay" );
static const QString pluginDescription = QObject::tr( "A plugin for placing diagrams on vector layers" );
static const QString pluginVersion = QObject::tr( "Version 0.0.1" );

QgsDiagramOverlayPlugin::QgsDiagramOverlayPlugin( QgisInterface* iface ): QObject(), QgsVectorOverlayPlugin( pluginName, pluginDescription, pluginVersion ), mInterface( iface )
{
  if ( iface && iface->mainWindow() )
  {
    connect( iface->mainWindow(), SIGNAL( projectRead() ), this, SLOT( projectRead() ) );
  }
}

QgsDiagramOverlayPlugin::~QgsDiagramOverlayPlugin()
{

}

void QgsDiagramOverlayPlugin::projectRead()
{
  //for a test, print out the content of the project file
  QString projectFileName = QgsProject::instance()->fileName();

  if ( projectFileName.isEmpty() )
  {
    return;
  }

  QFile projectFile( projectFileName );
  QDomDocument projectDocument;
  if ( !projectDocument.setContent( &projectFile ) )
  {
    return;
  }

  //iterate over all maplayers
  QDomNodeList mapLayerList = projectDocument.documentElement().elementsByTagName( "maplayer" );
  QDomElement mapLayerElem;
  QDomNodeList overlayList;
  QDomElement overlayElem;
  QgsVectorLayer* currentVectorLayer = 0;
  QgsDiagramOverlay* newDiagramOverlay = 0;

  QDomNodeList idList;
  QString layerId;

  //iterate through all maplayer elements
  for ( int i = 0; i < mapLayerList.size(); ++i )
  {
    mapLayerElem = mapLayerList.at( i ).toElement();
    overlayList = mapLayerElem.elementsByTagName( "overlay" );

    //find out layer id
    idList = mapLayerElem.elementsByTagName( "id" );
    if ( idList.size() < 1 )
    {
      continue;
    }
    layerId = idList.at( 0 ).toElement().text();

    //iterate through all overlay elements
    for ( int j = 0; j < overlayList.size(); ++j )
    {
      overlayElem = overlayList.at( j ).toElement();
      if ( overlayElem.attribute( "type" ) == "diagram" )
      {
        //get a pointer to the vector layer which owns the diagram overlay (use QgsMapLayerRegistry)
        currentVectorLayer = qobject_cast<QgsVectorLayer *>( QgsMapLayerRegistry::instance()->mapLayer( layerId ) );
        if ( !currentVectorLayer )
        {
          continue;
        }

        //create an overlay object
        newDiagramOverlay = new QgsDiagramOverlay( currentVectorLayer );
        newDiagramOverlay->readXML( overlayElem );

        //add the overlay to the vector layer
        currentVectorLayer->addOverlay( newDiagramOverlay );

        //notify the legend that the layer legend needs to be changed
        if ( mInterface && mInterface->legendInterface() )
        {
          mInterface->legendInterface()->refreshLayerSymbology( currentVectorLayer );
        }
      }
    }
  }
}

QgsApplyDialog* QgsDiagramOverlayPlugin::dialog( QgsVectorLayer* vl ) const
{
  return new QgsDiagramDialog( vl );
}

QGISEXTERN QgisPlugin* classFactory( QgisInterface* iface )
{
  return new QgsDiagramOverlayPlugin( iface );
}

QGISEXTERN QString name()
{
  return pluginName;
}

QGISEXTERN QString description()
{
  return pluginDescription;
}

QGISEXTERN QString version()
{
  return pluginVersion;
}

QGISEXTERN int type()
{
  return QgisPlugin::VECTOR_OVERLAY;
}

QGISEXTERN void unload( QgisPlugin* theQgsDiagramOverlayPluginPointer )
{
  delete theQgsDiagramOverlayPluginPointer;
}
