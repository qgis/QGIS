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
#include "moc_qgsvectorlayertools.cpp"
#include "qgsfeaturerequest.h"
#include "qgslogger.h"
#include "qgsvectorlayerutils.h"
#include "qgsproject.h"


QgsVectorLayerTools::QgsVectorLayerTools()
  : QObject( nullptr )
{}

bool QgsVectorLayerTools::copyMoveFeatures( QgsVectorLayer *layer, QgsFeatureRequest &request, double dx, double dy, QString *errorMsg, const bool topologicalEditing, QgsVectorLayer *topologicalLayer, QString *childrenInfoMsg ) const
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
  QgsVectorLayerUtils::QgsDuplicateFeatureContext duplicateFeatureContext;
  QMap<QString, int> duplicateFeatureCount;
  while ( fi.nextFeature( f ) )
  {
    browsedFeatureCount++;

    if ( f.hasGeometry() )
    {
      QgsGeometry geom = f.geometry();
      geom.translate( dx, dy );
      f.setGeometry( geom );
    }

    QgsFeature newFeature;
    if ( mProject )
    {
      newFeature = QgsVectorLayerUtils::duplicateFeature( layer, f, mProject, duplicateFeatureContext );
      if ( !newFeature.isValid() )
      {
        couldNotWriteCount++;
        QgsDebugError( QStringLiteral( "Could not add new feature. Original copied feature id: %1" ).arg( f.id() ) );
      }
      else
      {
        fidList.insert( newFeature.id() );
      }

      const auto duplicateFeatureContextLayers = duplicateFeatureContext.layers();
      for ( QgsVectorLayer *chl : duplicateFeatureContextLayers )
      {
        if ( duplicateFeatureCount.contains( chl->name() ) )
        {
          duplicateFeatureCount[chl->name()] += duplicateFeatureContext.duplicatedFeatures( chl ).size();
        }
        else
        {
          duplicateFeatureCount[chl->name()] = duplicateFeatureContext.duplicatedFeatures( chl ).size();
        }
      }
    }
    else
    {
      newFeature = QgsVectorLayerUtils::createFeature( layer, f.geometry(), f.attributes().toMap() );
      if ( !layer->addFeature( newFeature ) )
      {
        couldNotWriteCount++;
        QgsDebugError( QStringLiteral( "Could not add new feature. Original copied feature id: %1" ).arg( f.id() ) );
      }
      else
      {
        fidList.insert( newFeature.id() );
      }
    }

    // translate
    if ( newFeature.hasGeometry() )
    {
      QgsGeometry geom = newFeature.geometry();
      if ( topologicalEditing )
      {
        if ( topologicalLayer )
        {
          topologicalLayer->addTopologicalPoints( geom );
        }
        layer->addTopologicalPoints( geom );
      }
    }
    else
    {
      noGeometryCount++;
    }
  }

  QString childrenInfo;
  for ( auto it = duplicateFeatureCount.constBegin(); it != duplicateFeatureCount.constEnd(); ++it )
  {
    childrenInfo += ( tr( "\n%n children on layer %1 duplicated", nullptr, it.value() ).arg( it.key() ) );
  }

  request = QgsFeatureRequest();
  request.setFilterFids( fidList );

  if ( childrenInfoMsg && !childrenInfo.isEmpty() )
  {
    childrenInfoMsg->append( childrenInfo );
  }

  if ( !couldNotWriteCount && !noGeometryCount )
  {
    res = true;
  }
  else if ( errorMsg )
  {
    errorMsg = new QString( tr( "Only %1 out of %2 features were copied." )
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

bool QgsVectorLayerTools::forceSuppressFormPopup() const
{
  return mForceSuppressFormPopup;
}

void QgsVectorLayerTools::setForceSuppressFormPopup( bool forceSuppressFormPopup )
{
  mForceSuppressFormPopup = forceSuppressFormPopup;
}
