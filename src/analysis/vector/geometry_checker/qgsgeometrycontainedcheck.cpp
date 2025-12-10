/***************************************************************************
    qgsgeometrycontainedcheck.cpp
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

#include "qgsgeometrycontainedcheck.h"

#include "qgsfeaturepool.h"
#include "qgsfeedback.h"
#include "qgsgeometrycheckcontext.h"
#include "qgsgeometryengine.h"
#include "qgsvectorlayer.h"

QgsGeometryCheck::Result QgsGeometryContainedCheck::collectErrors( const QMap<QString, QgsFeaturePool *> &featurePools, QList<QgsGeometryCheckError *> &errors, QStringList &messages, QgsFeedback *feedback, const LayerFeatureIds &ids ) const
{
  QMap<QString, QSet<QVariant>> uniqueIds;
  const QMap<QString, QgsFeatureIds> featureIds = ids.isEmpty() ? allLayerFeatureIds( featurePools ) : ids.toMap();
  const QgsGeometryCheckerUtils::LayerFeatures layerFeaturesA( featurePools, featureIds, compatibleGeometryTypes(), feedback, mContext );
  for ( const QgsGeometryCheckerUtils::LayerFeature &layerFeatureA : layerFeaturesA )
  {
    if ( feedback && feedback->isCanceled() )
    {
      return QgsGeometryCheck::Result::Canceled;
    }

    if ( context()->uniqueIdFieldIndex != -1 )
    {
      QgsGeometryCheck::Result result = checkUniqueId( layerFeatureA, uniqueIds );
      if ( result != QgsGeometryCheck::Result::Success )
      {
        return result;
      }
    }

    const QgsRectangle bboxA = layerFeatureA.geometry().boundingBox();
    std::unique_ptr<QgsGeometryEngine> geomEngineA( QgsGeometry::createGeometryEngine( layerFeatureA.geometry().constGet(), mContext->tolerance ) );
    if ( !geomEngineA->isValid() )
    {
      messages.append( tr( "Contained check failed for (%1): the geometry is invalid" ).arg( layerFeatureA.id() ) );
      continue;
    }
    const QgsGeometryCheckerUtils::LayerFeatures layerFeaturesB( featurePools, featureIds.keys(), bboxA, compatibleGeometryTypes(), mContext );
    for ( const QgsGeometryCheckerUtils::LayerFeature &layerFeatureB : layerFeaturesB )
    {
      if ( feedback && feedback->isCanceled() )
      {
        return QgsGeometryCheck::Result::Canceled;
      }

      if ( layerFeatureA == layerFeatureB )
      {
        continue;
      }
      std::unique_ptr<QgsGeometryEngine> geomEngineB( QgsGeometry::createGeometryEngine( layerFeatureB.geometry().constGet(), mContext->tolerance ) );
      if ( !geomEngineB->isValid() )
      {
        messages.append( tr( "Contained check failed for (%1): the geometry is invalid" ).arg( layerFeatureB.id() ) );
        continue;
      }
      QString errMsg;
      // If A contains B and B contains A, it would mean that the geometries are identical, which is covered by the duplicate check
      if ( geomEngineA->contains( layerFeatureB.geometry().constGet(), &errMsg ) && !geomEngineB->contains( layerFeatureA.geometry().constGet(), &errMsg ) && errMsg.isEmpty() )
      {
        errors.append( new QgsGeometryContainedCheckError( this, layerFeatureB, layerFeatureB.geometry().constGet()->centroid(), layerFeatureA ) );
      }
      else if ( !errMsg.isEmpty() )
      {
        messages.append( tr( "Contained check failed for (%1, %2): %3" ).arg( layerFeatureB.id(), layerFeatureA.id(), errMsg ) );
      }
    }
  }
  return QgsGeometryCheck::Result::Success;
}

void QgsGeometryContainedCheck::fixError( const QMap<QString, QgsFeaturePool *> &featurePools, QgsGeometryCheckError *error, int method, const QMap<QString, int> & /*mergeAttributeIndices*/, Changes &changes ) const
{
  QgsGeometryContainedCheckError *containerError = static_cast<QgsGeometryContainedCheckError *>( error );
  QgsFeaturePool *featurePoolA = featurePools[error->layerId()];
  QgsFeaturePool *featurePoolB = featurePools[containerError->containingFeature().first];

  QgsFeature featureA;
  QgsFeature featureB;
  if ( !featurePoolA->getFeature( error->featureId(), featureA ) || !featurePoolB->getFeature( containerError->containingFeature().second, featureB ) )
  {
    error->setObsolete();
    return;
  }

  // Check if error still applies
  const QgsGeometryCheckerUtils::LayerFeature layerFeatureA( featurePoolA, featureA, mContext, true );
  const QgsGeometryCheckerUtils::LayerFeature layerFeatureB( featurePoolB, featureB, mContext, true );

  std::unique_ptr<QgsGeometryEngine> geomEngineA( QgsGeometry::createGeometryEngine( layerFeatureA.geometry().constGet(), mContext->tolerance ) );
  std::unique_ptr<QgsGeometryEngine> geomEngineB( QgsGeometry::createGeometryEngine( layerFeatureB.geometry().constGet(), mContext->tolerance ) );

  if ( !( geomEngineB->contains( layerFeatureA.geometry().constGet() ) && !geomEngineA->contains( layerFeatureB.geometry().constGet() ) ) )
  {
    error->setObsolete();
    return;
  }

  // Fix error
  if ( method == NoChange )
  {
    error->setFixed( method );
  }
  else if ( method == Delete )
  {
    changes[error->layerId()][featureA.id()].append( Change( ChangeFeature, ChangeRemoved ) );
    featurePoolA->deleteFeature( featureA.id() );
    error->setFixed( method );
  }
  else
  {
    error->setFixFailed( tr( "Unknown method" ) );
  }
}

QStringList QgsGeometryContainedCheck::resolutionMethods() const
{
  static const QStringList methods = QStringList()
                                     << tr( "Delete feature" )
                                     << tr( "No action" );
  return methods;
}
