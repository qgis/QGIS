/***************************************************************************
 *  qgsgeometryholescheck.cpp                                              *
 *  -------------------                                                    *
 *  copyright            : (C) 2014 by Sandro Mani / Sourcepole AG         *
 *  email                : smani@sourcepole.ch                             *
 ***************************************************************************/

#include "qgsgeometryholecheck.h"
#include "../utils/qgsfeaturepool.h"

void QgsGeometryHoleCheck::collectErrors( QList<QgsGeometryCheckError*>& errors, QStringList &/*messages*/, QAtomicInt* progressCounter , const QgsFeatureIds &ids ) const
{
  const QgsFeatureIds& featureIds = ids.isEmpty() ? mFeaturePool->getFeatureIds() : ids;
  Q_FOREACH ( const QgsFeatureId& featureid, featureIds )
  {
    if ( progressCounter ) progressCounter->fetchAndAddRelaxed( 1 );
    QgsFeature feature;
    if ( !mFeaturePool->get( featureid, feature ) )
    {
      continue;
    }

    QgsAbstractGeometryV2* geom = feature.geometry()->geometry();
    for ( int iPart = 0, nParts = geom->partCount(); iPart < nParts; ++iPart )
    {
      // Rings after the first one are interiors
      for ( int iRing = 1, nRings = geom->ringCount( iPart ); iRing < nRings; ++iRing )
      {
        errors.append( new QgsGeometryCheckError( this, featureid, QgsGeomUtils::getGeomPart( geom, iPart )->centroid(), QgsVertexId( iPart, iRing ) ) );
      }
    }
  }
}

void QgsGeometryHoleCheck::fixError( QgsGeometryCheckError* error, int method, int /*mergeAttributeIndex*/, Changes &changes ) const
{
  QgsFeature feature;
  if ( !mFeaturePool->get( error->featureId(), feature ) )
  {
    error->setObsolete();
    return;
  }
  QgsAbstractGeometryV2* geom = feature.geometry()->geometry();
  const QgsVertexId& vidx = error->vidx();

  // Check if ring still exists
  if ( !vidx.isValid( geom ) )
  {
    error->setObsolete();
    return;
  }

  // Fix error
  if ( method == NoChange )
  {
    error->setFixed( method );
  }
  else if ( method == RemoveHoles )
  {
    deleteFeatureGeometryRing( feature, vidx.part, vidx.ring, changes );
    error->setFixed( method );
  }
  else
  {
    error->setFixFailed( tr( "Unknown method" ) );
  }
}

const QStringList& QgsGeometryHoleCheck::getResolutionMethods() const
{
  static QStringList methods = QStringList() << tr( "Remove hole" ) << tr( "No action" );
  return methods;
}
