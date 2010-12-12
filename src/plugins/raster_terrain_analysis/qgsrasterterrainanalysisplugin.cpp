/***************************************************************************
                          qgsrasterterrainanalysisplugin.cpp  -  description
                             -------------------
    begin                : August 6th, 2009
    copyright            : (C) 2009 by Marco Hugentobler
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

#include "qgsrasterterrainanalysisplugin.h"
#include "qgis.h"
#include "qgisinterface.h"
#include "qgsmaplayer.h"
#include "qgsmaplayerregistry.h"
#include "qgsaspectfilter.h"
#include "qgsslopefilter.h"
#include "qgsruggednessfilter.h"
#include "qgstotalcurvaturefilter.h"
#include "qgsrasterterrainanalysisdialog.h"
#include <QAction>
#include <QProgressDialog>

static const QString name_ = QObject::tr( "Raster Terrain Analysis plugin" );
static const QString description_ = QObject::tr( "A plugin for raster based terrain analysis" );
static const QString version_ = QObject::tr( "Version 0.1" );
static const QString icon_ = ":/raster/raster_terrain_icon.png";

QgsRasterTerrainAnalysisPlugin::QgsRasterTerrainAnalysisPlugin( QgisInterface* iface ): mIface( iface ), mAction( 0 )
{

}

QgsRasterTerrainAnalysisPlugin::~QgsRasterTerrainAnalysisPlugin()
{

}

void QgsRasterTerrainAnalysisPlugin::initGui()
{
  //create Action
  if ( mIface )
  {
    mAction = new QAction( QIcon( ":/raster/raster_terrain_icon.png" ), tr( "&Raster based terrain analysis..." ), 0 );
    QObject::connect( mAction, SIGNAL( triggered() ), this, SLOT( run() ) );
    mIface->addToolBarIcon( mAction );
    mIface->addPluginToMenu( tr( "&Raster based terrain analysis..." ), mAction );
  }
}

void QgsRasterTerrainAnalysisPlugin::unload()
{
  if ( mIface )
  {
    mIface->removePluginMenu( tr( "&Raster based terrain analysis..." ), mAction );
    mIface ->removeToolBarIcon( mAction );
    delete mAction;
  }
}

void QgsRasterTerrainAnalysisPlugin::run()
{
  QgsRasterTerrainAnalysisDialog d( mIface );
  if ( d.exec() == QDialog::Accepted )
  {
    //get input layer from id
    QString inputLayerId = d.selectedInputLayerId();
    QgsMapLayer* inputLayer = QgsMapLayerRegistry::instance()->mapLayer( inputLayerId );
    if ( !inputLayer )
    {
      return;
    }
    QString inputFilePath = inputLayer->source();

    QString analysisMethod = d.selectedAnalysisMethod();
    QString selectedFormat = d.selectedDriverKey();
    QString outputFile = d.selectedOuputFilePath();
    int returnValue;

    QgsNineCellFilter* filter = 0;
    if ( d.selectedAnalysisMethod() == tr( "Slope" ) )
    {
      filter = new QgsSlopeFilter( inputFilePath, outputFile, selectedFormat );
    }
    else if ( d.selectedAnalysisMethod() == tr( "Aspect" ) )
    {
      filter = new QgsAspectFilter( inputFilePath, outputFile, selectedFormat );
    }
    else if ( d.selectedAnalysisMethod() == tr( "Ruggedness index" ) )
    {
      filter = new QgsRuggednessFilter( inputFilePath, outputFile, selectedFormat );
    }
    else if ( d.selectedAnalysisMethod() == tr( "Total curvature" ) )
    {
      filter = new QgsTotalCurvatureFilter( inputFilePath, outputFile, selectedFormat );
    }

    if ( filter )
    {
      QProgressDialog p( tr( "Calculating " ) + d.selectedAnalysisMethod() + "...", tr( "Abort..." ), 0, 0 );
      p.setWindowModality( Qt::WindowModal );
      returnValue = filter->processRaster( &p );
      delete filter;
      if ( d.addLayerToProject() )
      {
        mIface->addRasterLayer( outputFile, d.selectedAnalysisMethod() );
      }
    }
  }
}

//global methods for the plugin manager
QGISEXTERN QgisPlugin* classFactory( QgisInterface * ifacePointer )
{
  return new QgsRasterTerrainAnalysisPlugin( ifacePointer );
}

QGISEXTERN QString name()
{
  return name_;
}

QGISEXTERN QString description()
{
  return description_;
}

QGISEXTERN QString version()
{
  return version_;
}

QGISEXTERN QString icon()
{
  return icon_;
}

QGISEXTERN int type()
{
  return QgisPlugin::UI;
}

QGISEXTERN void unload( QgisPlugin* pluginPointer )
{
  delete pluginPointer;
}


