/***************************************************************************
    qgsgeometrytypecheck.cpp
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

#include "qgsgeometrytypecheck.h"
#include "qgsgeometrycollectionv2.h"
#include "qgsmulticurvev2.h"
#include "qgsmultilinestringv2.h"
#include "qgsmultipointv2.h"
#include "qgsmultipolygonv2.h"
#include "qgsmultirenderchecker.h"
#include "../utils/qgsfeaturepool.h"


void QgsGeometryTypeCheck::collectErrors( QList<QgsGeometryCheckError*>& errors, QStringList &/*messages*/, QAtomicInt* progressCounter , const QgsFeatureIds &ids ) const
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

    QgsWKBTypes::Type type = QgsWKBTypes::flatType( geom->wkbType() );
    if (( mAllowedTypes & ( 1 << type ) ) == 0 )
    {
      errors.append( new QgsGeometryTypeCheckError( this, featureid, geom->centroid(), type ) );
    }
  }
}

void QgsGeometryTypeCheck::fixError( QgsGeometryCheckError* error, int method, int /*mergeAttributeIndex*/, Changes &changes ) const
{
  QgsFeature feature;
  if ( !mFeaturePool->get( error->featureId(), feature ) )
  {
    error->setObsolete();
    return;
  }
  QgsAbstractGeometryV2* geom = feature.geometry()->geometry();

  // Check if error still applies
  QgsWKBTypes::Type type = QgsWKBTypes::flatType( geom->wkbType() );
  if (( mAllowedTypes & ( 1 << type ) ) != 0 )
  {
    error->setObsolete();
    return;
  }

  // Fix with selected method
  if ( method == NoChange )
  {
    error->setFixed( method );
  }
  else if ( method == Convert )
  {
    // Check if corresponding single type is allowed
    if ( QgsWKBTypes::isMultiType( type ) && (( 1 << QgsWKBTypes::singleType( type ) ) & mAllowedTypes ) != 0 )
    {
      // Explode multi-type feature into single-type features
      for ( int iPart = 1, nParts = geom->partCount(); iPart < nParts; ++iPart )
      {
        QgsFeature newFeature;
        newFeature.setAttributes( feature.attributes() );
        newFeature.setGeometry( new QgsGeometry( QgsGeomUtils::getGeomPart( geom, iPart )->clone() ) );
        mFeaturePool->addFeature( newFeature );
        changes[newFeature.id()].append( Change( ChangeFeature, ChangeAdded ) );
      }
      // Recycle feature for part 0
      feature.setGeometry( new QgsGeometry( QgsGeomUtils::getGeomPart( geom, 0 )->clone() ) );
      mFeaturePool->updateFeature( feature );
      changes[feature.id()].append( Change( ChangeFeature, ChangeChanged ) );
    }
    // Check if corresponding multi type is allowed
    else if ( QgsWKBTypes::isSingleType( type ) && (( 1 << QgsWKBTypes::multiType( type ) ) & mAllowedTypes ) != 0 )
    {
      QgsGeometryCollectionV2* geomCollection = nullptr;
      switch ( QgsWKBTypes::multiType( type ) )
      {
        case QgsWKBTypes::MultiPoint:
        {
          geomCollection = new QgsMultiPointV2();
          break;
        }
        case QgsWKBTypes::MultiLineString:
        {
          geomCollection = new QgsMultiLineStringV2();
          break;
        }
        case QgsWKBTypes::MultiPolygon:
        {
          geomCollection = new QgsMultiPolygonV2();
          break;
        }
        case QgsWKBTypes::MultiCurve:
        {
          geomCollection = new QgsMultiCurveV2();
          break;
        }
        case QgsWKBTypes::MultiSurface:
        {
          geomCollection = new QgsMultiSurfaceV2();
          break;
        }
        default:
          break;
      }
      if ( !geomCollection )
      {
        error->setFixFailed( tr( "Unknown geometry type" ) );
      }
      else
      {
        geomCollection->addGeometry( geom->clone() );

        feature.setGeometry( new QgsGeometry( geomCollection ) );
        mFeaturePool->updateFeature( feature );
        changes[feature.id()].append( Change( ChangeFeature, ChangeChanged ) );
      }
    }
    // Delete feature
    else
    {
      mFeaturePool->deleteFeature( feature );
      changes[error->featureId()].append( Change( ChangeFeature, ChangeRemoved ) );
    }
    error->setFixed( method );
  }
  else if ( method == Delete )
  {
    mFeaturePool->deleteFeature( feature );
    error->setFixed( method );
    changes[error->featureId()].append( Change( ChangeFeature, ChangeRemoved ) );
  }
  else
  {
    error->setFixFailed( tr( "Unknown method" ) );
  }
}

const QStringList& QgsGeometryTypeCheck::getResolutionMethods() const
{
  static QStringList methods = QStringList()
                               << tr( "Convert to corresponding multi or single type if possible, otherwise delete feature" )
                               << tr( "Delete feature" )
                               << tr( "No action" );
  return methods;
}
