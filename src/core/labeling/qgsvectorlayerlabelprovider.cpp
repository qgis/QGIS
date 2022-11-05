/***************************************************************************
  qgsvectorlayerlabelprovider.cpp
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

#include "qgsvectorlayerlabelprovider.h"

#include "qgsgeometry.h"
#include "qgslabelsearchtree.h"
#include "qgspallabeling.h"
#include "qgstextlabelfeature.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayerfeatureiterator.h"
#include "qgsrenderer.h"
#include "qgspolygon.h"
#include "qgslinestring.h"
#include "qgsmultipolygon.h"
#include "qgslogger.h"
#include "qgsexpressioncontextutils.h"
#include "qgsmaskidprovider.h"
#include "qgstextcharacterformat.h"
#include "qgstextfragment.h"
#include "qgslabelingresults.h"
#include "qgstextrenderer.h"

#include "feature.h"
#include "labelposition.h"
#include "callouts/qgscallout.h"
#include "qgssymbol.h"
#include "qgsmarkersymbol.h"

#include "pal/layer.h"

#include <QPicture>
#include <QTextDocument>
#include <QTextFragment>

using namespace pal;

QgsVectorLayerLabelProvider::QgsVectorLayerLabelProvider( QgsVectorLayer *layer, const QString &providerId, bool withFeatureLoop, const QgsPalLayerSettings *settings, const QString &layerName )
  : QgsAbstractLabelProvider( layer, providerId )
  , mSettings( settings ? * settings : QgsPalLayerSettings() ) // TODO: all providers should have valid settings?
  , mLayerGeometryType( layer->geometryType() )
  , mRenderer( layer->renderer() )
  , mFields( layer->fields() )
  , mCrs( layer->crs() )
{
  mName = layerName.isEmpty() ? layer->id() : layerName;

  if ( withFeatureLoop )
  {
    mSource = std::make_unique<QgsVectorLayerFeatureSource>( layer );
  }

  init();
}

QgsVectorLayerLabelProvider::QgsVectorLayerLabelProvider( QgsWkbTypes::GeometryType geometryType, const QgsFields &fields, const QgsCoordinateReferenceSystem &crs, const QString &providerId, const QgsPalLayerSettings *settings, QgsMapLayer *layer, const QString &layerName )
  : QgsAbstractLabelProvider( layer, providerId )
  , mSettings( settings ? * settings : QgsPalLayerSettings() ) // TODO: all providers should have valid settings?
  , mLayerGeometryType( geometryType )
  , mRenderer( nullptr )
  , mFields( fields )
  , mCrs( crs )
{
  mName = layerName.isEmpty() ? layer->id() : layerName;

  init();
}

void QgsVectorLayerLabelProvider::init()
{
  mPlacement = mSettings.placement;

  mFlags = Flags();
  if ( mSettings.drawLabels )
    mFlags |= DrawLabels;
  if ( mSettings.lineSettings().mergeLines() && !mSettings.lineSettings().addDirectionSymbol() )
    mFlags |= MergeConnectedLines;
  if ( mSettings.centroidInside )
    mFlags |= CentroidMustBeInside;

  mPriority = 1 - mSettings.priority / 10.0; // convert 0..10 --> 1..0

  if ( mLayerGeometryType == QgsWkbTypes::PointGeometry && mRenderer )
  {
    //override obstacle type to treat any intersection of a label with the point symbol as a high cost conflict
    mObstacleType = QgsLabelObstacleSettings::PolygonWhole;
  }
  else
  {
    mObstacleType = mSettings.obstacleSettings().type();
  }

  mUpsidedownLabels = mSettings.upsidedownLabels;
}


QgsVectorLayerLabelProvider::~QgsVectorLayerLabelProvider()
{
  qDeleteAll( mLabels );
}


bool QgsVectorLayerLabelProvider::prepare( QgsRenderContext &context, QSet<QString> &attributeNames )
{
  const QgsMapSettings &mapSettings = mEngine->mapSettings();

  return mSettings.prepare( context, attributeNames, mFields, mapSettings, mCrs );
}

void QgsVectorLayerLabelProvider::startRender( QgsRenderContext &context )
{
  QgsAbstractLabelProvider::startRender( context );
  mSettings.startRender( context );
}

void QgsVectorLayerLabelProvider::stopRender( QgsRenderContext &context )
{
  QgsAbstractLabelProvider::stopRender( context );
  mSettings.stopRender( context );
}

QList<QgsLabelFeature *> QgsVectorLayerLabelProvider::labelFeatures( QgsRenderContext &ctx )
{
  if ( !mSource )
  {
    // we have created the provider with "own feature loop" == false
    // so it is assumed that prepare() has been already called followed by registerFeature() calls
    return mLabels;
  }

  QSet<QString> attrNames;
  if ( !prepare( ctx, attrNames ) )
    return QList<QgsLabelFeature *>();

  if ( mRenderer )
    mRenderer->startRender( ctx, mFields );

  QgsRectangle layerExtent = ctx.extent();
  if ( mSettings.ct.isValid() && !mSettings.ct.isShortCircuited() )
  {
    QgsCoordinateTransform extentTransform = mSettings.ct;
    extentTransform.setBallparkTransformsAreAppropriate( true );
    layerExtent = extentTransform.transformBoundingBox( ctx.extent(), Qgis::TransformDirection::Reverse );
  }

  QgsFeatureRequest request;
  request.setFilterRect( layerExtent );
  request.setSubsetOfAttributes( attrNames, mFields );
  QgsFeatureIterator fit = mSource->getFeatures( request );

  QgsExpressionContextScope *symbolScope = new QgsExpressionContextScope();
  ctx.expressionContext().appendScope( symbolScope );
  QgsFeature fet;
  while ( fit.nextFeature( fet ) )
  {
    QgsGeometry obstacleGeometry;
    const QgsSymbol *symbol = nullptr;
    if ( mRenderer )
    {
      QgsSymbolList symbols = mRenderer->originalSymbolsForFeature( fet, ctx );
      if ( !symbols.isEmpty() && fet.geometry().type() == QgsWkbTypes::PointGeometry )
      {
        //point feature, use symbol bounds as obstacle
        obstacleGeometry = QgsVectorLayerLabelProvider::getPointObstacleGeometry( fet, ctx, symbols );
      }
      if ( !symbols.isEmpty() )
      {
        symbol = symbols.at( 0 );
        symbolScope = QgsExpressionContextUtils::updateSymbolScope( symbol, symbolScope );
      }
    }
    ctx.expressionContext().setFeature( fet );
    registerFeature( fet, ctx, obstacleGeometry, symbol );
  }

  if ( ctx.expressionContext().lastScope() == symbolScope )
    delete ctx.expressionContext().popScope();

  if ( mRenderer )
    mRenderer->stopRender( ctx );

  return mLabels;
}

QList< QgsLabelFeature * > QgsVectorLayerLabelProvider::registerFeature( const QgsFeature &feature, QgsRenderContext &context, const QgsGeometry &obstacleGeometry, const QgsSymbol *symbol )
{
  std::unique_ptr< QgsLabelFeature > label = mSettings.registerFeatureWithDetails( feature, context, obstacleGeometry, symbol );
  QList< QgsLabelFeature * > res;
  if ( label )
  {
    res << label.get();
    mLabels << label.release();
  }
  return res;
}

QgsGeometry QgsVectorLayerLabelProvider::getPointObstacleGeometry( QgsFeature &fet, QgsRenderContext &context, const QgsSymbolList &symbols )
{
  if ( !fet.hasGeometry() || fet.geometry().type() != QgsWkbTypes::PointGeometry )
    return QgsGeometry();

  bool isMultiPoint = fet.geometry().constGet()->nCoordinates() > 1;
  std::unique_ptr< QgsAbstractGeometry > obstacleGeom;
  if ( isMultiPoint )
    obstacleGeom = std::make_unique< QgsMultiPolygon >();

  // for each point
  for ( int i = 0; i < fet.geometry().constGet()->nCoordinates(); ++i )
  {
    QRectF bounds;
    QgsPoint p = fet.geometry().constGet()->vertexAt( QgsVertexId( i, 0, 0 ) );
    double x = p.x();
    double y = p.y();
    double z = 0; // dummy variable for coordinate transforms

    //transform point to pixels
    if ( context.coordinateTransform().isValid() )
    {
      try
      {
        context.coordinateTransform().transformInPlace( x, y, z );
      }
      catch ( QgsCsException & )
      {
        return QgsGeometry();
      }
    }
    context.mapToPixel().transformInPlace( x, y );

    QPointF pt( x, y );
    const auto constSymbols = symbols;
    for ( QgsSymbol *symbol : constSymbols )
    {
      if ( symbol->type() == Qgis::SymbolType::Marker )
      {
        if ( bounds.isValid() )
          bounds = bounds.united( static_cast< QgsMarkerSymbol * >( symbol )->bounds( pt, context, fet ) );
        else
          bounds = static_cast< QgsMarkerSymbol * >( symbol )->bounds( pt, context, fet );
      }
    }

    //convert bounds to a geometry
    QVector< double > bX;
    bX << bounds.left() << bounds.right() << bounds.right() << bounds.left();
    QVector< double > bY;
    bY << bounds.top() << bounds.top() << bounds.bottom() << bounds.bottom();
    std::unique_ptr< QgsLineString > boundLineString = std::make_unique< QgsLineString >( bX, bY );

    //then transform back to map units
    //TODO - remove when labeling is refactored to use screen units
    for ( int i = 0; i < boundLineString->numPoints(); ++i )
    {
      QgsPointXY point = context.mapToPixel().toMapCoordinates( static_cast<int>( boundLineString->xAt( i ) ),
                         static_cast<int>( boundLineString->yAt( i ) ) );
      boundLineString->setXAt( i, point.x() );
      boundLineString->setYAt( i, point.y() );
    }
    if ( context.coordinateTransform().isValid() )
    {
      try
      {
        boundLineString->transform( context.coordinateTransform(), Qgis::TransformDirection::Reverse );
      }
      catch ( QgsCsException & )
      {
        return QgsGeometry();
      }
    }
    boundLineString->close();

    if ( context.coordinateTransform().isValid() )
    {
      // coordinate transforms may have resulted in nan coordinates - if so, strip these out
      boundLineString->filterVertices( []( const QgsPoint & point )->bool
      {
        return std::isfinite( point.x() ) && std::isfinite( point.y() );
      } );
      if ( !boundLineString->isRing() )
        return QgsGeometry();
    }

    std::unique_ptr< QgsPolygon > obstaclePolygon = std::make_unique< QgsPolygon >();
    obstaclePolygon->setExteriorRing( boundLineString.release() );

    if ( isMultiPoint )
    {
      static_cast<QgsMultiPolygon *>( obstacleGeom.get() )->addGeometry( obstaclePolygon.release() );
    }
    else
    {
      obstacleGeom = std::move( obstaclePolygon );
    }
  }

  return QgsGeometry( std::move( obstacleGeom ) );
}

void QgsVectorLayerLabelProvider::drawLabelBackground( QgsRenderContext &context, LabelPosition *label ) const
{
  if ( !mSettings.drawLabels )
    return;

  // render callout
  if ( mSettings.callout() && mSettings.callout()->drawOrder() == QgsCallout::OrderBelowAllLabels )
  {
    drawCallout( context, label );
  }
}

void QgsVectorLayerLabelProvider::drawCallout( QgsRenderContext &context, pal::LabelPosition *label ) const
{
  bool enabled = mSettings.callout()->enabled();
  if ( mSettings.dataDefinedProperties().isActive( QgsPalLayerSettings::CalloutDraw ) )
  {
    context.expressionContext().setOriginalValueVariable( enabled );
    enabled = mSettings.dataDefinedProperties().valueAsBool( QgsPalLayerSettings::CalloutDraw, context.expressionContext(), enabled );
  }
  if ( enabled )
  {
    QgsMapToPixel xform = context.mapToPixel();
    xform.setMapRotation( 0, 0, 0 );
    QPointF outPt = xform.transform( label->getX(), label->getY() ).toQPointF();
    QgsPointXY outPt2 = xform.transform( label->getX() + label->getWidth(), label->getY() + label->getHeight() );
    QRectF rect( outPt.x(), outPt.y(), outPt2.x() - outPt.x(), outPt2.y() - outPt.y() );

    QgsGeometry g( QgsGeos::fromGeos( label->getFeaturePart()->feature()->geometry() ) );
    g.transform( xform.transform() );
    QgsCallout::QgsCalloutContext calloutContext;
    calloutContext.allFeaturePartsLabeled = label->getFeaturePart()->feature()->labelAllParts();
    calloutContext.originalFeatureCrs = label->getFeaturePart()->feature()->originalFeatureCrs();
    mSettings.callout()->render( context, rect, label->getAlpha() * 180 / M_PI, g, calloutContext );

    const QList< QgsCalloutPosition > renderedPositions = calloutContext.positions();

    for ( QgsCalloutPosition position : renderedPositions )
    {
      position.layerID = mLayerId;
      position.featureId = label->getFeaturePart()->featureId();
      position.providerID = mProviderId;
      mEngine->results()->mLabelSearchTree->insertCallout( position );
    }
  }
}

void QgsVectorLayerLabelProvider::drawLabel( QgsRenderContext &context, pal::LabelPosition *label ) const
{
  if ( !mSettings.drawLabels )
    return;

  QgsTextLabelFeature *lf = dynamic_cast<QgsTextLabelFeature *>( label->getFeaturePart()->feature() );

  // Copy to temp, editable layer settings
  // these settings will be changed by any data defined values, then used for rendering label components
  // settings may be adjusted during rendering of components
  QgsPalLayerSettings tmpLyr( mSettings );

  // apply any previously applied data defined settings for the label
  const QMap< QgsPalLayerSettings::Property, QVariant > &ddValues = lf->dataDefinedValues();

  //font
  QFont dFont = lf->definedFont();
  QgsDebugMsgLevel( QStringLiteral( "PAL font tmpLyr: %1, Style: %2" ).arg( tmpLyr.format().font().toString(), tmpLyr.format().font().styleName() ), 4 );
  QgsDebugMsgLevel( QStringLiteral( "PAL font definedFont: %1, Style: %2" ).arg( dFont.toString(), dFont.styleName() ), 4 );

  QgsTextFormat format = tmpLyr.format();
  format.setFont( dFont );

  // size has already been calculated and stored in the defined font - this calculated size
  // is in pixels
  format.setSize( dFont.pixelSize() );
  format.setSizeUnit( QgsUnitTypes::RenderPixels );
  tmpLyr.setFormat( format );

  if ( tmpLyr.multilineAlign == Qgis::LabelMultiLineAlignment::FollowPlacement )
  {
    //calculate font alignment based on label quadrant
    switch ( label->getQuadrant() )
    {
      case LabelPosition::QuadrantAboveLeft:
      case LabelPosition::QuadrantLeft:
      case LabelPosition::QuadrantBelowLeft:
        tmpLyr.multilineAlign = Qgis::LabelMultiLineAlignment::Right;
        break;
      case LabelPosition::QuadrantAbove:
      case LabelPosition::QuadrantOver:
      case LabelPosition::QuadrantBelow:
        tmpLyr.multilineAlign = Qgis::LabelMultiLineAlignment::Center;
        break;
      case LabelPosition::QuadrantAboveRight:
      case LabelPosition::QuadrantRight:
      case LabelPosition::QuadrantBelowRight:
        tmpLyr.multilineAlign = Qgis::LabelMultiLineAlignment::Left;
        break;
    }
  }

  // update tmpLyr with any data defined text style values
  QgsPalLabeling::dataDefinedTextStyle( tmpLyr, ddValues );

  // update tmpLyr with any data defined text buffer values
  QgsPalLabeling::dataDefinedTextBuffer( tmpLyr, ddValues );

  // update tmpLyr with any data defined text mask values
  QgsPalLabeling::dataDefinedTextMask( tmpLyr, ddValues );

  // update tmpLyr with any data defined text formatting values
  QgsPalLabeling::dataDefinedTextFormatting( tmpLyr, ddValues );

  // update tmpLyr with any data defined shape background values
  QgsPalLabeling::dataDefinedShapeBackground( tmpLyr, ddValues );

  // update tmpLyr with any data defined drop shadow values
  QgsPalLabeling::dataDefinedDropShadow( tmpLyr, ddValues );

  // Render the components of a label in reverse order
  //   (backgrounds -> text)

  // render callout
  if ( mSettings.callout() && mSettings.callout()->drawOrder() == QgsCallout::OrderBelowIndividualLabels )
  {
    drawCallout( context, label );
  }

  if ( tmpLyr.format().shadow().enabled() && tmpLyr.format().shadow().shadowPlacement() == QgsTextShadowSettings::ShadowLowest )
  {
    QgsTextFormat format = tmpLyr.format();

    if ( tmpLyr.format().background().enabled() && tmpLyr.format().background().type() != QgsTextBackgroundSettings::ShapeMarkerSymbol ) // background shadows not compatible with marker symbol backgrounds
    {
      format.shadow().setShadowPlacement( QgsTextShadowSettings::ShadowShape );
    }
    else if ( tmpLyr.format().buffer().enabled() )
    {
      format.shadow().setShadowPlacement( QgsTextShadowSettings::ShadowBuffer );
    }
    else
    {
      format.shadow().setShadowPlacement( QgsTextShadowSettings::ShadowText );
    }

    tmpLyr.setFormat( format );
  }

  if ( tmpLyr.format().background().enabled() )
  {
    drawLabelPrivate( label, context, tmpLyr, Qgis::TextComponent::Background );
  }

  if ( tmpLyr.format().buffer().enabled() )
  {
    drawLabelPrivate( label, context, tmpLyr, Qgis::TextComponent::Buffer );
  }

  drawLabelPrivate( label, context, tmpLyr, Qgis::TextComponent::Text );

  // add to the results
  QString labeltext = label->getFeaturePart()->feature()->labelText();
  mEngine->results()->mLabelSearchTree->insertLabel( label, label->getFeaturePart()->featureId(), mLayerId, labeltext, dFont, false, lf->hasFixedPosition(), mProviderId );
}

void QgsVectorLayerLabelProvider::drawUnplacedLabel( QgsRenderContext &context, LabelPosition *label ) const
{
  QgsTextLabelFeature *lf = dynamic_cast<QgsTextLabelFeature *>( label->getFeaturePart()->feature() );

  QgsTextFormat format = mSettings.format();
  if ( mSettings.drawLabels
       && mSettings.unplacedVisibility() != Qgis::UnplacedLabelVisibility::NeverShow
       && mEngine->engineSettings().flags() & Qgis::LabelingFlag::DrawUnplacedLabels )
  {
    QgsPalLayerSettings tmpLyr( mSettings );
    format = tmpLyr.format();
    format.setColor( mEngine->engineSettings().unplacedLabelColor() );
    tmpLyr.setFormat( format );
    drawLabelPrivate( label, context, tmpLyr, Qgis::TextComponent::Text );
  }

  // add to the results
  QString labeltext = label->getFeaturePart()->feature()->labelText();
  mEngine->results()->mLabelSearchTree->insertLabel( label, label->getFeaturePart()->featureId(), mLayerId, labeltext, format.font(), false, lf->hasFixedPosition(), mProviderId, true );
}

void QgsVectorLayerLabelProvider::drawLabelPrivate( pal::LabelPosition *label, QgsRenderContext &context, QgsPalLayerSettings &tmpLyr, Qgis::TextComponent drawType, double dpiRatio ) const
{
  // NOTE: this is repeatedly called for multi-part labels
  QPainter *painter = context.painter();

  // features are pre-rotated but not scaled/translated,
  // so we only disable rotation here. Ideally, they'd be
  // also pre-scaled/translated, as suggested here:
  // https://github.com/qgis/QGIS/issues/20071
  QgsMapToPixel xform = context.mapToPixel();
  xform.setMapRotation( 0, 0, 0 );

  QPointF outPt = xform.transform( label->getX(), label->getY() ).toQPointF();

  if ( mEngine->engineSettings().testFlag( Qgis::LabelingFlag::DrawLabelRectOnly ) )  // TODO: this should get directly to labeling engine
  {
    //debugging rect
    if ( drawType != Qgis::TextComponent::Text )
      return;

    QgsPointXY outPt2 = xform.transform( label->getX() + label->getWidth(), label->getY() + label->getHeight() );
    QRectF rect( 0, 0, outPt2.x() - outPt.x(), outPt2.y() - outPt.y() );
    painter->save();
    painter->setRenderHint( QPainter::Antialiasing, false );
    painter->translate( QPointF( outPt.x(), outPt.y() ) );
    painter->rotate( -label->getAlpha() * 180 / M_PI );

    if ( label->conflictsWithObstacle() )
    {
      painter->setBrush( QColor( 255, 0, 0, 100 ) );
      painter->setPen( QColor( 255, 0, 0, 150 ) );
    }
    else
    {
      painter->setBrush( QColor( 0, 255, 0, 100 ) );
      painter->setPen( QColor( 0, 255, 0, 150 ) );
    }

    painter->drawRect( rect );
    painter->restore();

    if ( label->nextPart() )
      drawLabelPrivate( label->nextPart(), context, tmpLyr, drawType, dpiRatio );

    return;
  }
  if ( mEngine->engineSettings().testFlag( Qgis::LabelingFlag::DrawLabelMetrics ) )
  {
    if ( drawType != Qgis::TextComponent::Text )
      return;

    QgsPointXY outPt2 = xform.transform( label->getX() + label->getWidth(), label->getY() + label->getHeight() );
    QRectF rect( 0, 0, outPt2.x() - outPt.x(), outPt2.y() - outPt.y() );
    painter->save();
    painter->setRenderHint( QPainter::Antialiasing, false );
    painter->translate( QPointF( outPt.x(), outPt.y() ) );
    painter->rotate( -label->getAlpha() * 180 / M_PI );

    painter->setBrush( Qt::NoBrush );
    painter->setPen( QColor( 255, 0, 0, 220 ) );

    painter->drawRect( rect );


    painter->setPen( QColor( 0, 0, 0, 60 ) );
    const QgsMargins &margins = label->getFeaturePart()->feature()->visualMargin();
    if ( margins.top() > 0 )
    {
      const double topMargin = margins.top() / context.mapToPixel().mapUnitsPerPixel();
      painter->drawLine( QPointF( rect.left(), rect.top() - topMargin ), QPointF( rect.right(), rect.top() - topMargin ) );
    }
    if ( margins.bottom() > 0 )
    {
      const double bottomMargin = margins.top() / context.mapToPixel().mapUnitsPerPixel();
      painter->drawLine( QPointF( rect.left(), rect.bottom() + bottomMargin ), QPointF( rect.right(), rect.bottom() + bottomMargin ) );
    }

    const QRectF outerBounds = label->getFeaturePart()->feature()->outerBounds();
    if ( !outerBounds.isNull() )
    {
      const QRectF mapOuterBounds = QRectF( label->getX() + outerBounds.left(),
                                            label->getY() + outerBounds.top(),
                                            outerBounds.width(), outerBounds.height() );

      QgsPointXY outerBoundsPt1 = xform.transform( mapOuterBounds.left(), mapOuterBounds.top() );
      QgsPointXY outerBoundsPt2 = xform.transform( mapOuterBounds.right(), mapOuterBounds.bottom() );

      const QRectF outerBoundsPixel( outerBoundsPt1.x() - outPt.x(),
                                     outerBoundsPt1.y() - outPt.y(),
                                     outerBoundsPt2.x() - outerBoundsPt1.x(),
                                     outerBoundsPt2.y() - outerBoundsPt1.y() );

      QPen pen( QColor( 255, 0, 255, 140 ) );
      pen.setCosmetic( true );
      pen.setWidth( 1 );
      painter->setPen( pen );
      painter->drawRect( outerBoundsPixel );
    }

    if ( QgsTextLabelFeature *textFeature = dynamic_cast< QgsTextLabelFeature * >( label->getFeaturePart()->feature() ) )
    {
      const QgsTextDocumentMetrics &metrics = textFeature->documentMetrics();
      const QgsTextDocument &document = textFeature->document();
      const int blockCount = document.size();

      double prevBlockBaseline = rect.bottom() - rect.top();

      // draw block baselines
      for ( int blockIndex = 0; blockIndex < blockCount; ++blockIndex )
      {
        const double blockBaseLine = metrics.baselineOffset( blockIndex, Qgis::TextLayoutMode::Labeling );

        const QgsTextBlock &block = document.at( blockIndex );
        const int fragmentCount = block.size();
        double left = 0;
        for ( int fragmentIndex = 0; fragmentIndex < fragmentCount; ++fragmentIndex )
        {
          const double fragmentVerticalOffset = metrics.fragmentVerticalOffset( blockIndex, fragmentIndex, Qgis::TextLayoutMode::Labeling );
          const double right = left + metrics.fragmentHorizontalAdvance( blockIndex, fragmentIndex, Qgis::TextLayoutMode::Labeling );

          if ( fragmentIndex > 0 )
          {
            QPen pen( QColor( 0, 0, 255, 220 ) );
            pen.setStyle( Qt::PenStyle::DashLine );

            painter->setPen( pen );

            painter->drawLine( QPointF( rect.left() + left, rect.top() + blockBaseLine + fragmentVerticalOffset ),
                               QPointF( rect.left() + left, rect.top() + prevBlockBaseline ) );

          }

          painter->setPen( QColor( 0, 0, 255, 220 ) );
          painter->drawLine( QPointF( rect.left() + left, rect.top()  + blockBaseLine + fragmentVerticalOffset ),
                             QPointF( rect.left() + right, rect.top() + blockBaseLine + fragmentVerticalOffset ) );
          left = right;
        }
        prevBlockBaseline = blockBaseLine;
      }
    }

    painter->restore();
  }

  QgsTextRenderer::Component component;
  component.dpiRatio = dpiRatio;
  component.origin = outPt;
  component.rotation = label->getAlpha();

  if ( drawType == Qgis::TextComponent::Background )
  {
    // get rotated label's center point
    QPointF centerPt( outPt );
    QgsPointXY outPt2 = xform.transform( label->getX() + label->getWidth() / 2,
                                         label->getY() + label->getHeight() / 2 );

    double xc = outPt2.x() - outPt.x();
    double yc = outPt2.y() - outPt.y();

    double angle = -component.rotation;
    double xd = xc * std::cos( angle ) - yc * std::sin( angle );
    double yd = xc * std::sin( angle ) + yc * std::cos( angle );

    centerPt.setX( centerPt.x() + xd );
    centerPt.setY( centerPt.y() + yd );

    component.center = centerPt;

    {
      // label size has already been calculated using any symbology reference scale factor -- we need
      // to temporarily remove the reference scale here or we'll be applying the scaling twice
      QgsScopedRenderContextReferenceScaleOverride referenceScaleOverride( context, -1.0 );

      // convert label size to render units
      double labelWidthPx = context.convertToPainterUnits( label->getWidth(), QgsUnitTypes::RenderMapUnits, QgsMapUnitScale() );
      double labelHeightPx = context.convertToPainterUnits( label->getHeight(), QgsUnitTypes::RenderMapUnits, QgsMapUnitScale() );

      component.size = QSizeF( labelWidthPx, labelHeightPx );
    }

    QgsTextRenderer::drawBackground( context, component, tmpLyr.format(), QgsTextDocumentMetrics(), Qgis::TextLayoutMode::Labeling );
  }

  else if ( drawType == Qgis::TextComponent::Buffer
            || drawType == Qgis::TextComponent::Text )
  {

    // TODO: optimize access :)
    QgsTextLabelFeature *lf = static_cast<QgsTextLabelFeature *>( label->getFeaturePart()->feature() );
    QString txt = lf->text( label->getPartId() );

    if ( auto *lMaskIdProvider = context.maskIdProvider() )
    {
      int maskId = lMaskIdProvider->maskId( label->getFeaturePart()->layer()->provider()->layerId(),
                                            label->getFeaturePart()->layer()->provider()->providerId() );
      context.setCurrentMaskId( maskId );
    }

    //add the direction symbol if needed
    if ( !txt.isEmpty() && tmpLyr.placement == Qgis::LabelPlacement::Line &&
         tmpLyr.lineSettings().addDirectionSymbol() )
    {
      bool prependSymb = false;
      QString symb = tmpLyr.lineSettings().rightDirectionSymbol();

      if ( label->getReversed() )
      {
        prependSymb = true;
        symb = tmpLyr.lineSettings().leftDirectionSymbol();
      }

      if ( tmpLyr.lineSettings().reverseDirectionSymbol() )
      {
        if ( symb == tmpLyr.lineSettings().rightDirectionSymbol() )
        {
          prependSymb = true;
          symb = tmpLyr.lineSettings().leftDirectionSymbol();
        }
        else
        {
          prependSymb = false;
          symb = tmpLyr.lineSettings().rightDirectionSymbol();
        }
      }

      switch ( tmpLyr.lineSettings().directionSymbolPlacement() )
      {
        case QgsLabelLineSettings::DirectionSymbolPlacement::SymbolAbove:
          prependSymb = true;
          symb = symb + QStringLiteral( "\n" );
          break;

        case QgsLabelLineSettings::DirectionSymbolPlacement::SymbolBelow:
          prependSymb = false;
          symb = QStringLiteral( "\n" ) + symb;
          break;

        case QgsLabelLineSettings::DirectionSymbolPlacement::SymbolLeftRight:
          break;
      }

      if ( prependSymb )
      {
        txt.prepend( symb );
      }
      else
      {
        txt.append( symb );
      }
    }

    Qgis::TextHorizontalAlignment hAlign = Qgis::TextHorizontalAlignment::Left;
    if ( tmpLyr.multilineAlign == Qgis::LabelMultiLineAlignment::Center )
      hAlign = Qgis::TextHorizontalAlignment::Center;
    else if ( tmpLyr.multilineAlign == Qgis::LabelMultiLineAlignment::Right )
      hAlign = Qgis::TextHorizontalAlignment::Right;
    else if ( tmpLyr.multilineAlign == Qgis::LabelMultiLineAlignment::Justify )
      hAlign = Qgis::TextHorizontalAlignment::Justify;

    QgsTextRenderer::Component component;
    component.origin = outPt;
    component.rotation = label->getAlpha();

    QgsTextDocument document;
    QgsTextDocumentMetrics metrics;
    if ( !tmpLyr.format().allowHtmlFormatting() || tmpLyr.placement == Qgis::LabelPlacement::Curved )
    {
      const QgsTextCharacterFormat c = lf->characterFormat( label->getPartId() );
      const QStringList multiLineList = QgsPalLabeling::splitToLines( txt, tmpLyr.wrapChar, tmpLyr.autoWrapLength, tmpLyr.useMaxLineLengthForAutoWrap );
      for ( const QString &line : multiLineList )
        document.append( QgsTextBlock( QgsTextFragment( line, c ) ) );

      QgsScopedRenderContextReferenceScaleOverride referenceScaleOverride( context, -1.0 );
      metrics = QgsTextDocumentMetrics::calculateMetrics( document, tmpLyr.format(), context );
    }
    else
    {
      document = lf->document();
      metrics = lf->documentMetrics();
    }

    QgsTextRenderer::drawTextInternal( drawType, context, tmpLyr.format(), component, document,
                                       metrics, hAlign, Qgis::TextVerticalAlignment::Top, Qgis::TextLayoutMode::Labeling );

  }
  if ( label->nextPart() )
    drawLabelPrivate( label->nextPart(), context, tmpLyr, drawType, dpiRatio );
}

const QgsPalLayerSettings &QgsVectorLayerLabelProvider::settings() const
{
  return mSettings;
}
