/***************************************************************************
    qgsgeometrycontainedcheck.cpp
    ---------------------
    begin                : September 2015
    copyright            : (C) 2014 by Sandro Mani / Sourcepole AG
    email                : smani at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsgeometryengine.h"
#include "qgsgeometrycontainedcheck.h"
#include "../utils/qgsfeaturepool.h"

void QgsGeometryContainedCheck::collectErrors( QList<QgsGeometryCheckError *> &errors, QStringList &messages, QAtomicInt *progressCounter, const QMap<QString, QgsFeatureIds> &ids ) const
{
  QMap<QString, QgsFeatureIds> featureIds = ids.isEmpty() ? allLayerFeatureIds() : ids;
  for ( const QString &layerId : featureIds.keys() )
  {
    if ( !getCompatibility( getFeaturePool( layerId )->getLayer()->geometryType() ) )
    {
      continue;
    }
    for ( QgsFeatureId featureid : featureIds[layerId] )
    {
      if ( progressCounter ) progressCounter->fetchAndAddRelaxed( 1 );
      QgsFeature feature;
      if ( !getFeaturePool( layerId )->get( featureid, feature ) )
      {
        continue;
      }

      QgsGeometry featureGeom = feature.geometry();
      QgsGeometryEngine *geomEngine = QgsGeometryCheckerUtils::createGeomEngine( featureGeom.geometry(), QgsGeometryCheckPrecision::tolerance() );

      QgsFeatureIds ids = getFeaturePool( layerId )->getIntersects( featureGeom.geometry()->boundingBox() );
      for ( QgsFeatureId otherid : ids )
      {
        if ( otherid == featureid )
        {
          continue;
        }
        QgsFeature otherFeature;
        if ( !getFeaturePool( layerId )->get( otherid, otherFeature ) )
        {
          continue;
        }

        QString errMsg;
        if ( geomEngine->within( *otherFeature.geometry().geometry(), &errMsg ) )
        {
          errors.append( new QgsGeometryContainedCheckError( this, layerId, featureid, feature.geometry().geometry()->centroid(), otherid ) );
        }
        else if ( !errMsg.isEmpty() )
        {
          messages.append( tr( "Feature %1 within feature %2: %3" ).arg( feature.id() ).arg( otherFeature.id() ).arg( errMsg ) );
        }
      }
      delete geomEngine;
    }
  }
}

void QgsGeometryContainedCheck::fixError( QgsGeometryCheckError *error, int method, const QMap<QString, int> & /*mergeAttributeIndices*/, Changes &changes ) const
{
  QgsGeometryContainedCheckError *coverError = static_cast<QgsGeometryContainedCheckError *>( error );

  QgsFeature feature;
  QgsFeature otherFeature;
  if ( !getFeaturePool( error->layerId() )->get( error->featureId(), feature ) ||
       !getFeaturePool( error->layerId() )->get( coverError->otherId(), otherFeature ) )
  {
    error->setObsolete();
    return;
  }

  // Check if error still applies
  QgsGeometry featureGeom = feature.geometry();
  QgsGeometryEngine *geomEngine = QgsGeometryCheckerUtils::createGeomEngine( featureGeom.geometry(), QgsGeometryCheckPrecision::tolerance() );

  if ( !geomEngine->within( otherFeature.geometry().geometry() ) )
  {
    delete geomEngine;
    error->setObsolete();
    return;
  }
  delete geomEngine;

  // Fix error
  if ( method == NoChange )
  {
    error->setFixed( method );
  }
  else if ( method == Delete )
  {
    changes[error->layerId()][feature.id()].append( Change( ChangeFeature, ChangeRemoved ) );
    getFeaturePool( error->layerId() )->deleteFeature( feature );
    error->setFixed( method );
  }
  else
  {
    error->setFixFailed( tr( "Unknown method" ) );
  }
}

QStringList QgsGeometryContainedCheck::getResolutionMethods() const
{
  static QStringList methods = QStringList()
                               << tr( "Delete feature" )
                               << tr( "No action" );
  return methods;
}
