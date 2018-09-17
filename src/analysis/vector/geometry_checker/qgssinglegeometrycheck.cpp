/***************************************************************************
                      qgssinglegeometrycheck.cpp
                     --------------------------------------
Date                 : 6.9.2018
Copyright            : (C) 2018 by Matthias Kuhn
email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgssinglegeometrycheck.h"
#include "qgspoint.h"

QgsSingleGeometryCheck::QgsSingleGeometryCheck( CheckType checkType, const QList<QgsWkbTypes::GeometryType> &compatibleGeometryTypes, QgsGeometryCheckerContext *context )
  : QgsGeometryCheck( checkType, compatibleGeometryTypes, context )
{

}

void QgsSingleGeometryCheck::collectErrors( QList<QgsGeometryCheckError *> &errors, QStringList &messages, QAtomicInt *progressCounter, const QMap<QString, QgsFeatureIds> &ids ) const
{
  Q_UNUSED( messages )
  QMap<QString, QgsFeatureIds> featureIds = ids.isEmpty() ? allLayerFeatureIds() : ids;
  QgsGeometryCheckerUtils::LayerFeatures layerFeatures( mContext->featurePools, featureIds, mCompatibleGeometryTypes, progressCounter, mContext );
  for ( const QgsGeometryCheckerUtils::LayerFeature &layerFeature : layerFeatures )
  {
    const auto singleErrors = processGeometry( layerFeature.geometry(), QVariantMap() );
    for ( const auto error : singleErrors )
      errors.append( convertToGeometryCheckError( error, layerFeature ) );
  }
}

QgsGeometryCheckErrorSingle *QgsSingleGeometryCheck::convertToGeometryCheckError( QgsSingleGeometryCheckError *singleGeometryCheckError, const QgsGeometryCheckerUtils::LayerFeature &layerFeature ) const
{
  return new QgsGeometryCheckErrorSingle( singleGeometryCheckError, layerFeature );
}

void QgsSingleGeometryCheckError::update( const QgsSingleGeometryCheckError *other )
{
  Q_ASSERT( mCheck == other->mCheck );
  mErrorLocation = other->mErrorLocation;
  mVertexId = other->mVertexId;
  mGeometry = other->mGeometry;
}

bool QgsSingleGeometryCheckError::isEqual( const QgsSingleGeometryCheckError *other ) const
{
  return mGeometry == other->mGeometry
         && mCheck == other->mCheck
         && mErrorLocation == other->mErrorLocation
         && mVertexId == other->mVertexId;
}

bool QgsSingleGeometryCheckError::handleChanges( const QList<QgsGeometryCheck::Change> &changes )
{
  Q_UNUSED( changes )
  return true;
}

QString QgsSingleGeometryCheckError::description() const
{
  return mCheck->errorDescription();
}

const QgsSingleGeometryCheck *QgsSingleGeometryCheckError::check() const
{
  return mCheck;
}

QgsGeometry QgsSingleGeometryCheckError::errorLocation() const
{
  return mErrorLocation;
}

QgsVertexId QgsSingleGeometryCheckError::vertexId() const
{
  return mVertexId;
}

QgsGeometryCheckErrorSingle::QgsGeometryCheckErrorSingle( QgsSingleGeometryCheckError *error, const QgsGeometryCheckerUtils::LayerFeature &layerFeature )
  : QgsGeometryCheckError( error->check(), layerFeature, QgsPointXY( error->errorLocation().constGet()->centroid() ), error->vertexId() ) // TODO: should send geometry to QgsGeometryCheckError
  , mError( error )
{

}

QgsSingleGeometryCheckError *QgsGeometryCheckErrorSingle::singleError() const
{
  return mError;
}

bool QgsGeometryCheckErrorSingle::handleChanges( const QgsGeometryCheck::Changes &changes )
{
  if ( !QgsGeometryCheckError::handleChanges( changes ) )
    return false;

  return mError->handleChanges( changes.value( layerId() ).value( featureId() ) );
}

void QgsSingleGeometryCheck::collectErrors( QList<QgsGeometryCheckError *> &errors, QStringList &messages, QAtomicInt *progressCounter, const QMap<QString, QgsFeatureIds> &ids ) const
{
  QMap<QString, QgsFeatureIds> featureIds = ids.isEmpty() ? allLayerFeatureIds() : ids;
  QgsGeometryCheckerUtils::LayerFeatures layerFeatures( mContext->featurePools, featureIds, mCompatibleGeometryTypes, progressCounter, mContext );
  for ( const QgsGeometryCheckerUtils::LayerFeature &layerFeature : layerFeatures )
  {
    errors.append( processGeometry( layerFeature.geometry() ) );
  }
}
