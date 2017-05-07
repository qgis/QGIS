/***************************************************************************
                         qgsinbuiltlocatorfilters.cpp
                         ----------------------------
    begin                : May 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgsinbuiltlocatorfilters.h"
#include "qgsproject.h"
#include "qgslayertree.h"
#include "qgsfeedback.h"
#include "qgisapp.h"
#include "qgsstringutils.h"
#include "qgsmaplayermodel.h"
#include "qgscomposition.h"
#include "qgslayoutmanager.h"

QgsLayerTreeLocatorFilter::QgsLayerTreeLocatorFilter( QObject *parent )
  : QgsLocatorFilter( parent )
{}

void QgsLayerTreeLocatorFilter::fetchResults( const QString &string, QgsFeedback *feedback )
{
  QgsLayerTree *tree = QgsProject::instance()->layerTreeRoot();
  QList<QgsLayerTreeLayer *> layers = tree->findLayers();
  Q_FOREACH ( QgsLayerTreeLayer *layer, layers )
  {
    if ( feedback->isCanceled() )
      return;

    if ( layer->layer() && layer->layer()->name().contains( string, Qt::CaseInsensitive ) )
    {
      QgsLocatorResult result;
      result.filter = this;
      result.displayString = layer->layer()->name();
      result.userData = layer->layerId();
      result.icon = QgsMapLayerModel::iconForLayer( layer->layer() );
      emit resultFetched( result );
    }
  }
}

void QgsLayerTreeLocatorFilter::triggerResult( const QgsLocatorResult &result )
{
  QString layerId = result.userData.toString();
  QgsMapLayer *layer = QgsProject::instance()->mapLayer( layerId );
  QgisApp::instance()->setActiveLayer( layer );
}

//
// QgsLayoutLocatorFilter
//

QgsLayoutLocatorFilter::QgsLayoutLocatorFilter( QObject *parent )
  : QgsLocatorFilter( parent )
{}

void QgsLayoutLocatorFilter::fetchResults( const QString &string, QgsFeedback *feedback )
{
  Q_FOREACH ( QgsComposition *composition, QgsProject::instance()->layoutManager()->compositions() )
  {
    if ( feedback->isCanceled() )
      return;

    if ( composition && composition->name().contains( string, Qt::CaseInsensitive ) )
    {
      QgsLocatorResult result;
      result.filter = this;
      result.displayString = composition->name();
      result.userData = composition->name();
      //result.icon = QgsMapLayerModel::iconForLayer( layer->layer() );
      emit resultFetched( result );
    }
  }
}

void QgsLayoutLocatorFilter::triggerResult( const QgsLocatorResult &result )
{
  QString layoutName = result.userData.toString();
  QgsComposition *composition = QgsProject::instance()->layoutManager()->compositionByName( layoutName );
  if ( !composition )
    return;

  QgisApp::instance()->openComposer( composition );
}



