/***************************************************************************
    qgsgeometryduplicatecheck.cpp
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
#include "qgsgeometryengine.h"
#include "qgsgeometryduplicatecheck.h"
#include "qgsspatialindex.h"
#include "qgsgeometry.h"
#include "qgsfeaturepool.h"
#include "qgsvectorlayer.h"

QString QgsGeometryDuplicateCheckError::duplicatesString( const QMap<QString, QgsFeaturePool *> &featurePools, const QMap<QString, QList<QgsFeatureId>> &duplicates )
{
  QStringList str;
  for ( auto it = duplicates.constBegin(); it != duplicates.constEnd(); ++it )
  {
    str.append( featurePools[it.key()]->layer()->name() + ":" );
    QStringList ids;
    for ( QgsFeatureId id : it.value() )
    {
      ids.append( QString::number( id ) );
    }
    str.back() += ids.join( "," );
  }
  return str.join( QStringLiteral( "; " ) );
}


void QgsGeometryDuplicateCheck::collectErrors( const QMap<QString, QgsFeaturePool *> &featurePools, QList<QgsGeometryCheckError *> &errors, QStringList &messages, QgsFeedback *feedback, const LayerFeatureIds &ids ) const
{
  QMap<QString, QgsFeatureIds> featureIds = ids.isEmpty() ? allLayerFeatureIds( featurePools ) : ids.toMap();
  QgsGeometryCheckerUtils::LayerFeatures layerFeaturesA( featurePools, featureIds, compatibleGeometryTypes(), feedback, mContext, true );
  QList<QString> layerIds = featureIds.keys();
  for ( const QgsGeometryCheckerUtils::LayerFeature &layerFeatureA : layerFeaturesA )
  {
    // Ensure each pair of layers only gets compared once: remove the current layer from the layerIds, but add it to the layerList for layerFeaturesB
    layerIds.removeOne( layerFeatureA.layer()->id() );

    QgsRectangle bboxA = layerFeatureA.geometry().boundingBox();
    std::unique_ptr< QgsGeometryEngine > geomEngineA = QgsGeometryCheckerUtils::createGeomEngine( layerFeatureA.geometry().constGet(), mContext->tolerance );
    if ( !geomEngineA->isValid() )
    {
      messages.append( tr( "Duplicate check failed for (%1): the geometry is invalid" ).arg( layerFeatureA.id() ) );
      continue;
    }
    QMap<QString, QList<QgsFeatureId>> duplicates;

    QgsWkbTypes::GeometryType geomType = layerFeatureA.feature().geometry().type();
    QgsGeometryCheckerUtils::LayerFeatures layerFeaturesB( featurePools, QList<QString>() << layerFeatureA.layer()->id() << layerIds, bboxA, {geomType}, mContext );
    for ( const QgsGeometryCheckerUtils::LayerFeature &layerFeatureB : layerFeaturesB )
    {
      // > : only report overlaps within same layer once
      if ( layerFeatureA.layer()->id() == layerFeatureB.layer()->id() && layerFeatureB.feature().id() >= layerFeatureA.feature().id() )
      {
        continue;
      }
      QString errMsg;
      QgsAbstractGeometry *diffGeom = geomEngineA->symDifference( layerFeatureB.geometry().constGet(), &errMsg );
      if ( errMsg.isEmpty() && diffGeom && diffGeom->isEmpty() )
      {
        duplicates[layerFeatureB.layer()->id()].append( layerFeatureB.feature().id() );
      }
      else if ( !errMsg.isEmpty() )
      {
        messages.append( tr( "Duplicate check failed for (%1, %2): %3" ).arg( layerFeatureA.id(), layerFeatureB.id(), errMsg ) );
      }
      delete diffGeom;
    }
    if ( !duplicates.isEmpty() )
    {
      errors.append( new QgsGeometryDuplicateCheckError( this, layerFeatureA, layerFeatureA.geometry().constGet()->centroid(), featurePools, duplicates ) );
    }
  }
}

void QgsGeometryDuplicateCheck::fixError( const QMap<QString, QgsFeaturePool *> &featurePools, QgsGeometryCheckError *error, int method, const QMap<QString, int> & /*mergeAttributeIndices*/, Changes &changes ) const
{
  QgsFeaturePool *featurePoolA = featurePools[ error->layerId() ];
  QgsFeature featureA;
  if ( !featurePoolA->getFeature( error->featureId(), featureA ) )
  {
    error->setObsolete();
    return;
  }

  if ( method == NoChange )
  {
    error->setFixed( method );
  }
  else if ( method == RemoveDuplicates )
  {
    QgsGeometryCheckerUtils::LayerFeature layerFeatureA( featurePoolA, featureA, mContext, true );
    std::unique_ptr< QgsGeometryEngine > geomEngineA = QgsGeometryCheckerUtils::createGeomEngine( layerFeatureA.geometry().constGet(), mContext->tolerance );

    QgsGeometryDuplicateCheckError *duplicateError = static_cast<QgsGeometryDuplicateCheckError *>( error );
    for ( const QString &layerIdB : duplicateError->duplicates().keys() )
    {
      QgsFeaturePool *featurePoolB = featurePools[ layerIdB ];
      for ( QgsFeatureId idB : duplicateError->duplicates()[layerIdB] )
      {
        QgsFeature featureB;
        if ( !featurePoolB->getFeature( idB, featureB ) )
        {
          continue;
        }
        QgsGeometryCheckerUtils::LayerFeature layerFeatureB( featurePoolB, featureB, mContext, true );
        QgsAbstractGeometry *diffGeom = geomEngineA->symDifference( layerFeatureB.geometry().constGet() );
        if ( diffGeom && diffGeom->isEmpty() )
        {
          featurePoolB->deleteFeature( featureB.id() );
          changes[layerIdB][idB].append( Change( ChangeFeature, ChangeRemoved ) );
        }

        delete diffGeom;
      }
    }
    error->setFixed( method );
  }
  else
  {
    error->setFixFailed( tr( "Unknown method" ) );
  }
}

QStringList QgsGeometryDuplicateCheck::resolutionMethods() const
{
  static QStringList methods = QStringList()
                               << tr( "No action" )
                               << tr( "Remove duplicates" );
  return methods;
}
