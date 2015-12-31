#include "qgslabelfeature.h"

#include "feature.h"


QgsLabelFeature::QgsLabelFeature( QgsFeatureId id, GEOSGeometry* geometry, const QSizeF& size )
    : mLayer( nullptr )
    , mId( id )
    , mGeometry( geometry )
    , mObstacleGeometry( nullptr )
    , mSize( size )
    , mPriority( -1 )
    , mZIndex( 0 )
    , mHasFixedPosition( false )
    , mHasFixedAngle( false )
    , mFixedAngle( 0 )
    , mHasFixedQuadrant( false )
    , mDistLabel( 0 )
    , mRepeatDistance( 0 )
    , mAlwaysShow( false )
    , mIsObstacle( false )
    , mObstacleFactor( 1 )
    , mInfo( nullptr )
{
}

QgsLabelFeature::~QgsLabelFeature()
{
  if ( mGeometry )
    GEOSGeom_destroy_r( QgsGeometry::getGEOSHandler(), mGeometry );

  if ( mObstacleGeometry )
    GEOSGeom_destroy_r( QgsGeometry::getGEOSHandler(), mObstacleGeometry );

  delete mInfo;
}

void QgsLabelFeature::setObstacleGeometry( GEOSGeometry* obstacleGeom )
{
  if ( mObstacleGeometry )
    GEOSGeom_destroy_r( QgsGeometry::getGEOSHandler(), mObstacleGeometry );

  mObstacleGeometry = obstacleGeom;
}
