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
    : mSettings( *diagSettings )
    , mDiagRenderer( diagRenderer->clone() )
    , mLayerId( layerId )
    , mFields( fields )
    , mLayerCrs( crs )
    , mSource( source )
    , mOwnsSource( ownsSource )
{
  init();
}


QgsVectorLayerDiagramProvider::QgsVectorLayerDiagramProvider( QgsVectorLayer* layer, bool ownFeatureLoop )
    : mSettings( *layer->diagramLayerSettings() )
    , mDiagRenderer( layer->diagramRenderer()->clone() )
    , mLayerId( layer->id() )
    , mFields( layer->fields() )
    , mLayerCrs( layer->crs() )
    , mSource( ownFeatureLoop ? new QgsVectorLayerFeatureSource( layer ) : 0 )
    , mOwnsSource( ownFeatureLoop )
{
  init();
}


void QgsVectorLayerDiagramProvider::init()
{
  mName = mLayerId;
  mPriority = 1 - mSettings.priority / 10.0; // convert 0..10 --> 1..0
  mPlacement = QgsPalLayerSettings::Placement( mSettings.placement );
  mLinePlacementFlags = mSettings.placementFlags;
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
  if ( mSettings.ct )
    layerExtent = mSettings.ct->transformBoundingBox( context.extent(), QgsCoordinateTransform::ReverseTransform );

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
  feature.setFields( mSettings.fields );
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

  mSettings.renderer->renderDiagram( feature, context, centerPt.toQPointF() );

  //insert into label search tree to manipulate position interactively
  mEngine->results()->mLabelSearchTree->insertLabel( label, label->getFeaturePart()->featureId(), mLayerId, QString(), QFont(), true, false );

}


bool QgsVectorLayerDiagramProvider::prepare( const QgsRenderContext& context, QStringList& attributeNames )
{
  QgsDiagramLayerSettings& s2 = mSettings;
  const QgsMapSettings& mapSettings = mEngine->mapSettings();

  s2.ct = 0;
  if ( mapSettings.hasCrsTransformEnabled() )
  {
    if ( context.coordinateTransform() )
      // this is context for layer rendering - use its CT as it includes correct datum transform
      s2.ct = context.coordinateTransform()->clone();
    else
      // otherwise fall back to creating our own CT - this one may not have the correct datum transform!
      s2.ct = new QgsCoordinateTransform( mLayerCrs, mapSettings.destinationCrs() );
  }

  s2.xform = &mapSettings.mapToPixel();

  s2.fields = mFields;

  s2.renderer = mDiagRenderer;

  const QgsDiagramRendererV2* diagRenderer = s2.renderer;

  //add attributes needed by the diagram renderer
  QList<QString> att = diagRenderer->diagramAttributes();
  QList<QString>::const_iterator attIt = att.constBegin();
  for ( ; attIt != att.constEnd(); ++attIt )
  {
    QgsExpression* expression = diagRenderer->diagram()->getExpression( *attIt, context.expressionContext() );
    QStringList columns = expression->referencedColumns();
    QStringList::const_iterator columnsIterator = columns.constBegin();
    for ( ; columnsIterator != columns.constEnd(); ++columnsIterator )
    {
      if ( !attributeNames.contains( *columnsIterator ) )
        attributeNames << *columnsIterator;
    }
  }

  const QgsLinearlyInterpolatedDiagramRenderer* linearlyInterpolatedDiagramRenderer = dynamic_cast<const QgsLinearlyInterpolatedDiagramRenderer*>( diagRenderer );
  if ( linearlyInterpolatedDiagramRenderer != NULL )
  {
    if ( linearlyInterpolatedDiagramRenderer->classificationAttributeIsExpression() )
    {
      QgsExpression* expression = diagRenderer->diagram()->getExpression( linearlyInterpolatedDiagramRenderer->classificationAttributeExpression(), context.expressionContext() );
      QStringList columns = expression->referencedColumns();
      QStringList::const_iterator columnsIterator = columns.constBegin();
      for ( ; columnsIterator != columns.constEnd(); ++columnsIterator )
      {
        if ( !attributeNames.contains( *columnsIterator ) )
          attributeNames << *columnsIterator;
      }
    }
    else
    {
      QString name = mFields.at( linearlyInterpolatedDiagramRenderer->classificationAttribute() ).name();
      if ( !attributeNames.contains( name ) )
        attributeNames << name;
    }
  }

  //and the ones needed for data defined diagram positions
  if ( mSettings.xPosColumn != -1 )
    attributeNames << mFields.at( mSettings.xPosColumn ).name();
  if ( mSettings.yPosColumn != -1 )
    attributeNames << mFields.at( mSettings.yPosColumn ).name();

  return true;
}


