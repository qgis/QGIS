/***************************************************************************
  topolError.h
  TOPOLogy checker
  -------------------
         date                 : May 2009
         copyright            : (C) 2009 by Vita Cizek
         email                : weetya (at) gmail.com

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "topolError.h"
#include "qgsmessagelog.h"
#include "qgsfeatureiterator.h"
#include "qgsvectorlayer.h"

//TODO: tell dock to parse errorlist when feature is deleted
bool TopolError::fix( const QString &fixName )
{
  QgsMessageLog::logMessage( QObject::tr( "Using fix %1." ).arg( fixName ), QObject::tr( "Topology plugin" ), Qgis::MessageLevel::Info );
  return ( this->*mFixMap[fixName] )();
}

bool TopolError::fixMove( const FeatureLayer &fl1, const FeatureLayer &fl2 )
{
  bool ok;
  QgsFeature f1, f2;

  ok = fl1.layer->getFeatures( QgsFeatureRequest().setFilterFid( fl1.feature.id() ) ).nextFeature( f1 );
  ok = ok && fl2.layer->getFeatures( ( QgsFeatureRequest().setFilterFid( fl2.feature.id() ) ) ).nextFeature( f2 );

  if ( !ok )
    return false;


  // 0 means success
  const QgsGeometry g = f1.geometry();
  QgsGeometry difference = g.makeDifference( f2.geometry() );
  if ( !difference.isNull() )
  {
    return fl1.layer->changeGeometry( f1.id(), difference );
  }

  return false;
}

bool TopolError::fixMoveFirst()
{
  return fixMove( mFeaturePairs.at( 0 ), mFeaturePairs.at( 1 ) );
}

bool TopolError::fixMoveSecond()
{
  return fixMove( mFeaturePairs.at( 1 ), mFeaturePairs.at( 0 ) );
}

bool TopolError::fixUnion( const FeatureLayer &fl1, const FeatureLayer &fl2 )
{
  bool ok;
  QgsFeature f1, f2;

  ok = fl1.layer->getFeatures( QgsFeatureRequest().setFilterFid( fl1.feature.id() ) ).nextFeature( f1 );
  ok = ok && fl2.layer->getFeatures( ( QgsFeatureRequest().setFilterFid( fl2.feature.id() ) ) ).nextFeature( f2 );

  if ( !ok )
    return false;

  QgsGeometry g = f1.geometry().combine( f2.geometry() );
  if ( g.isNull() )
    return false;

  if ( fl2.layer->deleteFeature( f2.id() ) )
    return fl1.layer->changeGeometry( f1.id(), g );

  return false;
}

bool TopolError::fixSnap()
{
  bool ok;
  QgsFeature f1, f2;
  FeatureLayer fl = mFeaturePairs.at( 1 );
  ok = fl.layer->getFeatures( ( QgsFeatureRequest().setFilterFid( fl.feature.id() ) ) ).nextFeature( f2 );
  fl = mFeaturePairs.first();
  ok = ok && fl.layer->getFeatures( QgsFeatureRequest().setFilterFid( fl.feature.id() ) ).nextFeature( f1 );

  if ( !ok )
    return false;

  const QgsGeometry ge = f1.geometry();

  QgsPolylineXY line = ge.asPolyline();
  QgsPolylineXY conflictLine = mConflict.asPolyline();
  line.last() = conflictLine.last();

  QgsGeometry newG = QgsGeometry::fromPolylineXY( line );
  const bool ret = fl.layer->changeGeometry( f1.id(), newG );

  return ret;
}

bool TopolError::fixUnionFirst()
{
  return fixUnion( mFeaturePairs.at( 0 ), mFeaturePairs.at( 1 ) );
}

bool TopolError::fixUnionSecond()
{
  return fixUnion( mFeaturePairs.at( 1 ), mFeaturePairs.at( 0 ) );
}

bool TopolError::fixDeleteFirst()
{
  FeatureLayer fl = mFeaturePairs.at( 0 );
  return fl.layer->deleteFeature( fl.feature.id() );
}

bool TopolError::fixDeleteSecond()
{
  FeatureLayer fl = mFeaturePairs.at( 1 );
  return fl.layer->deleteFeature( fl.feature.id() );
}

TopolError::TopolError( const QgsRectangle &boundingBox, const QgsGeometry &conflict, const QList<FeatureLayer> &featurePairs )
  : mBoundingBox( boundingBox )
  , mConflict( conflict )
  , mFeaturePairs( featurePairs )
{
  mFixMap[QObject::tr( "Select automatic fix" )] = &TopolError::fixDummy;
}

TopolErrorIntersection::TopolErrorIntersection( const QgsRectangle &boundingBox, const QgsGeometry &conflict, QList<FeatureLayer> featurePairs )
  : TopolError( boundingBox, conflict, featurePairs )
{
  mName = QObject::tr( "intersecting geometries" );

  mFixMap[QObject::tr( "Move blue feature" )] = &TopolErrorIntersection::fixMoveFirst;
  mFixMap[QObject::tr( "Move red feature" )] = &TopolErrorIntersection::fixMoveSecond;
  mFixMap[QObject::tr( "Delete blue feature" )] = &TopolErrorIntersection::fixDeleteFirst;
  mFixMap[QObject::tr( "Delete red feature" )] = &TopolErrorIntersection::fixDeleteSecond;

  // allow union only when both features have the same geometry type
  if ( featurePairs.first().feature.geometry().type() == featurePairs[1].feature.geometry().type() )
  {
    mFixMap[QObject::tr( "Union to blue feature" )] = &TopolErrorIntersection::fixUnionFirst;
    mFixMap[QObject::tr( "Union to red feature" )] = &TopolErrorIntersection::fixUnionSecond;
  }
}

TopolErrorClose::TopolErrorClose( const QgsRectangle &boundingBox, const QgsGeometry &conflict, const QList<FeatureLayer> &featurePairs )
  : TopolError( boundingBox, conflict, featurePairs )
{
  mName = QObject::tr( "features too close" );

  mFixMap[QObject::tr( "Move blue feature" )] = &TopolErrorClose::fixMoveFirst;
  mFixMap[QObject::tr( "Move red feature" )] = &TopolErrorClose::fixMoveSecond;
  mFixMap[QObject::tr( "Snap to segment" )] = &TopolErrorClose::fixSnap;
}

TopolErrorCovered::TopolErrorCovered( const QgsRectangle &boundingBox, const QgsGeometry &conflict, const QList<FeatureLayer> &featurePairs )
  : TopolError( boundingBox, conflict, featurePairs )
{
  mName = QObject::tr( "point not covered by segment" );
  mFixMap[QObject::tr( "Delete point" )] = &TopolErrorCovered::fixDeleteFirst;
}

TopolErrorShort::TopolErrorShort( const QgsRectangle &boundingBox, const QgsGeometry &conflict, const QList<FeatureLayer> &featurePairs )
  : TopolError( boundingBox, conflict, featurePairs )
{
  mName = QObject::tr( "segment too short" );
  mFixMap[QObject::tr( "Delete feature" )] = &TopolErrorShort::fixDeleteFirst;
}

TopolErrorValid::TopolErrorValid( const QgsRectangle &boundingBox, const QgsGeometry &conflict, const QList<FeatureLayer> &featurePairs )
  : TopolError( boundingBox, conflict, featurePairs )
{
  mName = QObject::tr( "invalid geometry" );
  mFixMap[QObject::tr( "Delete feature" )] = &TopolErrorValid::fixDeleteFirst;
}

TopolErrorDangle::TopolErrorDangle( const QgsRectangle &boundingBox, const QgsGeometry &conflict, const QList<FeatureLayer> &featurePairs )
  : TopolError( boundingBox, conflict, featurePairs )
{
  mName = QObject::tr( "dangling end" );
  mFixMap[QObject::tr( "Delete feature" )] = &TopolErrorDangle::fixDeleteFirst;
}

TopolErrorDuplicates::TopolErrorDuplicates( const QgsRectangle &boundingBox, const QgsGeometry &conflict, const QList<FeatureLayer> &featurePairs )
  : TopolError( boundingBox, conflict, featurePairs )
{
  mName = QObject::tr( "duplicate geometry" );
  //mFixMap["Delete feature"] = &TopolErrorDuplicates::fixDeleteFirst;
}

TopolErrorPseudos::TopolErrorPseudos( const QgsRectangle &boundingBox, const QgsGeometry &conflict, const QList<FeatureLayer> &featurePairs )
  : TopolError( boundingBox, conflict, featurePairs )
{
  mName = QObject::tr( "pseudo node" );
  //mFixMap["Delete feature"] = &TopolErrorDuplicates::fixDeleteFirst;
}

TopolErrorOverlaps::TopolErrorOverlaps( const QgsRectangle &boundingBox, const QgsGeometry &conflict, const QList<FeatureLayer> &featurePairs )
  : TopolError( boundingBox, conflict, featurePairs )
{
  mName = QObject::tr( "overlaps" );
  //mFixMap["Delete feature"] = &TopolErrorDuplicates::fixDeleteFirst;
}

TopolErrorGaps::TopolErrorGaps( const QgsRectangle &boundingBox, const QgsGeometry &conflict, const QList<FeatureLayer> &featurePairs )
  : TopolError( boundingBox, conflict, featurePairs )
{
  mName = QObject::tr( "gaps" );
  //mFixMap["Delete feature"] = &TopolErrorDuplicates::fixDeleteFirst;
}

TopolErrorPointNotCoveredByLineEnds::TopolErrorPointNotCoveredByLineEnds( const QgsRectangle &boundingBox, const QgsGeometry &conflict, const QList<FeatureLayer> &featurePairs )
  : TopolError( boundingBox, conflict, featurePairs )
{
  mName = QObject::tr( "point not covered" );
}

TopolErrorLineEndsNotCoveredByPoints::TopolErrorLineEndsNotCoveredByPoints( const QgsRectangle &boundingBox, const QgsGeometry &conflict, const QList<FeatureLayer> &featurePairs )
  : TopolError( boundingBox, conflict, featurePairs )
{
  mName = QObject::tr( "line ends not covered by point" );
}

TopolErrorPointNotInPolygon::TopolErrorPointNotInPolygon( const QgsRectangle &boundingBox, const QgsGeometry &conflict, const QList<FeatureLayer> &featurePairs )
  : TopolError( boundingBox, conflict, featurePairs )
{
  mName = QObject::tr( "point not in polygon" );
}

TopolErrorPolygonContainsPoint::TopolErrorPolygonContainsPoint( const QgsRectangle &boundingBox, const QgsGeometry &conflict, const QList<FeatureLayer> &featurePairs )
  : TopolError( boundingBox, conflict, featurePairs )
{
  mName = QObject::tr( "polygon does not contain point" );
}

TopolErroMultiPart::TopolErroMultiPart( const QgsRectangle &boundingBox, const QgsGeometry &conflict, const QList<FeatureLayer> &featurePairs )
  : TopolError( boundingBox, conflict, featurePairs )
{
  mName = QObject::tr( "multipart feature" );
}
