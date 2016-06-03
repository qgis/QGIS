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

void QgsGeometryDuplicateCheck::collectErrors( QList<QgsGeometryCheckError*>& errors, QStringList &messages, QAtomicInt* progressCounter , const QgsFeatureIds &ids ) const
{
  const QgsFeatureIds& featureIds = ids.isEmpty() ? mFeaturePool->getFeatureIds() : ids;
  Q_FOREACH ( QgsFeatureId featureid, featureIds )
  {
    if ( progressCounter ) progressCounter->fetchAndAddRelaxed( 1 );
    QgsFeature feature;
    if ( !mFeaturePool->get( featureid, feature ) )
    {
      continue;
    }
    QgsGeometryEngine* geomEngine = QgsGeomUtils::createGeomEngine( feature.geometry()->geometry(), QgsGeometryCheckPrecision::tolerance() );

    QList<QgsFeatureId> duplicates;
    QgsFeatureIds ids = mFeaturePool->getIntersects( feature.geometry()->geometry()->boundingBox() );
    Q_FOREACH ( QgsFeatureId id, ids )
    {
      // > : only report overlaps once
      if ( id >= featureid )
      {
        continue;
      }
      QgsFeature testFeature;
      if ( !mFeaturePool->get( id, testFeature ) )
      {
        continue;
      }
      QString errMsg;
      QgsAbstractGeometryV2* diffGeom = geomEngine->symDifference( *testFeature.geometry()->geometry(), &errMsg );
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
      qSort( duplicates );
      errors.append( new QgsGeometryDuplicateCheckError( this, featureid, feature.geometry()->geometry()->centroid(), duplicates ) );
    }
    delete geomEngine;
  }
}

void QgsGeometryDuplicateCheck::fixError( QgsGeometryCheckError* error, int method, int /*mergeAttributeIndex*/, Changes &changes ) const
{
  QgsFeature feature;
  if ( !mFeaturePool->get( error->featureId(), feature ) )
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
    QgsGeometryEngine* geomEngine = QgsGeomUtils::createGeomEngine( feature.geometry()->geometry(), QgsGeometryCheckPrecision::tolerance() );

    QgsGeometryDuplicateCheckError* duplicateError = static_cast<QgsGeometryDuplicateCheckError*>( error );
    Q_FOREACH ( QgsFeatureId id, duplicateError->duplicates() )
    {
      QgsFeature testFeature;
      if ( !mFeaturePool->get( id, testFeature ) )
      {
        continue;
      }
      QgsAbstractGeometryV2* diffGeom = geomEngine->symDifference( *testFeature.geometry()->geometry() );
      if ( diffGeom && diffGeom->area() < QgsGeometryCheckPrecision::tolerance() )
      {
        mFeaturePool->deleteFeature( testFeature );
        changes[id].append( Change( ChangeFeature, ChangeRemoved ) );
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

const QStringList& QgsGeometryDuplicateCheck::getResolutionMethods() const
{
  static QStringList methods = QStringList()
                               << tr( "No action" )
                               << tr( "Remove duplicates" );
  return methods;
}
