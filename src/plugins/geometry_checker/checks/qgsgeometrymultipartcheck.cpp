/***************************************************************************
    qgsgeometrymultipartcheck.cpp
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

#include "qgsgeometrymultipartcheck.h"
#include "../utils/qgsfeaturepool.h"

void QgsGeometryMultipartCheck::collectErrors( QList<QgsGeometryCheckError*>& errors, QStringList &/*messages*/, QAtomicInt* progressCounter , const QgsFeatureIds &ids ) const
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

    QgsWKBTypes::Type type = geom->wkbType();
    if ( geom->partCount() == 1 && QgsWKBTypes::isMultiType( type ) )
    {
      errors.append( new QgsGeometryCheckError( this, featureid, geom->centroid() ) );
    }
  }
}

void QgsGeometryMultipartCheck::fixError( QgsGeometryCheckError* error, int method, int /*mergeAttributeIndex*/, Changes &changes ) const
{
  QgsFeature feature;
  if ( !mFeaturePool->get( error->featureId(), feature ) )
  {
    error->setObsolete();
    return;
  }
  QgsAbstractGeometryV2* geom = feature.geometry()->geometry();

  // Check if error still applies
  if ( geom->partCount() > 1 || !QgsWKBTypes::isMultiType( geom->wkbType() ) )
  {
    error->setObsolete();
    return;
  }

  // Fix error
  if ( method == NoChange )
  {
    error->setFixed( method );
  }
  else if ( method == ConvertToSingle )
  {
    feature.setGeometry( new QgsGeometry( QgsGeomUtils::getGeomPart( geom, 0 )->clone() ) );
    mFeaturePool->updateFeature( feature );
    error->setFixed( method );
    changes[feature.id()].append( Change( ChangeFeature, ChangeChanged ) );
  }
  else if ( method == RemoveObject )
  {
    mFeaturePool->deleteFeature( feature );
    error->setFixed( method );
    changes[feature.id()].append( Change( ChangeFeature, ChangeRemoved ) );
  }
  else
  {
    error->setFixFailed( tr( "Unknown method" ) );
  }
}

const QStringList& QgsGeometryMultipartCheck::getResolutionMethods() const
{
  static QStringList methods = QStringList()
                               << tr( "Convert to single part feature" )
                               << tr( "Delete feature" )
                               << tr( "No action" );
  return methods;
}
