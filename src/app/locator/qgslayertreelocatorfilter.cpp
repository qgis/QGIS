/***************************************************************************
                        qgslayertreelocatorfilters.cpp
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

#include "qgslayertreelocatorfilter.h"
#include "qgslayertree.h"
#include "qgsproject.h"
#include "qgsiconutils.h"
#include "qgisapp.h"


QgsLayerTreeLocatorFilter::QgsLayerTreeLocatorFilter( QObject *parent )
  : QgsLocatorFilter( parent )
{}

QgsLayerTreeLocatorFilter *QgsLayerTreeLocatorFilter::clone() const
{
  return new QgsLayerTreeLocatorFilter();
}

void QgsLayerTreeLocatorFilter::fetchResults( const QString &string, const QgsLocatorContext &context, QgsFeedback * )
{
  QgsLayerTree *tree = QgsProject::instance()->layerTreeRoot();
  const QList<QgsLayerTreeLayer *> layers = tree->findLayers();
  for ( QgsLayerTreeLayer *layer : layers )
  {
    // if the layer is broken, don't include it in the results
    if ( ! layer->layer() )
      continue;

    QgsLocatorResult result;
    result.displayString = layer->layer()->name();
    result.setUserData( layer->layerId() );
    result.icon = QgsIconUtils::iconForLayer( layer->layer() );

    // return all the layers in case the string query is empty using an equal default score
    if ( context.usingPrefix && string.isEmpty() )
    {
      emit resultFetched( result );
      continue;
    }

    result.score = fuzzyScore( result.displayString, string );

    if ( result.score > 0 )
      emit resultFetched( result );
  }
}

void QgsLayerTreeLocatorFilter::triggerResult( const QgsLocatorResult &result )
{
  const QString layerId = result.userData().toString();
  QgsMapLayer *layer = QgsProject::instance()->mapLayer( layerId );
  QgisApp::instance()->setActiveLayer( layer );
}
