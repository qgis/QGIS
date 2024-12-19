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
#include "qgsgeometrycheckcontext.h"
#include "qgspoint.h"



void QgsSingleGeometryCheck::collectErrors( const QMap<QString, QgsFeaturePool *> &featurePools,
    QList<QgsGeometryCheckError *> &errors,
    QStringList &messages, QgsFeedback *feedback, const LayerFeatureIds &ids ) const
{
  Q_UNUSED( messages )
  const QMap<QString, QgsFeatureIds> featureIds = ids.isEmpty() ? allLayerFeatureIds( featurePools ) : ids.toMap();
  const QgsGeometryCheckerUtils::LayerFeatures layerFeatures( featurePools, featureIds, compatibleGeometryTypes(), feedback, mContext );
  for ( const QgsGeometryCheckerUtils::LayerFeature &layerFeature : layerFeatures )
  {
    const auto singleErrors = processGeometry( layerFeature.geometry() );
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
  return mGeometry.equals( other->mGeometry )
         && mCheck == other->mCheck
         && mErrorLocation.equals( other->mErrorLocation )
         && mVertexId == other->mVertexId;
}

bool QgsSingleGeometryCheckError::handleChanges( const QList<QgsGeometryCheck::Change> &changes )
{
  Q_UNUSED( changes )
  return true;
}

QString QgsSingleGeometryCheckError::description() const
{
  return mCheck->description();
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
