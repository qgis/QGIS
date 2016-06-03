/***************************************************************************
    qgsgeometryoverlapcheck.cpp
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

#include "qgsgeometryengine.h"
#include "qgsgeometryoverlapcheck.h"
#include "../utils/qgsfeaturepool.h"

void QgsGeometryOverlapCheck::collectErrors( QList<QgsGeometryCheckError*>& errors, QStringList &messages, QAtomicInt* progressCounter , const QgsFeatureIds &ids ) const
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
    QgsGeometryEngine* geomEngine = QgsGeomUtils::createGeomEngine( geom, QgsGeometryCheckPrecision::tolerance() );

    QgsFeatureIds ids = mFeaturePool->getIntersects( feature.geometry()->boundingBox() );
    Q_FOREACH ( QgsFeatureId otherid, ids )
    {
      // >= : only report overlaps once
      if ( otherid >= featureid )
      {
        continue;
      }

      QgsFeature otherFeature;
      if ( !mFeaturePool->get( otherid, otherFeature ) )
      {
        continue;
      }

      QString errMsg;
      if ( geomEngine->overlaps( *otherFeature.geometry()->geometry(), &errMsg ) )
      {
        QgsAbstractGeometryV2* interGeom = geomEngine->intersection( *otherFeature.geometry()->geometry() );
        if ( interGeom && !interGeom->isEmpty() )
        {
          QgsGeomUtils::filter1DTypes( interGeom );
          for ( int iPart = 0, nParts = interGeom->partCount(); iPart < nParts; ++iPart )
          {
            double area = QgsGeomUtils::getGeomPart( interGeom, iPart )->area();
            if ( area > QgsGeometryCheckPrecision::reducedTolerance() && area < mThreshold )
            {
              errors.append( new QgsGeometryOverlapCheckError( this, featureid, QgsGeomUtils::getGeomPart( interGeom, iPart )->centroid(), area, otherid ) );
            }
          }
        }
        else if ( !errMsg.isEmpty() )
        {
          messages.append( tr( "Overlap check between features %1 and %2: %3" ).arg( feature.id() ).arg( otherFeature.id() ).arg( errMsg ) );
        }
        delete interGeom;
      }
    }
    delete geomEngine;
  }
}

void QgsGeometryOverlapCheck::fixError( QgsGeometryCheckError* error, int method, int /*mergeAttributeIndex*/, Changes &changes ) const
{
  QString errMsg;
  QgsGeometryOverlapCheckError* overlapError = static_cast<QgsGeometryOverlapCheckError*>( error );

  QgsFeature feature;
  QgsFeature otherFeature;
  if ( !mFeaturePool->get( error->featureId(), feature ) ||
       !mFeaturePool->get( overlapError->otherId(), otherFeature ) )
  {
    error->setObsolete();
    return;
  }
  QgsAbstractGeometryV2* geom = feature.geometry()->geometry();
  QgsGeometryEngine* geomEngine = QgsGeomUtils::createGeomEngine( geom, QgsGeometryCheckPrecision::tolerance() );

  // Check if error still applies
  if ( !geomEngine->overlaps( *otherFeature.geometry()->geometry() ) )
  {
    delete geomEngine;
    error->setObsolete();
    return;
  }
  QgsAbstractGeometryV2* interGeom = geomEngine->intersection( *otherFeature.geometry()->geometry(), &errMsg );
  delete geomEngine;
  if ( !interGeom )
  {
    error->setFixFailed( tr( "Failed to compute intersection between overlapping features: %1" ).arg( errMsg ) );
    return;
  }

  // Search which overlap part this error parametrizes (using fuzzy-matching of the area and centroid...)
  QgsAbstractGeometryV2* interPart = nullptr;
  for ( int iPart = 0, nParts = interGeom->partCount(); iPart < nParts; ++iPart )
  {
    QgsAbstractGeometryV2* part = QgsGeomUtils::getGeomPart( interGeom, iPart );
    if ( qAbs( part->area() - overlapError->value().toDouble() ) < QgsGeometryCheckPrecision::reducedTolerance() &&
         QgsGeomUtils::pointsFuzzyEqual( part->centroid(), overlapError->location(), QgsGeometryCheckPrecision::reducedTolerance() ) )
    {
      interPart = part;
      break;
    }
  }
  if ( !interPart || interPart->isEmpty() )
  {
    delete interGeom;
    error->setObsolete();
    return;
  }

  // Fix error
  if ( method == NoChange )
  {
    error->setFixed( method );
  }
  else if ( method == Subtract )
  {
    geomEngine = QgsGeomUtils::createGeomEngine( geom, QgsGeometryCheckPrecision::reducedTolerance() );
    QgsAbstractGeometryV2* diff1 = geomEngine->difference( *interPart, &errMsg );
    delete geomEngine;
    if ( !diff1 || diff1->isEmpty() )
    {
      delete diff1;
      diff1 = nullptr;
    }
    else
    {
      QgsGeomUtils::filter1DTypes( diff1 );
    }
    QgsGeometryEngine* otherGeomEngine = QgsGeomUtils::createGeomEngine( otherFeature.geometry()->geometry(), QgsGeometryCheckPrecision::reducedTolerance() );
    QgsAbstractGeometryV2* diff2 = otherGeomEngine->difference( *interPart, &errMsg );
    delete otherGeomEngine;
    if ( !diff2 || diff2->isEmpty() )
    {
      delete diff2;
      diff2 = nullptr;
    }
    else
    {
      QgsGeomUtils::filter1DTypes( diff2 );
    }
    double shared1 = diff1 ? QgsGeomUtils::sharedEdgeLength( diff1, interPart, QgsGeometryCheckPrecision::reducedPrecision() ) : 0;
    double shared2 = diff2 ? QgsGeomUtils::sharedEdgeLength( diff2, interPart, QgsGeometryCheckPrecision::reducedPrecision() ) : 0;
    if ( shared1 == 0. || shared2 == 0. )
    {
      error->setFixFailed( tr( "Could not find shared edges between intersection and overlapping features" ) );
    }
    else
    {
      if ( shared1 < shared2 )
      {
        feature.setGeometry( new QgsGeometry( diff1 ) );

        changes[feature.id()].append( Change( ChangeFeature, ChangeChanged ) );
        mFeaturePool->updateFeature( feature );

        delete diff2;
      }
      else
      {
        otherFeature.setGeometry( new QgsGeometry( diff2 ) );

        changes[otherFeature.id()].append( Change( ChangeFeature, ChangeChanged ) );
        mFeaturePool->updateFeature( otherFeature );

        delete diff1;
      }

      error->setFixed( method );
    }
  }
  else
  {
    error->setFixFailed( tr( "Unknown method" ) );
  }
  delete interGeom;
}

const QStringList& QgsGeometryOverlapCheck::getResolutionMethods() const
{
  static QStringList methods = QStringList()
                               << tr( "Remove overlapping area from neighboring polygon with shortest shared edge" )
                               << tr( "No action" );
  return methods;
}
