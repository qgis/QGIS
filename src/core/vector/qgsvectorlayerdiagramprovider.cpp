/***************************************************************************
  qgsvectorlayerdiagramprovider.cpp
  --------------------------------------
  Date                 : September 2015
  Copyright            : (C) 2015 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsvectorlayerdiagramprovider.h"

#include "qgsgeometry.h"
#include "qgslabelsearchtree.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayerfeatureiterator.h"
#include "diagram/qgsdiagram.h"
#include "qgsgeos.h"
#include "qgslabelingresults.h"
#include "qgsrendercontext.h"

#include "feature.h"
#include "labelposition.h"

QgsVectorLayerDiagramProvider::QgsVectorLayerDiagramProvider( QgsVectorLayer *layer, bool ownFeatureLoop )
  : QgsAbstractLabelProvider( layer )
  , mSettings( *layer->diagramLayerSettings() )
  , mDiagRenderer( layer->diagramRenderer()->clone() )
  , mFields( layer->fields() )
  , mLayerCrs( layer->crs() )
  , mSource( ownFeatureLoop ? new QgsVectorLayerFeatureSource( layer ) : nullptr )
  , mOwnsSource( ownFeatureLoop )
{
  init();
}


void QgsVectorLayerDiagramProvider::init()
{
  mName = mLayerId;
  mPriority = 1 - mSettings.priority() / 10.0; // convert 0..10 --> 1..0
  mPlacement = QgsPalLayerSettings::Placement( mSettings.placement() );
}


QgsVectorLayerDiagramProvider::~QgsVectorLayerDiagramProvider()
{
  if ( mOwnsSource )
    delete mSource;

  qDeleteAll( mFeatures );

  // renderer is owned by mSettings
}


QList<QgsLabelFeature *> QgsVectorLayerDiagramProvider::labelFeatures( QgsRenderContext &context )
{
  if ( !mSource )
  {
    // we have created the provider with "own feature loop" == false
    // so it is assumed that prepare() has been already called followed by registerFeature() calls
    return mFeatures;
  }

  QSet<QString> attributeNames;
  if ( !prepare( context, attributeNames ) )
    return QList<QgsLabelFeature *>();

  QgsRectangle layerExtent = context.extent();
  if ( !mSettings.coordinateTransform().isShortCircuited() )
  {
    QgsCoordinateTransform extentTransform = mSettings.coordinateTransform();
    extentTransform.setBallparkTransformsAreAppropriate( true );
    layerExtent = extentTransform.transformBoundingBox( context.extent(), Qgis::TransformDirection::Reverse );
  }

  QgsFeatureRequest request;
  request.setFilterRect( layerExtent );
  request.setSubsetOfAttributes( attributeNames, mFields );
  const QgsFeatureFilterProvider *featureFilterProvider = context.featureFilterProvider();
  if ( featureFilterProvider )
  {
    featureFilterProvider->filterFeatures( qobject_cast<QgsVectorLayer *>( mLayer ), request );
  }
  QgsFeatureIterator fit = mSource->getFeatures( request );

  QgsFeature fet;
  while ( fit.nextFeature( fet ) )
  {
    context.expressionContext().setFeature( fet );
    registerFeature( fet, context );
  }

  return mFeatures;
}


void QgsVectorLayerDiagramProvider::drawLabel( QgsRenderContext &context, pal::LabelPosition *label ) const
{
#if 1 // XXX strk
  // features are pre-rotated but not scaled/translated,
  // so we only disable rotation here. Ideally, they'd be
  // also pre-scaled/translated, as suggested here:
  // https://github.com/qgis/QGIS/issues/20071
  QgsMapToPixel xform = context.mapToPixel();
  xform.setMapRotation( 0, 0, 0 );
#else
  const QgsMapToPixel &xform = context.mapToPixel();
#endif

  QgsDiagramLabelFeature *dlf = dynamic_cast<QgsDiagramLabelFeature *>( label->getFeaturePart()->feature() );

  QgsFeature feature;
  feature.setFields( mFields );
  feature.setValid( true );
  feature.setId( label->getFeaturePart()->featureId() );
  feature.setAttributes( dlf->attributes() );

  context.expressionContext().setFeature( feature );

  //calculate top-left point for diagram
  //first, calculate the centroid of the label (accounts for PAL creating
  //rotated labels when we do not want to draw the diagrams rotated)
  double centerX = 0;
  double centerY = 0;
  for ( int i = 0; i < 4; ++i )
  {
    centerX += label->getX( i );
    centerY += label->getY( i );
  }
  QgsPointXY outPt( centerX / 4.0, centerY / 4.0 );
  //then, calculate the top left point for the diagram with this center position
  QgsPointXY centerPt = xform.transform( outPt.x() - label->getWidth() / 2,
                                         outPt.y() - label->getHeight() / 2 );

  mSettings.renderer()->renderDiagram( feature, context, centerPt.toQPointF(), mSettings.dataDefinedProperties() );

  //insert into label search tree to manipulate position interactively
  mEngine->results()->mLabelSearchTree->insertLabel( label, label->getFeaturePart()->featureId(), mLayerId, QString(), QFont(), true, false );

}


bool QgsVectorLayerDiagramProvider::prepare( const QgsRenderContext &context, QSet<QString> &attributeNames )
{
  QgsDiagramLayerSettings &s2 = mSettings;
  const QgsMapSettings &mapSettings = mEngine->mapSettings();

  if ( context.coordinateTransform().isValid() )
    // this is context for layer rendering
    s2.setCoordinateTransform( context.coordinateTransform() );
  else
  {
    // otherwise fall back to creating our own CT
    s2.setCoordinateTransform( QgsCoordinateTransform( mLayerCrs, mapSettings.destinationCrs(), context.transformContext() ) );
  }

  s2.setRenderer( mDiagRenderer );

  bool result = s2.prepare( context.expressionContext() );

  //add attributes needed by the diagram renderer
  attributeNames.unite( s2.referencedFields( context.expressionContext() ) );

  return result;
}


void QgsVectorLayerDiagramProvider::registerFeature( QgsFeature &feature, QgsRenderContext &context, const QgsGeometry &obstacleGeometry )
{
  QgsLabelFeature *label = registerDiagram( feature, context, obstacleGeometry );
  if ( label )
    mFeatures << label;
}

void QgsVectorLayerDiagramProvider::setClipFeatureGeometry( const QgsGeometry &geometry )
{
  mLabelClipFeatureGeom = geometry;
}

QgsLabelFeature *QgsVectorLayerDiagramProvider::registerDiagram( QgsFeature &feat, QgsRenderContext &context, const QgsGeometry &obstacleGeometry )
{
  const QgsMapSettings &mapSettings = mEngine->mapSettings();

  const QgsDiagramRenderer *dr = mSettings.renderer();
  if ( dr )
  {
    QList<QgsDiagramSettings> settingList = dr->diagramSettings();
    if ( !settingList.isEmpty() && settingList.at( 0 ).scaleBasedVisibility )
    {
      double maxScale = settingList.at( 0 ).maximumScale;
      if ( maxScale > 0 && context.rendererScale() < maxScale )
      {
        return nullptr;
      }

      double minScale = settingList.at( 0 ).minimumScale;
      if ( minScale > 0 && context.rendererScale() > minScale )
      {
        return nullptr;
      }
    }
  }

  // data defined show diagram? check this before doing any other processing
  if ( !mSettings.dataDefinedProperties().valueAsBool( QgsDiagramLayerSettings::Show, context.expressionContext(), true ) )
    return nullptr;

  // data defined obstacle?
  bool isObstacle = mSettings.dataDefinedProperties().valueAsBool( QgsDiagramLayerSettings::IsObstacle, context.expressionContext(), mSettings.isObstacle() );

  //convert geom to geos
  QgsGeometry geom = feat.geometry();
  QgsGeometry extentGeom = QgsGeometry::fromRect( mapSettings.visibleExtent() );
  if ( !qgsDoubleNear( mapSettings.rotation(), 0.0 ) )
  {
    //PAL features are prerotated, so extent also needs to be unrotated
    extentGeom.rotate( -mapSettings.rotation(), mapSettings.visibleExtent().center() );
  }

  if ( QgsPalLabeling::geometryRequiresPreparation( geom, context, mSettings.coordinateTransform(), extentGeom ) )
  {
    geom = QgsPalLabeling::prepareGeometry( geom, context, mSettings.coordinateTransform(), extentGeom );
  }
  if ( geom.isEmpty() )
    return nullptr;

  const QgsGeometry clipGeometry = mLabelClipFeatureGeom.isNull() ? context.featureClipGeometry() : mLabelClipFeatureGeom;
  if ( !clipGeometry.isEmpty() )
  {
    const QgsWkbTypes::GeometryType expectedType = geom.type();
    geom = geom.intersection( clipGeometry );
    geom.convertGeometryCollectionToSubclass( expectedType );
  }
  if ( geom.isEmpty() )
    return nullptr;

  QgsGeometry preparedObstacleGeom;
  if ( isObstacle && !obstacleGeometry.isNull() && QgsPalLabeling::geometryRequiresPreparation( obstacleGeometry, context, mSettings.coordinateTransform(), extentGeom ) )
  {
    preparedObstacleGeom = QgsPalLabeling::prepareGeometry( obstacleGeometry, context, mSettings.coordinateTransform(), extentGeom );
  }
  else if ( mSettings.isObstacle() && !obstacleGeometry.isNull() )
  {
    preparedObstacleGeom = obstacleGeometry;
  }

  double diagramWidth = 0;
  double diagramHeight = 0;
  if ( dr )
  {
    QSizeF diagSize = dr->sizeMapUnits( feat, context );
    if ( diagSize.isValid() )
    {
      diagramWidth = diagSize.width();
      diagramHeight = diagSize.height();
    }
  }

  //  feature to the layer
  bool alwaysShow = mSettings.showAllDiagrams();
  context.expressionContext().setOriginalValueVariable( alwaysShow );
  alwaysShow = mSettings.dataDefinedProperties().valueAsBool( QgsDiagramLayerSettings::AlwaysShow, context.expressionContext(), alwaysShow );

  // new style data defined position
  bool ddPos = false;
  double ddPosX = 0.0;
  double ddPosY = 0.0;
  if ( mSettings.dataDefinedProperties().hasProperty( QgsDiagramLayerSettings::PositionX )
       && mSettings.dataDefinedProperties().property( QgsDiagramLayerSettings::PositionX ).isActive()
       && mSettings.dataDefinedProperties().hasProperty( QgsDiagramLayerSettings::PositionY )
       && mSettings.dataDefinedProperties().property( QgsDiagramLayerSettings::PositionY ).isActive() )
  {
    ddPosX = mSettings.dataDefinedProperties().valueAsDouble( QgsDiagramLayerSettings::PositionX, context.expressionContext(), std::numeric_limits<double>::quiet_NaN() );
    ddPosY = mSettings.dataDefinedProperties().valueAsDouble( QgsDiagramLayerSettings::PositionY, context.expressionContext(), std::numeric_limits<double>::quiet_NaN() );

    ddPos = !std::isnan( ddPosX ) && !std::isnan( ddPosY );

    if ( ddPos )
    {
      QgsCoordinateTransform ct = mSettings.coordinateTransform();
      if ( ct.isValid() && !ct.isShortCircuited() )
      {
        double z = 0;
        ct.transformInPlace( ddPosX, ddPosY, z );
      }
      //data defined diagram position is always centered
      ddPosX -= diagramWidth / 2.0;
      ddPosY -= diagramHeight / 2.0;
    }
  }

  QgsDiagramLabelFeature *lf = new QgsDiagramLabelFeature( feat.id(), QgsGeos::asGeos( geom ), QSizeF( diagramWidth, diagramHeight ) );
  lf->setHasFixedPosition( ddPos );
  lf->setFixedPosition( QgsPointXY( ddPosX, ddPosY ) );
  lf->setHasFixedAngle( true );
  lf->setFixedAngle( 0 );
  lf->setAlwaysShow( alwaysShow );
  QgsLabelObstacleSettings os;
  os.setIsObstacle( isObstacle );
  os.setObstacleGeometry( preparedObstacleGeom );
  lf->setObstacleSettings( os );

  if ( dr )
  {
    //append the diagram attributes to lbl
    lf->setAttributes( feat.attributes() );
  }

  // data defined priority?
  if ( mSettings.dataDefinedProperties().hasProperty( QgsDiagramLayerSettings::Priority )
       && mSettings.dataDefinedProperties().property( QgsDiagramLayerSettings::Priority ).isActive() )
  {
    context.expressionContext().setOriginalValueVariable( mSettings.priority() );
    double priorityD = mSettings.dataDefinedProperties().valueAsDouble( QgsDiagramLayerSettings::Priority, context.expressionContext(), mSettings.priority() );
    priorityD = std::clamp( priorityD, 0.0, 10.0 );
    priorityD = 1 - priorityD / 10.0; // convert 0..10 --> 1..0
    lf->setPriority( priorityD );
  }

  // z-Index
  double zIndex = mSettings.zIndex();
  if ( mSettings.dataDefinedProperties().hasProperty( QgsDiagramLayerSettings::ZIndex )
       && mSettings.dataDefinedProperties().property( QgsDiagramLayerSettings::ZIndex ).isActive() )
  {
    context.expressionContext().setOriginalValueVariable( zIndex );
    zIndex = mSettings.dataDefinedProperties().valueAsDouble( QgsDiagramLayerSettings::ZIndex, context.expressionContext(), zIndex );
  }
  lf->setZIndex( zIndex );

  // label distance
  QgsPointXY ptZero = mapSettings.mapToPixel().toMapCoordinates( 0, 0 );
  QgsPointXY ptOne = mapSettings.mapToPixel().toMapCoordinates( 1, 0 );
  double dist = mSettings.distance();

  if ( mSettings.dataDefinedProperties().hasProperty( QgsDiagramLayerSettings::Distance )
       && mSettings.dataDefinedProperties().property( QgsDiagramLayerSettings::Distance ).isActive() )
  {
    context.expressionContext().setOriginalValueVariable( dist );
    dist = mSettings.dataDefinedProperties().valueAsDouble( QgsDiagramLayerSettings::Distance, context.expressionContext(), dist );
  }

  dist *= ptOne.distance( ptZero );

  lf->setDistLabel( dist );
  return lf;
}

