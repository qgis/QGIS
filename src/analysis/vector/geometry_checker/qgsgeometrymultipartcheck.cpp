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

#include "qgsgeometrycheckcontext.h"
#include "qgsgeometrymultipartcheck.h"
#include "qgsfeaturepool.h"

QList<QgsSingleGeometryCheckError *> QgsGeometryMultipartCheck::processGeometry( const QgsGeometry &geometry ) const
{
  QList<QgsSingleGeometryCheckError *> errors;

  const QgsAbstractGeometry *geom = geometry.constGet();
  const QgsWkbTypes::Type type = geom->wkbType();
  if ( geom->partCount() == 1 && QgsWkbTypes::isMultiType( type ) )
  {
    errors.append( new QgsSingleGeometryCheckError( this, geometry, geometry ) );
  }
  return errors;
}

void QgsGeometryMultipartCheck::fixError( const QMap<QString, QgsFeaturePool *> &featurePools, QgsGeometryCheckError *error, int method, const QMap<QString, int> & /*mergeAttributeIndices*/, Changes &changes ) const
{
  QgsFeaturePool *featurePool = featurePools[ error->layerId() ];
  QgsFeature feature;
  if ( !featurePool->getFeature( error->featureId(), feature ) )
  {
    error->setObsolete();
    return;
  }
  const QgsGeometry featureGeom = feature.geometry();
  const QgsAbstractGeometry *geom = featureGeom.constGet();

  // Check if error still applies
  if ( geom->partCount() > 1 || !QgsWkbTypes::isMultiType( geom->wkbType() ) )
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
    feature.setGeometry( QgsGeometry( QgsGeometryCheckerUtils::getGeomPart( geom, 0 )->clone() ) );
    featurePool->updateFeature( feature );
    error->setFixed( method );
    changes[error->layerId()][feature.id()].append( Change( ChangeFeature, ChangeChanged ) );
  }
  else if ( method == RemoveObject )
  {
    featurePool->deleteFeature( feature.id() );
    error->setFixed( method );
    changes[error->layerId()][feature.id()].append( Change( ChangeFeature, ChangeRemoved ) );
  }
  else
  {
    error->setFixFailed( tr( "Unknown method" ) );
  }
}

QStringList QgsGeometryMultipartCheck::resolutionMethods() const
{
  static const QStringList methods = QStringList()
                                     << tr( "Convert to single part feature" )
                                     << tr( "Delete feature" )
                                     << tr( "No action" );
  return methods;
}
