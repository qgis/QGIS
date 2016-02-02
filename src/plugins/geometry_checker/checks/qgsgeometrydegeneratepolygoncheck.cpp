/***************************************************************************
 *  qgsgeometrydegeneratepolygoncheck.cpp                                  *
 *  -------------------                                                    *
 *  copyright            : (C) 2014 by Sandro Mani / Sourcepole AG         *
 *  email                : smani@sourcepole.ch                             *
 ***************************************************************************/

#include "qgsgeometrydegeneratepolygoncheck.h"
#include "../utils/qgsfeaturepool.h"

void QgsGeometryDegeneratePolygonCheck::collectErrors( QList<QgsGeometryCheckError*>& errors, QStringList &/*messages*/, QAtomicInt* progressCounter , const QgsFeatureIds &ids ) const
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
    QgsAbstractGeometryV2* geom = feature.geometry()->geometry();
    for ( int iPart = 0, nParts = geom->partCount(); iPart < nParts; ++iPart )
    {
      for ( int iRing = 0, nRings = geom->ringCount( iPart ); iRing < nRings; ++iRing )
      {
        if ( QgsGeomUtils::polyLineSize( geom, iPart, iRing ) < 3 )
        {
          errors.append( new QgsGeometryCheckError( this, featureid, geom->vertexAt( QgsVertexId( iPart, iRing, 0 ) ), QgsVertexId( iPart, iRing ) ) );
        }
      }
    }
  }
}

void QgsGeometryDegeneratePolygonCheck::fixError( QgsGeometryCheckError* error, int method, int /*mergeAttributeIndex*/, Changes &changes ) const
{
  QgsFeature feature;
  if ( !mFeaturePool->get( error->featureId(), feature ) )
  {
    error->setObsolete();
    return;
  }
  QgsAbstractGeometryV2* geom = feature.geometry()->geometry();
  QgsVertexId vidx = error->vidx();

  // Check if ring still exists
  if ( !vidx.isValid( geom ) )
  {
    error->setObsolete();
    return;
  }

  // Check if error still applies
  if ( QgsGeomUtils::polyLineSize( geom, vidx.part, vidx.ring ) >= 3 )
  {
    error->setObsolete();
    return;
  }

  // Fix error
  if ( method == NoChange )
  {
    error->setFixed( method );
  }
  else if ( method == DeleteRing )
  {
    deleteFeatureGeometryRing( feature, vidx.part, vidx.ring, changes );
    error->setFixed( method );
  }
  else
  {
    error->setFixFailed( tr( "Unknown method" ) );
  }
}

const QStringList& QgsGeometryDegeneratePolygonCheck::getResolutionMethods() const
{
  static QStringList methods = QStringList() << tr( "Delete feature" ) << tr( "No action" );
  return methods;
}