void QgsVectorLayerDiagramProvider::registerFeature( QgsFeature& feature, QgsRenderContext& context )
{
  QgsLabelFeature* label = registerDiagram( feature, context );
  if ( label )
    mFeatures << label;
}


QgsLabelFeature* QgsVectorLayerDiagramProvider::registerDiagram( QgsFeature& feat, QgsRenderContext &context )
{
  const QgsMapSettings& mapSettings = mEngine->mapSettings();

  QgsDiagramRendererV2* dr = mSettings.renderer;
  if ( dr )
  {
    QList<QgsDiagramSettings> settingList = dr->diagramSettings();
    if ( settingList.size() > 0 && settingList.at( 0 ).scaleBasedVisibility )
    {
      double minScale = settingList.at( 0 ).minScaleDenominator;
      if ( minScale > 0 && context.rendererScale() < minScale )
      {
        return 0;
      }

      double maxScale = settingList.at( 0 ).maxScaleDenominator;
      if ( maxScale > 0 && context.rendererScale() > maxScale )
      {
        return 0;
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

  const GEOSGeometry* geos_geom = 0;
  QScopedPointer<QgsGeometry> preparedGeom;
  if ( QgsPalLabeling::geometryRequiresPreparation( geom, context, mSettings.ct, extentGeom.data() ) )
  {
    preparedGeom.reset( QgsPalLabeling::prepareGeometry( geom, context, mSettings.ct, extentGeom.data() ) );
    if ( !preparedGeom.data() )
      return 0;
    geos_geom = preparedGeom.data()->asGeos();
  }
  else
  {
    geos_geom = geom->asGeos();
  }

  if ( geos_geom == 0 )
  {
    return 0; // invalid geometry
  }

  GEOSGeometry* geomCopy = GEOSGeom_clone_r( QgsGeometry::getGEOSHandler(), geos_geom );

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
  bool alwaysShow = mSettings.showAll;
  int ddColX = mSettings.xPosColumn;
  int ddColY = mSettings.yPosColumn;
  double ddPosX = 0.0;
  double ddPosY = 0.0;
  bool ddPos = ( ddColX >= 0 && ddColY >= 0 );
  if ( ddPos )
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
      const QgsCoordinateTransform* ct = mSettings.ct;
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

  QgsDiagramLabelFeature* lf = new QgsDiagramLabelFeature( feat.id(), geomCopy, QSizeF( diagramWidth, diagramHeight ) );
  lf->setHasFixedPosition( ddPos );
  lf->setFixedPosition( QgsPoint( ddPosX, ddPosY ) );
  lf->setHasFixedAngle( true );
  lf->setFixedAngle( 0 );
  lf->setAlwaysShow( alwaysShow );
  lf->setIsObstacle( mSettings.obstacle );

  if ( dr )
  {
    //append the diagram attributes to lbl
    lf->setAttributes( feat.attributes() );
  }

  QgsPoint ptZero = mSettings.xform->toMapCoordinates( 0, 0 );
  QgsPoint ptOne = mSettings.xform->toMapCoordinates( 1, 0 );
  lf->setDistLabel( qAbs( ptOne.x() - ptZero.x() ) * mSettings.dist );
  return lf;
}
