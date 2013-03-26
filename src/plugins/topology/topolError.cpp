/***************************************************************************
  topolError.h
  TOPOLogy checker
  -------------------
         date                 : May 2009
         copyright            : Vita Cizek
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

//TODO: tell dock to parse errorlist when feature is deleted
bool TopolError::fix( QString fixName )
{
  std::cout << "fix: \"" << fixName.toStdString() << "\"\n";
  ( this->*mFixMap[fixName] )();
}

bool TopolError::fixMove( FeatureLayer fl1, FeatureLayer fl2 )
{
  bool ok;
  QgsFeature f1, f2;

  ok = fl1.layer->getFeatures( QgsFeatureRequest().setFilterFid( fl1.feature.id() ) ).nextFeature( f1 );
  ok = ok && fl2.layer->getFeatures(( QgsFeatureRequest().setFilterFid( fl2.feature.id() ) ) ).nextFeature( f2 );

  if ( !ok )
    return false;


  QgsGeometry* g2, *g1 = f1.geometry();
  // 0 means success
  if ( !f1.geometry()->makeDifference( f2.geometry() ) )
    return fl1.layer->changeGeometry( f1.id(), f1.geometry() );

  return false;
}

bool TopolError::fixMoveFirst()
{
  return fixMove( mFeaturePairs.first(), mFeaturePairs[1] );
}

bool TopolError::fixMoveSecond()
{
  return fixMove( mFeaturePairs[1], mFeaturePairs.first() );
}

bool TopolError::fixUnion( FeatureLayer fl1, FeatureLayer fl2 )
{
  bool ok;
  QgsFeature f1, f2;

  ok = fl1.layer->getFeatures( QgsFeatureRequest().setFilterFid( fl1.feature.id() ) ).nextFeature( f1 );
  ok = ok && fl2.layer->getFeatures(( QgsFeatureRequest().setFilterFid( fl2.feature.id() ) ) ).nextFeature( f2 );

  if ( !ok )
    return false;

  QgsGeometry* g = f1.geometry()->combine( f2.geometry() );
  if ( !g )
    return false;

  if ( fl2.layer->deleteFeature( f2.id() ) )
    return fl1.layer->changeGeometry( f1.id(), g );

  return false;
}

bool TopolError::fixSnap()
{
  bool ok;
  QgsFeature f1, f2;
  FeatureLayer fl = mFeaturePairs[1];
  ok = fl.layer->getFeatures(( QgsFeatureRequest().setFilterFid( fl.feature.id() ) ) ).nextFeature( f2 );
  fl = mFeaturePairs.first();
  ok = ok && fl.layer->getFeatures( QgsFeatureRequest().setFilterFid( fl.feature.id() ) ).nextFeature( f1 );

  if ( !ok )
    return false;

  QgsGeometry* ge = f1.geometry();

  QgsPolyline line = ge->asPolyline();
  line.last() = mConflict->asPolyline().last();

  QgsGeometry* newG = QgsGeometry::fromPolyline( line );
  bool ret = fl.layer->changeGeometry( f1.id(), newG );
  delete newG;

  return ret;
}

bool TopolError::fixUnionFirst()
{
  return fixUnion( mFeaturePairs.first(), mFeaturePairs[1] );
}

bool TopolError::fixUnionSecond()
{
  return fixUnion( mFeaturePairs[1], mFeaturePairs.first() );
}

bool TopolError::fixDeleteFirst()
{
  FeatureLayer fl = mFeaturePairs.first();
  return fl.layer->deleteFeature( fl.feature.id() );
}

bool TopolError::fixDeleteSecond()
{
  FeatureLayer fl = mFeaturePairs[1];
  return fl.layer->deleteFeature( fl.feature.id() );
}

TopolError::TopolError( QgsRectangle theBoundingBox, QgsGeometry* theConflict, QList<FeatureLayer> theFeaturePairs ):
    mFeaturePairs( theFeaturePairs ),
    mBoundingBox( theBoundingBox ),
    mConflict( theConflict )
{
  mFixMap["Select automatic fix"] = &TopolError::fixDummy;
}

TopolErrorIntersection::TopolErrorIntersection( QgsRectangle theBoundingBox, QgsGeometry* theConflict, QList<FeatureLayer> theFeaturePairs ) : TopolError( theBoundingBox, theConflict, theFeaturePairs )
{
  mName = "intersecting geometries";

  mFixMap["Move blue feature"] = &TopolErrorIntersection::fixMoveFirst;
  mFixMap["Move red feature"] = &TopolErrorIntersection::fixMoveSecond;
  mFixMap["Delete blue feature"] = &TopolErrorIntersection::fixDeleteFirst;
  mFixMap["Delete red feature"] = &TopolErrorIntersection::fixDeleteSecond;

  // allow union only when both features have the same geometry type
  if ( theFeaturePairs.first().feature.geometry()->type() == theFeaturePairs[1].feature.geometry()->type() )
  {
    mFixMap["Union to blue feature"] = &TopolErrorIntersection::fixUnionFirst;
    mFixMap["Union to red feature"] = &TopolErrorIntersection::fixUnionSecond;
  }
}

TopolErrorClose::TopolErrorClose( QgsRectangle theBoundingBox, QgsGeometry* theConflict, QList<FeatureLayer> theFeaturePairs ) : TopolError( theBoundingBox, theConflict, theFeaturePairs )
{
  mName = "features too close";

  mFixMap["Move blue feature"] = &TopolErrorClose::fixMoveFirst;
  mFixMap["Move red feature"] = &TopolErrorClose::fixMoveSecond;
  mFixMap["Snap to segment"] = &TopolErrorClose::fixSnap;
}

TopolErrorCovered::TopolErrorCovered( QgsRectangle theBoundingBox, QgsGeometry* theConflict, QList<FeatureLayer> theFeaturePairs ) : TopolError( theBoundingBox, theConflict, theFeaturePairs )
{
  mName = "point not covered by segment";
  mFixMap["Delete point"] = &TopolErrorCovered::fixDeleteFirst;
}

TopolErrorShort::TopolErrorShort( QgsRectangle theBoundingBox, QgsGeometry* theConflict, QList<FeatureLayer> theFeaturePairs ) : TopolError( theBoundingBox, theConflict, theFeaturePairs )
{
  mName = "segment too short";
  mFixMap["Delete feature"] = &TopolErrorShort::fixDeleteFirst;
}

TopolErrorValid::TopolErrorValid( QgsRectangle theBoundingBox, QgsGeometry* theConflict, QList<FeatureLayer> theFeaturePairs ) : TopolError( theBoundingBox, theConflict, theFeaturePairs )
{
  mName = "invalid geometry";
  mFixMap["Delete feature"] = &TopolErrorValid::fixDeleteFirst;
}

TopolErrorDangle::TopolErrorDangle( QgsRectangle theBoundingBox, QgsGeometry* theConflict, QList<FeatureLayer> theFeaturePairs ) : TopolError( theBoundingBox, theConflict, theFeaturePairs )
{
  mName = "dangling end";
  mFixMap["Delete feature"] = &TopolErrorDangle::fixDeleteFirst;
}

TopolErrorDuplicates::TopolErrorDuplicates( QgsRectangle theBoundingBox, QgsGeometry* theConflict, QList<FeatureLayer> theFeaturePairs ) : TopolError( theBoundingBox, theConflict, theFeaturePairs )
{
  mName = "duplicate geometry";
  //mFixMap["Delete feature"] = &TopolErrorDuplicates::fixDeleteFirst;
}

TopolErrorPseudos::TopolErrorPseudos( QgsRectangle theBoundingBox, QgsGeometry* theConflict, QList<FeatureLayer> theFeaturePairs ) : TopolError( theBoundingBox, theConflict, theFeaturePairs )
{
  mName = "pseudo node";
  //mFixMap["Delete feature"] = &TopolErrorDuplicates::fixDeleteFirst;
}

TopolErrorOverlaps::TopolErrorOverlaps( QgsRectangle theBoundingBox, QgsGeometry* theConflict, QList<FeatureLayer> theFeaturePairs ) : TopolError( theBoundingBox, theConflict, theFeaturePairs )
{
  mName = "overlaps";
  //mFixMap["Delete feature"] = &TopolErrorDuplicates::fixDeleteFirst;
}

TopolErrorGaps::TopolErrorGaps( QgsRectangle theBoundingBox, QgsGeometry* theConflict, QList<FeatureLayer> theFeaturePairs ) : TopolError( theBoundingBox, theConflict, theFeaturePairs )
{
  mName = "gaps";
  //mFixMap["Delete feature"] = &TopolErrorDuplicates::fixDeleteFirst;
}

TopolErrorPointNotCoveredByLineEnds::TopolErrorPointNotCoveredByLineEnds( QgsRectangle theBoundingBox, QgsGeometry* theConflict, QList<FeatureLayer> theFeaturePairs ) : TopolError( theBoundingBox, theConflict, theFeaturePairs )
{
  mName = "point not covered";
}

TopolErrorLineEndsNotCoveredByPoints::TopolErrorLineEndsNotCoveredByPoints( QgsRectangle theBoundingBox, QgsGeometry* theConflict, QList<FeatureLayer> theFeaturePairs ) : TopolError( theBoundingBox, theConflict, theFeaturePairs )
{
  mName = "line ends not covered by point";
}

TopolErrorPointNotInPolygon::TopolErrorPointNotInPolygon( QgsRectangle theBoundingBox, QgsGeometry* theConflict, QList<FeatureLayer> theFeaturePairs ) : TopolError( theBoundingBox, theConflict, theFeaturePairs )
{
  mName = "point not in polygon";
}

TopolErrorPolygonContainsPoint::TopolErrorPolygonContainsPoint( QgsRectangle theBoundingBox, QgsGeometry* theConflict, QList<FeatureLayer> theFeaturePairs ) : TopolError( theBoundingBox, theConflict, theFeaturePairs )
{
  mName = "polygon does not contain point";
}

TopolErroMultiPart::TopolErroMultiPart( QgsRectangle theBoundingBox, QgsGeometry* theConflict, QList<FeatureLayer> theFeaturePairs ) : TopolError( theBoundingBox, theConflict, theFeaturePairs )
{
  mName = "multipart feature";
}
