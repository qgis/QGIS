/***************************************************************************
    qgsvectorlayertools.cpp
    ---------------------
    begin                : 09.11.2016
    copyright            : (C) 2016 by Denis Rouzaud
    email                : denis.rouzaud@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgsvectorlayer.h"
#include "qgsvectorlayertools.h"
#include "qgsfeaturerequest.h"
#include "qgslogger.h"
#include "qgsvectorlayerutils.h"


QgsVectorLayerTools::QgsVectorLayerTools()
  : QObject( nullptr )
{}

bool QgsVectorLayerTools::copyMoveFeatures( QgsVectorLayer *layer, QgsFeatureRequest &request, double dx, double dy, QString *errorMsg, const bool topologicalEditing, QgsVectorLayer *topologicalLayer ) const
{
  bool res = false;
  if ( !layer || !layer->isEditable() )
  {
    return false;
  }

  QgsFeatureIterator fi = layer->getFeatures( request );
  QgsFeature f;

  int browsedFeatureCount = 0;
  int couldNotWriteCount = 0;
  int noGeometryCount = 0;

  QgsFeatureIds fidList;

  while ( fi.nextFeature( f ) )
  {
    browsedFeatureCount++;

    QgsFeature newFeature = QgsVectorLayerUtils::createFeature( layer, f.geometry(), f.attributes().toMap() );

    // translate
    if ( newFeature.hasGeometry() )
    {
      QgsGeometry geom = newFeature.geometry();
      geom.translate( dx, dy );
      newFeature.setGeometry( geom );
#ifdef QGISDEBUG
      const QgsFeatureId fid = newFeature.id();
#endif
      // paste feature
      if ( !layer->addFeature( newFeature ) )
      {
        couldNotWriteCount++;
        QgsDebugMsg( QStringLiteral( "Could not add new feature. Original copied feature id: %1" ).arg( fid ) );
      }
      else
      {
        fidList.insert( newFeature.id() );
        if ( topologicalEditing )
        {
          if ( topologicalLayer )
          {
            topologicalLayer->addTopologicalPoints( geom );
          }
          layer->addTopologicalPoints( geom );
        }
      }
    }
    else
    {
      noGeometryCount++;
    }
  }

  request = QgsFeatureRequest();
  request.setFilterFids( fidList );

  if ( !couldNotWriteCount && !noGeometryCount )
  {
    res = true;
  }
  else if ( errorMsg )
  {
    errorMsg = new QString( QString( tr( "Only %1 out of %2 features were copied." ) )
                            .arg( browsedFeatureCount - couldNotWriteCount - noGeometryCount, browsedFeatureCount ) );
    if ( noGeometryCount )
    {
      errorMsg->append( " " );
      errorMsg->append( tr( "Some features have no geometry." ) );
    }
    if ( couldNotWriteCount )
    {
      errorMsg->append( " " );
      errorMsg->append( tr( "Some could not be created on the layer." ) );
    }
  }
  return res;
}
