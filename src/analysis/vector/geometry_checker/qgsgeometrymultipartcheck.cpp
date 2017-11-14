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
#include "qgsfeaturepool.h"

void QgsGeometryMultipartCheck::collectErrors( QList<QgsGeometryCheckError *> &errors, QStringList &/*messages*/, QAtomicInt *progressCounter, const QMap<QString, QgsFeatureIds> &ids ) const
{
  QMap<QString, QgsFeatureIds> featureIds = ids.isEmpty() ? allLayerFeatureIds() : ids;
  QgsGeometryCheckerUtils::LayerFeatures layerFeatures( mContext->featurePools, featureIds, mCompatibleGeometryTypes, progressCounter );
  for ( const QgsGeometryCheckerUtils::LayerFeature &layerFeature : layerFeatures )
  {
    const QgsAbstractGeometry *geom = layerFeature.geometry();
    QgsWkbTypes::Type type = geom->wkbType();
    if ( geom->partCount() == 1 && QgsWkbTypes::isMultiType( type ) )
    {
      errors.append( new QgsGeometryCheckError( this, layerFeature, geom->centroid() ) );
    }
  }
}

void QgsGeometryMultipartCheck::fixError( QgsGeometryCheckError *error, int method, const QMap<QString, int> & /*mergeAttributeIndices*/, Changes &changes ) const
{
  QgsFeaturePool *featurePool = mContext->featurePools[ error->layerId() ];
  QgsFeature feature;
  if ( !featurePool->get( error->featureId(), feature ) )
  {
    error->setObsolete();
    return;
  }
  QgsGeometry featureGeom = feature.geometry();
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

QStringList QgsGeometryMultipartCheck::getResolutionMethods() const
{
  static QStringList methods = QStringList()
                               << tr( "Convert to single part feature" )
                               << tr( "Delete feature" )
                               << tr( "No action" );
  return methods;
}
