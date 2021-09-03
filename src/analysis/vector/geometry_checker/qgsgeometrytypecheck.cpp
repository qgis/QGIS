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

#include "qgsgeometrycheckcontext.h"
#include "qgsgeometrytypecheck.h"
#include "qgsgeometrycollection.h"
#include "qgsmulticurve.h"
#include "qgsmultilinestring.h"
#include "qgsmultipoint.h"
#include "qgsmultipolygon.h"
#include "qgsmultirenderchecker.h"
#include "qgsfeaturepool.h"



QList<QgsSingleGeometryCheckError *> QgsGeometryTypeCheck::processGeometry( const QgsGeometry &geometry ) const
{
  QList<QgsSingleGeometryCheckError *> errors;
  const QgsAbstractGeometry *geom = geometry.constGet();
  const QgsWkbTypes::Type type = QgsWkbTypes::flatType( geom->wkbType() );
  if ( ( mAllowedTypes & ( 1 << type ) ) == 0 )
  {
    errors.append( new QgsGeometryTypeCheckError( this, geometry, geometry, type ) );
  }
  return errors;
}

void QgsGeometryTypeCheck::fixError( const QMap<QString, QgsFeaturePool *> &featurePools, QgsGeometryCheckError *error, int method, const QMap<QString, int> & /*mergeAttributeIndices*/, Changes &changes ) const
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
  const QgsWkbTypes::Type type = QgsWkbTypes::flatType( geom->wkbType() );
  if ( ( mAllowedTypes & ( 1 << type ) ) != 0 )
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
    if ( QgsWkbTypes::isMultiType( type ) && ( ( 1 << QgsWkbTypes::singleType( type ) ) & mAllowedTypes ) != 0 )
    {
      // Explode multi-type feature into single-type features
      for ( int iPart = 1, nParts = geom->partCount(); iPart < nParts; ++iPart )
      {
        QgsFeature newFeature;
        newFeature.setAttributes( feature.attributes() );
        newFeature.setGeometry( QgsGeometry( QgsGeometryCheckerUtils::getGeomPart( geom, iPart )->clone() ) );
        featurePool->addFeature( newFeature );
        changes[error->layerId()][newFeature.id()].append( Change( ChangeFeature, ChangeAdded ) );
      }
      // Recycle feature for part 0
      feature.setGeometry( QgsGeometry( QgsGeometryCheckerUtils::getGeomPart( geom, 0 )->clone() ) );
      featurePool->updateFeature( feature );
      changes[error->layerId()][feature.id()].append( Change( ChangeFeature, ChangeChanged ) );
    }
    // Check if corresponding multi type is allowed
    else if ( QgsWkbTypes::isSingleType( type ) && ( ( 1 << QgsWkbTypes::multiType( type ) ) & mAllowedTypes ) != 0 )
    {
      QgsGeometryCollection *geomCollection = nullptr;
      switch ( QgsWkbTypes::multiType( type ) )
      {
        case QgsWkbTypes::MultiPoint:
        {
          geomCollection = new QgsMultiPoint();
          break;
        }
        case QgsWkbTypes::MultiLineString:
        {
          geomCollection = new QgsMultiLineString();
          break;
        }
        case QgsWkbTypes::MultiPolygon:
        {
          geomCollection = new QgsMultiPolygon();
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
        featurePool->updateFeature( feature );
        changes[error->layerId()][feature.id()].append( Change( ChangeFeature, ChangeChanged ) );
      }
    }
    // Delete feature
    else
    {
      featurePool->deleteFeature( feature.id() );
      changes[error->layerId()][error->featureId()].append( Change( ChangeFeature, ChangeRemoved ) );
    }
    error->setFixed( method );
  }
  else if ( method == Delete )
  {
    featurePool->deleteFeature( feature.id() );
    error->setFixed( method );
    changes[error->layerId()][error->featureId()].append( Change( ChangeFeature, ChangeRemoved ) );
  }
  else
  {
    error->setFixFailed( tr( "Unknown method" ) );
  }
}

QStringList QgsGeometryTypeCheck::resolutionMethods() const
{
  static const QStringList methods = QStringList()
                                     << tr( "Convert to corresponding multi or single type if possible, otherwise delete feature" )
                                     << tr( "Delete feature" )
                                     << tr( "No action" );
  return methods;
}

QString QgsGeometryTypeCheck::factoryDescription()
{
  return tr( "Geometry type" );
}

QString QgsGeometryTypeCheck::description() const
{
  return factoryDescription();
}

QString QgsGeometryTypeCheck::factoryId()
{
  return QStringLiteral( "QgsGeometryTypeCheck" );
}

QgsGeometryCheck::CheckType QgsGeometryTypeCheck::factoryCheckType()
{
  return QgsGeometryCheck::FeatureCheck;
}

QString QgsGeometryTypeCheck::id() const
{
  return factoryId();
}

QgsGeometryCheck::CheckType QgsGeometryTypeCheck::checkType() const
{
  return factoryCheckType();
}

bool QgsGeometryTypeCheckError::isEqual( const QgsSingleGeometryCheckError *other ) const
{
  return QgsSingleGeometryCheckError::isEqual( other ) &&
         mFlatType == static_cast<const QgsGeometryTypeCheckError *>( other )->mFlatType;
}

QString QgsGeometryTypeCheckError::description() const
{
  return QStringLiteral( "%1 (%2)" ).arg( mCheck->description(), QgsWkbTypes::displayString( mFlatType ) );
}
