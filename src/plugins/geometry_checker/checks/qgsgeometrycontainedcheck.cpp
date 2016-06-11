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

void QgsGeometryContainedCheck::collectErrors( QList<QgsGeometryCheckError*>& errors, QStringList &messages, QAtomicInt* progressCounter , const QgsFeatureIds &ids ) const
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

    QgsFeatureIds ids = mFeaturePool->getIntersects( feature.geometry()->geometry()->boundingBox() );
    Q_FOREACH ( QgsFeatureId otherid, ids )
    {
      if ( otherid == featureid )
      {
        continue;
      }
      QgsFeature otherFeature;
      if ( !mFeaturePool->get( otherid, otherFeature ) )
      {
        continue;
      }

      QString errMsg;
      if ( geomEngine->within( *otherFeature.geometry()->geometry(), &errMsg ) )
      {
        errors.append( new QgsGeometryContainedCheckError( this, featureid, feature.geometry()->geometry()->centroid(), otherid ) );
      }
      else if ( !errMsg.isEmpty() )
      {
        messages.append( tr( "Feature %1 within feature %2: %3" ).arg( feature.id() ).arg( otherFeature.id() ).arg( errMsg ) );
      }
    }
    delete geomEngine;
  }
}

void QgsGeometryContainedCheck::fixError( QgsGeometryCheckError* error, int method, int /*mergeAttributeIndex*/, Changes &changes ) const
{
  QgsGeometryContainedCheckError* coverError = static_cast<QgsGeometryContainedCheckError*>( error );

  QgsFeature feature;
  QgsFeature otherFeature;
  if ( !mFeaturePool->get( error->featureId(), feature ) ||
       !mFeaturePool->get( coverError->otherId(), otherFeature ) )
  {
    error->setObsolete();
    return;
  }

  // Check if error still applies
  QgsGeometryEngine* geomEngine = QgsGeomUtils::createGeomEngine( feature.geometry()->geometry(), QgsGeometryCheckPrecision::tolerance() );

  if ( !geomEngine->within( *otherFeature.geometry()->geometry() ) )
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
    changes[feature.id()].append( Change( ChangeFeature, ChangeRemoved ) );
    mFeaturePool->deleteFeature( feature );
    error->setFixed( method );
  }
  else
  {
    error->setFixFailed( tr( "Unknown method" ) );
  }
}

const QStringList& QgsGeometryContainedCheck::getResolutionMethods() const
{
  static QStringList methods = QStringList()
                               << tr( "Delete feature" )
                               << tr( "No action" );
  return methods;
}
