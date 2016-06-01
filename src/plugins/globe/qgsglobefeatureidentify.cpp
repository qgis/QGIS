/***************************************************************************
    qgsglobefeatureidentify.cpp
     --------------------------------------
    Date                 : 27.10.2013
    Copyright            : (C) 2013 Matthias Kuhn
    Email                : matthias dot kuhn at gmx dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsglobefeatureidentify.h"
#include "qgsmapcanvas.h"
#include <qgsmaplayerregistry.h>
#include <qgsrubberband.h>
#include <qgsvectorlayer.h>

#include "featuresource/qgsglobefeaturesource.h"

#include <osg/ValueObject>
#include <osgEarth/Registry>

QgsGlobeFeatureIdentifyCallback::QgsGlobeFeatureIdentifyCallback( QgsMapCanvas* mapCanvas )
    : mCanvas( mapCanvas ), mRubberBand( new QgsRubberBand( mapCanvas, QGis::Polygon ) )
{
  QColor color( Qt::green );
  color.setAlpha( 190 );

  mRubberBand->setColor( color );
}

QgsGlobeFeatureIdentifyCallback::~QgsGlobeFeatureIdentifyCallback()
{
  mCanvas->scene()->removeItem( mRubberBand );
  delete mRubberBand;
}

#if OSGEARTH_VERSION_LESS_THAN(2, 7, 0)
void QgsGlobeFeatureIdentifyCallback::onHit( osgEarth::Features::FeatureSourceIndexNode* index, osgEarth::Features::FeatureID fid, const EventArgs& /*args*/ )
{
  QgsGlobeFeatureSource* globeSource = dynamic_cast<QgsGlobeFeatureSource*>( index->getFeatureSource() );
  if ( globeSource )
  {
    QgsVectorLayer* lyr = globeSource->layer();

#else
void QgsGlobeFeatureIdentifyCallback::onHit( osgEarth::ObjectID id )
{
  osgEarth::Features::FeatureIndex* index = osgEarth::Registry::objectIndex()->get<osgEarth::Features::FeatureIndex>( id );
  osgEarth::Features::Feature* feature = index->getFeature( id );
  osgEarth::Features::FeatureID fid = feature->getFID();
  std::string layerId;
  if ( feature->getUserValue( "qgisLayerId", layerId ) )
  {
    QgsVectorLayer* lyr = qobject_cast<QgsVectorLayer*>( QgsMapLayerRegistry::instance()->mapLayer( QString::fromStdString( layerId ) ) );
#endif
    if ( lyr )
    {
      QgsFeature feat;
      lyr->getFeatures( QgsFeatureRequest().setFilterFid( fid ) ).nextFeature( feat );

      if ( feat.isValid() )
        mRubberBand->setToGeometry( feat.geometry(), lyr );
      else
        mRubberBand->reset( QGis::Polygon );
    }
  }
  else
  {
    QgsDebugMsg( "Clicked feature was not on a QGIS layer" );
  }

}

#if OSGEARTH_VERSION_LESS_THAN(2, 7, 0)
void QgsGlobeFeatureIdentifyCallback::onMiss( const EventArgs &/*args*/ )
#else
void QgsGlobeFeatureIdentifyCallback::onMiss()
#endif
{
  mRubberBand->reset( QGis::Polygon );
}
