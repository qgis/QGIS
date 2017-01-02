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

#include "feature.h"
#include "labelposition.h"


QgsVectorLayerDiagramProvider::QgsVectorLayerDiagramProvider(
  const QgsDiagramLayerSettings* diagSettings,
  const QgsDiagramRenderer* diagRenderer,
  const QString& layerId,
  const QgsFields& fields,
  const QgsCoordinateReferenceSystem& crs,
  QgsAbstractFeatureSource* source,
  bool ownsSource )
    : QgsAbstractLabelProvider( layerId )
    , mSettings( *diagSettings )
    , mDiagRenderer( diagRenderer->clone() )
    , mFields( fields )
    , mLayerCrs( crs )
    , mSource( source )
    , mOwnsSource( ownsSource )
{
  init();
}


QgsVectorLayerDiagramProvider::QgsVectorLayerDiagramProvider( QgsVectorLayer* layer, bool ownFeatureLoop )
    : QgsAbstractLabelProvider( layer->id() )
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
  mLinePlacementFlags = mSettings.linePlacementFlags();
}


QgsVectorLayerDiagramProvider::~QgsVectorLayerDiagramProvider()
{
  if ( mOwnsSource )
    delete mSource;

  qDeleteAll( mFeatures );

  // renderer is owned by mSettings
}


QList<QgsLabelFeature*> QgsVectorLayerDiagramProvider::labelFeatures( QgsRenderContext &context )
{
  if ( !mSource )
  {
    // we have created the provider with "own feature loop" == false
    // so it is assumed that prepare() has been already called followed by registerFeature() calls
    return mFeatures;
  }

  QSet<QString> attributeNames;
  if ( !prepare( context, attributeNames ) )
    return QList<QgsLabelFeature*>();

  QgsRectangle layerExtent = context.extent();
  if ( mSettings.coordinateTransform().isValid() )
    layerExtent = mSettings.coordinateTransform().transformBoundingBox( context.extent(), QgsCoordinateTransform::ReverseTransform );

  QgsFeatureRequest request;
  request.setFilterRect( layerExtent );
  request.setSubsetOfAttributes( attributeNames, mFields );
  QgsFeatureIterator fit = mSource->getFeatures( request );


  QgsFeature fet;
  while ( fit.nextFeature( fet ) )
  {
    registerFeature( fet, context );
  }

  return mFeatures;
}


void QgsVectorLayerDiagramProvider::drawLabel( QgsRenderContext& context, pal::LabelPosition* label ) const
{
#if 1 // XXX strk
  // features are pre-rotated but not scaled/translated,
  // so we only disable rotation here. Ideally, they'd be
  // also pre-scaled/translated, as suggested here:
  // http://hub.qgis.org/issues/11856
  QgsMapToPixel xform = context.mapToPixel();
  xform.setMapRotation( 0, 0, 0 );
#else
  const QgsMapToPixel& xform = context.mapToPixel();
#endif

  QgsDiagramLabelFeature* dlf = dynamic_cast<QgsDiagramLabelFeature*>( label->getFeaturePart()->feature() );

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
  QgsPoint outPt( centerX / 4.0, centerY / 4.0 );
  //then, calculate the top left point for the diagram with this center position
  QgsPoint centerPt = xform.transform( outPt.x() - label->getWidth() / 2,
                                       outPt.y() - label->getHeight() / 2 );

  mSettings.renderer()->renderDiagram( feature, context, centerPt.toQPointF(), mSettings.properties() );

  //insert into label search tree to manipulate position interactively
  mEngine->results()->mLabelSearchTree->insertLabel( label, label->getFeaturePart()->featureId(), mLayerId, QString(), QFont(), true, false );

}


bool QgsVectorLayerDiagramProvider::prepare( const QgsRenderContext& context, QSet<QString>& attributeNames )
{
  QgsDiagramLayerSettings& s2 = mSettings;
  const QgsMapSettings& mapSettings = mEngine->mapSettings();

  if ( mapSettings.hasCrsTransformEnabled() )
  {
    if ( context.coordinateTransform().isValid() )
      // this is context for layer rendering - use its CT as it includes correct datum transform
      s2.setCoordinateTransform( context.coordinateTransform() );
    else
      // otherwise fall back to creating our own CT - this one may not have the correct datum transform!
      s2.setCoordinateTransform( QgsCoordinateTransform( mLayerCrs, mapSettings.destinationCrs() ) );
  }
  else
  {
    s2.setCoordinateTransform( QgsCoordinateTransform() );
  }

  s2.setRenderer( mDiagRenderer );

  //add attributes needed by the diagram renderer
  attributeNames.unite( s2.referencedFields( context.expressionContext(), mFields ) );

  return true;
}


