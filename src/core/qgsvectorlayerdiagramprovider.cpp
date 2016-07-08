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

#include "qgslabelsearchtree.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayerfeatureiterator.h"
#include "diagram/qgsdiagram.h"

#include "feature.h"
#include "labelposition.h"


QgsVectorLayerDiagramProvider::QgsVectorLayerDiagramProvider(
  const QgsDiagramLayerSettings* diagSettings,
  const QgsDiagramRendererV2* diagRenderer,
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
  mPriority = 1 - mSettings.getPriority() / 10.0; // convert 0..10 --> 1..0
  mPlacement = QgsPalLayerSettings::Placement( mSettings.getPlacement() );
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

  QStringList attributeNames;
  if ( !prepare( context, attributeNames ) )
    return QList<QgsLabelFeature*>();

  QgsRectangle layerExtent = context.extent();
  if ( mSettings.coordinateTransform() )
    layerExtent = mSettings.coordinateTransform()->transformBoundingBox( context.extent(), QgsCoordinateTransform::ReverseTransform );

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
  feature.setFeatureId( label->getFeaturePart()->featureId() );
  feature.setAttributes( dlf->attributes() );

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

  mSettings.getRenderer()->renderDiagram( feature, context, centerPt.toQPointF() );

  //insert into label search tree to manipulate position interactively
  mEngine->results()->mLabelSearchTree->insertLabel( label, label->getFeaturePart()->featureId(), mLayerId, QString(), QFont(), true, false );

}


bool QgsVectorLayerDiagramProvider::prepare( const QgsRenderContext& context, QStringList& attributeNames )
{
  QgsDiagramLayerSettings& s2 = mSettings;
  const QgsMapSettings& mapSettings = mEngine->mapSettings();

  if ( mapSettings.hasCrsTransformEnabled() )
  {
    if ( context.coordinateTransform() )
      // this is context for layer rendering - use its CT as it includes correct datum transform
      s2.setCoordinateTransform( context.coordinateTransform()->clone() );
    else
      // otherwise fall back to creating our own CT - this one may not have the correct datum transform!
      s2.setCoordinateTransform( new QgsCoordinateTransform( mLayerCrs, mapSettings.destinationCrs() ) );
  }
  else
  {
    s2.setCoordinateTransform( nullptr );
  }

  s2.setRenderer( mDiagRenderer );

  //add attributes needed by the diagram renderer
  Q_FOREACH ( const QString& field, s2.referencedFields( context.expressionContext(), mFields ) )
  {
    if ( !attributeNames.contains( field ) )
      attributeNames << field;
  }

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

  const QgsDiagramRendererV2* dr = mSettings.getRenderer();
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

  //convert geom to geos
  const QgsGeometry* geom = feat.constGeometry();
  QScopedPointer<QgsGeometry> extentGeom( QgsGeometry::fromRect( mapSettings.visibleExtent() ) );
  if ( !qgsDoubleNear( mapSettings.rotation(), 0.0 ) )
  {
    //PAL features are prerotated, so extent also needs to be unrotated
    extentGeom->rotate( -mapSettings.rotation(), mapSettings.visibleExtent().center() );
  }

  const GEOSGeometry* geos_geom = nullptr;
  QScopedPointer<QgsGeometry> preparedGeom;
  if ( QgsPalLabeling::geometryRequiresPreparation( geom, context, mSettings.coordinateTransform(), extentGeom.data() ) )
  {
    preparedGeom.reset( QgsPalLabeling::prepareGeometry( geom, context, mSettings.coordinateTransform(), extentGeom.data() ) );
    if ( !preparedGeom.data() )
      return nullptr;
    geos_geom = preparedGeom.data()->asGeos();
  }
  else
  {
    geos_geom = geom->asGeos();
  }

  if ( !geos_geom )
    return nullptr; // invalid geometry

  GEOSGeometry* geomCopy = GEOSGeom_clone_r( QgsGeometry::getGEOSHandler(), geos_geom );

  const GEOSGeometry* geosObstacleGeom = nullptr;
  QScopedPointer<QgsGeometry> scopedObstacleGeom;
  if ( mSettings.isObstacle() && obstacleGeometry && QgsPalLabeling::geometryRequiresPreparation( obstacleGeometry, context, mSettings.coordinateTransform(), extentGeom.data() ) )
  {
    scopedObstacleGeom.reset( QgsPalLabeling::prepareGeometry( obstacleGeometry, context, mSettings.coordinateTransform(), extentGeom.data() ) );
    geosObstacleGeom = scopedObstacleGeom.data()->asGeos();
  }
  else if ( mSettings.isObstacle() && obstacleGeometry )
  {
    geosObstacleGeom = obstacleGeometry->asGeos();
  }
  GEOSGeometry* geosObstacleGeomClone = nullptr;
  if ( geosObstacleGeom )
  {
    geosObstacleGeomClone = GEOSGeom_clone_r( QgsGeometry::getGEOSHandler(), geosObstacleGeom );
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
    else
    {
      const QgsCoordinateTransform* ct = mSettings.coordinateTransform();
      if ( ct )
      {
        double z = 0;
        ct->transformInPlace( ddPosX, ddPosY, z );
      }
      //data defined diagram position is always centered
      ddPosX -= diagramWidth / 2.0;
      ddPosY -= diagramHeight / 2.0;
    }
  }
  else
    ddPos = false;

  int ddColShow = mSettings.showColumn;
  if ( ddColShow >= 0 && ! feat.attribute( ddColShow ).isNull() )
  {
    bool showOk;
    bool ddShow = feat.attribute( ddColShow ).toDouble( &showOk );

    if ( showOk && ! ddShow )
      return nullptr;
  }

  QgsDiagramLabelFeature* lf = new QgsDiagramLabelFeature( feat.id(), geomCopy, QSizeF( diagramWidth, diagramHeight ) );
  lf->setHasFixedPosition( ddPos );
  lf->setFixedPosition( QgsPoint( ddPosX, ddPosY ) );
  lf->setHasFixedAngle( true );
  lf->setFixedAngle( 0 );
  lf->setAlwaysShow( alwaysShow );
  lf->setIsObstacle( mSettings.isObstacle() );
  lf->setZIndex( mSettings.getZIndex() );
  if ( geosObstacleGeomClone )
  {
    lf->setObstacleGeometry( geosObstacleGeomClone );
  }

  if ( dr )
  {
    //append the diagram attributes to lbl
    lf->setAttributes( feat.attributes() );
  }

  QgsPoint ptZero = mapSettings.mapToPixel().toMapCoordinates( 0, 0 );
  QgsPoint ptOne = mapSettings.mapToPixel().toMapCoordinates( 1, 0 );
  lf->setDistLabel( ptOne.distance( ptZero ) * mSettings.distance() );
  return lf;
}
