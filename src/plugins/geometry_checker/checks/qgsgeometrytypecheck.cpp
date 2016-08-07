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
#include "qgsgeometrycollection.h"
#include "qgsmulticurve.h"
#include "qgsmultilinestring.h"
#include "qgsmultipoint.h"
#include "qgsmultipolygon.h"
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
    QgsGeometry featureGeom = feature.geometry();
    QgsAbstractGeometry* geom = featureGeom.geometry();

    QgsWkbTypes::Type type = QgsWkbTypes::flatType( geom->wkbType() );
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
  QgsGeometry featureGeom = feature.geometry();
  QgsAbstractGeometry* geom = featureGeom.geometry();

  // Check if error still applies
  QgsWkbTypes::Type type = QgsWkbTypes::flatType( geom->wkbType() );
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
    if ( QgsWkbTypes::isMultiType( type ) && (( 1 << QgsWkbTypes::singleType( type ) ) & mAllowedTypes ) != 0 )
    {
      // Explode multi-type feature into single-type features
      for ( int iPart = 1, nParts = geom->partCount(); iPart < nParts; ++iPart )
      {
        QgsFeature newFeature;
        newFeature.setAttributes( feature.attributes() );
        newFeature.setGeometry( QgsGeometry( QgsGeometryCheckerUtils::getGeomPart( geom, iPart )->clone() ) );
        mFeaturePool->addFeature( newFeature );
        changes[newFeature.id()].append( Change( ChangeFeature, ChangeAdded ) );
      }
      // Recycle feature for part 0
      feature.setGeometry( QgsGeometry( QgsGeometryCheckerUtils::getGeomPart( geom, 0 )->clone() ) );
      mFeaturePool->updateFeature( feature );
      changes[feature.id()].append( Change( ChangeFeature, ChangeChanged ) );
    }
    // Check if corresponding multi type is allowed
    else if ( QgsWkbTypes::isSingleType( type ) && (( 1 << QgsWkbTypes::multiType( type ) ) & mAllowedTypes ) != 0 )
    {
      QgsGeometryCollection* geomCollection = nullptr;
      switch ( QgsWkbTypes::multiType( type ) )
      {
        case QgsWkbTypes::MultiPoint:
        {
          geomCollection = new QgsMultiPointV2();
          break;
        }
        case QgsWkbTypes::MultiLineString:
        {
          geomCollection = new QgsMultiLineString();
          break;
        }
        case QgsWkbTypes::MultiPolygon:
        {
          geomCollection = new QgsMultiPolygonV2();
          break;
        }
        case QgsWkbTypes::MultiCurve:
        {
          geomCollection = new QgsMultiCurve();
          break;
        }
        case QgsWkbTypes::MultiSurface:
        {
          geomCollection = new QgsMultiSurface();
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

        feature.setGeometry( QgsGeometry( geomCollection ) );
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
