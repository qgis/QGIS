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
#include "qgspalgeometry.h"
#include "qgsvectorlayer.h"
#include "diagram/qgsdiagram.h"

#include "feature.h"
#include "labelposition.h"


QgsVectorLayerDiagramProvider::QgsVectorLayerDiagramProvider( QgsVectorLayer* layer )
    : mLayer( layer )
{
  const QgsDiagramLayerSettings* diagSettings = layer->diagramLayerSettings();

  // initialize the local copy
  mSettings = *diagSettings;

  mPriority = 1 - diagSettings->priority / 10.0; // convert 0..10 --> 1..0
  mPlacement = QgsPalLayerSettings::Placement( diagSettings->placement );
  mLinePlacementFlags = diagSettings->placementFlags;
  if ( diagSettings->obstacle ) mFlags |= GeometriesAreObstacles;
}

QgsVectorLayerDiagramProvider::~QgsVectorLayerDiagramProvider()
{
  qDeleteAll( mSettings.geometries );
}

QString QgsVectorLayerDiagramProvider::id() const
{
  return mLayer->id() + "d";
}

QList<QgsLabelFeature*> QgsVectorLayerDiagramProvider::labelFeatures( const QgsMapSettings& mapSettings, const QgsRenderContext& context )
{

  QgsDiagramLayerSettings& s2 = mSettings;

  s2.ct = 0;
  if ( mapSettings.hasCrsTransformEnabled() )
    s2.ct = new QgsCoordinateTransform( mLayer->crs(), mapSettings.destinationCrs() );

  s2.xform = &mapSettings.mapToPixel();

  s2.fields = mLayer->fields();

  s2.renderer = mLayer->diagramRenderer()->clone();

  const QgsDiagramRendererV2* diagRenderer = mLayer->diagramRenderer();
  QgsFields fields = mLayer->fields();

  QStringList attributeNames;

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
      QString name = fields.at( linearlyInterpolatedDiagramRenderer->classificationAttribute() ).name();
      if ( !attributeNames.contains( name ) )
        attributeNames << name;
    }
  }

  //and the ones needed for data defined diagram positions
  if ( mSettings.xPosColumn != -1 )
    attributeNames << fields.at( mSettings.xPosColumn ).name();
  if ( mSettings.yPosColumn != -1 )
    attributeNames << fields.at( mSettings.yPosColumn ).name();


  QgsFeatureRequest request;
  request.setFilterRect( context.extent() );
  request.setSubsetOfAttributes( attributeNames, mLayer->fields() );
  QgsFeatureIterator fit = mLayer->getFeatures( request );

  QList<QgsLabelFeature*> features;

  QgsFeature fet;
  while ( fit.nextFeature( fet ) )
  {
    QgsLabelFeature* label = registerDiagram( fet, mapSettings, context );
    if ( label )
      features << label;
  }

  return features;
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

  QgsPalGeometry* palGeometry = dynamic_cast<QgsPalGeometry*>( label->getFeaturePart()->getUserGeometry() );

  QgsFeature feature;
  feature.setFields( mSettings.fields );
  palGeometry->feature( feature );

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
  //for diagrams, remove the additional 'd' at the end of the layer id
  QString layerId = id();
  layerId.chop( 1 );
  mEngine->results()->mLabelSearchTree->insertLabel( label, QString( palGeometry->strId() ).toInt(), layerId, QString(), QFont(), true, false );

}


QgsLabelFeature* QgsVectorLayerDiagramProvider::registerDiagram( QgsFeature& feat, const QgsMapSettings& mapSettings, const QgsRenderContext& context )
{

  QgsDiagramRendererV2* dr = mSettings.renderer;
  if ( dr )
  {
    QList<QgsDiagramSettings> settingList = dr->diagramSettings();
    if ( settingList.size() > 0 )
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

  //create PALGeometry with diagram = true
  QgsPalGeometry* lbl = new QgsPalGeometry( feat.id(), QString(), GEOSGeom_clone_r( QgsGeometry::getGEOSHandler(), geos_geom ) );
  lbl->setIsDiagram( true );

  // record the created geometry - it will be deleted at the end.
  mSettings.geometries.append( lbl );

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

    //append the diagram attributes to lbl
    lbl->setDiagramAttributes( feat.attributes() );
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

  QgsLabelFeature* lf = new QgsLabelFeature( lbl->strId(), lbl, QSizeF( diagramWidth, diagramHeight ) );
  lf->setHasFixedPosition( ddPos );
  lf->setFixedPosition( QgsPoint( ddPosX, ddPosY ) );
  lf->setHasFixedAngle( true );
  lf->setFixedAngle( 0 );
  lf->setAlwaysShow( alwaysShow );

  QgsPoint ptZero = mSettings.xform->toMapCoordinates( 0, 0 );
  QgsPoint ptOne = mSettings.xform->toMapCoordinates( 1, 0 );
  lf->setDistLabel( qAbs( ptOne.x() - ptZero.x() ) * mSettings.dist );
  return lf;
}
