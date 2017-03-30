/***************************************************************************
    qgsgeometryduplicatecheck.cpp
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
#include "qgsgeometryduplicatecheck.h"
#include "qgsspatialindex.h"
#include "qgsgeometry.h"
#include "../utils/qgsfeaturepool.h"

void QgsGeometryDuplicateCheck::collectErrors( QList<QgsGeometryCheckError *> &errors, QStringList &messages, QAtomicInt *progressCounter, const QMap<QString, QgsFeatureIds> &ids ) const
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

      QList<QgsFeatureId> duplicates;
      QgsFeatureIds ids = getFeaturePool( layerId )->getIntersects( featureGeom.geometry()->boundingBox() );
      for ( QgsFeatureId id : ids )
      {
        // > : only report overlaps once
        if ( id >= featureid )
        {
          continue;
        }
        QgsFeature testFeature;
        if ( !getFeaturePool( layerId )->get( id, testFeature ) )
        {
          continue;
        }
        QString errMsg;
        QgsAbstractGeometry *diffGeom = geomEngine->symDifference( *testFeature.geometry().geometry(), &errMsg );
        if ( diffGeom && diffGeom->area() < QgsGeometryCheckPrecision::tolerance() )
        {
          duplicates.append( id );
        }
        else if ( !diffGeom )
        {
          messages.append( tr( "Duplicate check between features %1 and %2: %3" ).arg( feature.id() ).arg( testFeature.id() ).arg( errMsg ) );
        }
        delete diffGeom;
      }
      if ( !duplicates.isEmpty() )
      {
        std::sort( duplicates.begin(), duplicates.end() );
        errors.append( new QgsGeometryDuplicateCheckError( this, layerId, featureid, feature.geometry().geometry()->centroid(), duplicates ) );
      }
      delete geomEngine;
    }
  }
}

void QgsGeometryDuplicateCheck::fixError( QgsGeometryCheckError *error, int method, const QMap<QString, int> & /*mergeAttributeIndices*/, Changes &changes ) const
{
  QgsFeature feature;
  if ( !getFeaturePool( error->layerId() )->get( error->featureId(), feature ) )
  {
    error->setObsolete();
    return;
  }

  if ( method == NoChange )
  {
    error->setFixed( method );
  }
  else if ( method == RemoveDuplicates )
  {
    QgsGeometry featureGeom = feature.geometry();
    QgsGeometryEngine *geomEngine = QgsGeometryCheckerUtils::createGeomEngine( featureGeom.geometry(), QgsGeometryCheckPrecision::tolerance() );

    QgsGeometryDuplicateCheckError *duplicateError = static_cast<QgsGeometryDuplicateCheckError *>( error );
    for ( QgsFeatureId id : duplicateError->duplicates() )
    {
      QgsFeature testFeature;
      if ( !getFeaturePool( error->layerId() )->get( id, testFeature ) )
      {
        continue;
      }
      QgsAbstractGeometry *diffGeom = geomEngine->symDifference( testFeature.geometry().geometry() );
      if ( diffGeom && diffGeom->area() < QgsGeometryCheckPrecision::tolerance() )
      {
        getFeaturePool( error->layerId() )->deleteFeature( testFeature );
        changes[error->layerId()][id].append( Change( ChangeFeature, ChangeRemoved ) );
      }

      delete diffGeom;
    }
    delete geomEngine;
    error->setFixed( method );
  }
  else
  {
    error->setFixFailed( tr( "Unknown method" ) );
  }
}

QStringList QgsGeometryDuplicateCheck::getResolutionMethods() const
{
  static QStringList methods = QStringList()
                               << tr( "No action" )
                               << tr( "Remove duplicates" );
  return methods;
}