void QgsVectorLayerDiagramProvider::registerFeature( QgsFeature& feature, QgsRenderContext& context, QgsGeometry* obstacleGeometry )
{
  QgsLabelFeature* label = registerDiagram( feature, context, obstacleGeometry );
  if ( label )
    mFeatures << label;
}


QgsLabelFeature* QgsVectorLayerDiagramProvider::registerDiagram( QgsFeature& feat, QgsRenderContext &context, QgsGeometry* obstacleGeometry )
{
  const QgsMapSettings& mapSettings = mEngine->mapSettings();

  const QgsDiagramRenderer* dr = mSettings.renderer();
  if ( dr )
  {
    QList<QgsDiagramSettings> settingList = dr->diagramSettings();
    if ( !settingList.isEmpty() && settingList.at( 0 ).scaleBasedVisibility )
    {
      double minScale = settingList.at( 0 ).minScaleDenominator;
      if ( minScale > 0 && context.rendererScale() < minScale )
      {
        return nullptr;
      }

      double maxScale = settingList.at( 0 ).maxScaleDenominator;
      if ( maxScale > 0 && context.rendererScale() > maxScale )
      {
        return nullptr;
      }
    }
  }

  // data defined show diagram? check this before doing any other processing
  if ( mSettings.properties().hasProperty( QgsDiagramLayerSettings::Show )
       && mSettings.properties().property( QgsDiagramLayerSettings::Show )->isActive() )
  {
    bool show = mSettings.properties().valueAsInt( QgsDiagramLayerSettings::Show, context.expressionContext(), true );
    if ( !show )
      return nullptr;
  }

  // data defined obstacle?
  bool isObstacle = mSettings.isObstacle();
  if ( mSettings.properties().hasProperty( QgsDiagramLayerSettings::IsObstacle )
       && mSettings.properties().property( QgsDiagramLayerSettings::IsObstacle )->isActive() )
  {
    isObstacle = mSettings.properties().valueAsInt( QgsDiagramLayerSettings::IsObstacle, context.expressionContext(), isObstacle );
  }

  //convert geom to geos
  QgsGeometry geom = feat.geometry();
  QgsGeometry extentGeom = QgsGeometry::fromRect( mapSettings.visibleExtent() );
  if ( !qgsDoubleNear( mapSettings.rotation(), 0.0 ) )
  {
    //PAL features are prerotated, so extent also needs to be unrotated
    extentGeom.rotate( -mapSettings.rotation(), mapSettings.visibleExtent().center() );
  }

  GEOSGeometry* geomCopy = nullptr;
  QScopedPointer<QgsGeometry> scopedPreparedGeom;
  if ( QgsPalLabeling::geometryRequiresPreparation( geom, context, mSettings.coordinateTransform(), &extentGeom ) )
  {
    scopedPreparedGeom.reset( new QgsGeometry( QgsPalLabeling::prepareGeometry( geom, context, mSettings.coordinateTransform(), &extentGeom ) ) );
    QgsGeometry* preparedGeom = scopedPreparedGeom.data();
    if ( preparedGeom->isEmpty() )
      return nullptr;
    geomCopy = preparedGeom->exportToGeos();
  }
  else
  {
    geomCopy = geom.exportToGeos();
  }

  if ( !geomCopy )
    return nullptr; // invalid geometry

  GEOSGeometry* geosObstacleGeomClone = nullptr;
  QScopedPointer<QgsGeometry> scopedObstacleGeom;
  if ( isObstacle && obstacleGeometry && QgsPalLabeling::geometryRequiresPreparation( *obstacleGeometry, context, mSettings.coordinateTransform(), &extentGeom ) )
  {
    QgsGeometry preparedObstacleGeom = QgsPalLabeling::prepareGeometry( *obstacleGeometry, context, mSettings.coordinateTransform(), &extentGeom );
    geosObstacleGeomClone = preparedObstacleGeom.exportToGeos();
  }
  else if ( mSettings.isObstacle() && obstacleGeometry )
  {
    geosObstacleGeomClone = obstacleGeometry->exportToGeos();
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
  if ( mSettings.properties().hasProperty( QgsDiagramLayerSettings::AlwaysShow )
       && mSettings.properties().property( QgsDiagramLayerSettings::AlwaysShow )->isActive() )
  {
    context.expressionContext().setOriginalValueVariable( alwaysShow );
    alwaysShow = mSettings.properties().valueAsInt( QgsDiagramLayerSettings::AlwaysShow, context.expressionContext(), alwaysShow );
  }

  // old style show
  int ddColShow = mSettings.showColumn;
  if ( ddColShow >= 0 && ! feat.attribute( ddColShow ).isNull() )
  {
    bool showOk;
    bool ddShow = feat.attribute( ddColShow ).toDouble( &showOk );

    if ( showOk && ! ddShow )
      return nullptr;
  }

  // old style data defined position
  // TODO - remove when xPosColumn and yPosColumn are removed from QgsDiagramLayerSettings
  int ddColX = mSettings.xPosColumn;
  int ddColY = mSettings.yPosColumn;
  double ddPosX = 0.0;
  double ddPosY = 0.0;
  bool ddPos = ( ddColX >= 0 && ddColY >= 0 );
  if ( ddPos && ! feat.attribute( ddColX ).isNull() && ! feat.attribute( ddColY ).isNull() )
  {
    bool posXOk, posYOk;
    ddPosX = feat.attribute( ddColX ).toDouble( &posXOk );
    ddPosY = feat.attribute( ddColY ).toDouble( &posYOk );
    if ( !posXOk || !posYOk )
    {
      ddPos = false;
    }
  }

  // new style data defined position
  if ( mSettings.properties().hasProperty( QgsDiagramLayerSettings::PositionX )
       && mSettings.properties().property( QgsDiagramLayerSettings::PositionX )->isActive()
       && mSettings.properties().hasProperty( QgsDiagramLayerSettings::PositionY )
       && mSettings.properties().property( QgsDiagramLayerSettings::PositionY )->isActive() )
  {
    ddPosX = mSettings.properties().valueAsDouble( QgsDiagramLayerSettings::PositionX, context.expressionContext(), ddPosX );
    ddPosY = mSettings.properties().valueAsDouble( QgsDiagramLayerSettings::PositionY, context.expressionContext(), ddPosY );
    ddPos = true;
  }

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


  QgsDiagramLabelFeature* lf = new QgsDiagramLabelFeature( feat.id(), geomCopy, QSizeF( diagramWidth, diagramHeight ) );
  lf->setHasFixedPosition( ddPos );
  lf->setFixedPosition( QgsPoint( ddPosX, ddPosY ) );
  lf->setHasFixedAngle( true );
  lf->setFixedAngle( 0 );
  lf->setAlwaysShow( alwaysShow );
  lf->setIsObstacle( isObstacle );
  if ( geosObstacleGeomClone )
  {
    lf->setObstacleGeometry( geosObstacleGeomClone );
  }

  if ( dr )
  {
    //append the diagram attributes to lbl
    lf->setAttributes( feat.attributes() );
  }

  // data defined priority?
  if ( mSettings.properties().hasProperty( QgsDiagramLayerSettings::Priority )
       && mSettings.properties().property( QgsDiagramLayerSettings::Priority )->isActive() )
  {
    context.expressionContext().setOriginalValueVariable( mSettings.priority() );
    double priorityD = mSettings.properties().valueAsDouble( QgsDiagramLayerSettings::Priority, context.expressionContext(), mSettings.getPriority() );
    priorityD = qBound( 0.0, priorityD, 10.0 );
    priorityD = 1 - priorityD / 10.0; // convert 0..10 --> 1..0
    lf->setPriority( priorityD );
  }

  // z-Index
  double zIndex = mSettings.zIndex();
  if ( mSettings.properties().hasProperty( QgsDiagramLayerSettings::ZIndex )
       && mSettings.properties().property( QgsDiagramLayerSettings::ZIndex )->isActive() )
  {
    context.expressionContext().setOriginalValueVariable( zIndex );
    zIndex = mSettings.properties().valueAsDouble( QgsDiagramLayerSettings::ZIndex, context.expressionContext(), zIndex );
  }
  lf->setZIndex( zIndex );

  // label distance
  QgsPoint ptZero = mapSettings.mapToPixel().toMapCoordinates( 0, 0 );
  QgsPoint ptOne = mapSettings.mapToPixel().toMapCoordinates( 1, 0 );
  double dist = mSettings.distance();

  if ( mSettings.properties().hasProperty( QgsDiagramLayerSettings::Distance )
       && mSettings.properties().property( QgsDiagramLayerSettings::Distance )->isActive() )
  {
    context.expressionContext().setOriginalValueVariable( dist );
    dist = mSettings.properties().valueAsDouble( QgsDiagramLayerSettings::Distance, context.expressionContext(), dist );
  }

  dist *= ptOne.distance( ptZero );

  lf->setDistLabel( dist );
  return lf;
}
