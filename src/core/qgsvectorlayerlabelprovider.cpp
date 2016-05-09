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

#include "qgsdatadefined.h"
#include "qgsgeometry.h"
#include "qgslabelsearchtree.h"
#include "qgspallabeling.h"
#include "qgstextlabelfeature.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayerfeatureiterator.h"
#include "qgsrendererv2.h"
#include "qgspolygonv2.h"
#include "qgslinestringv2.h"
#include "qgsmultipolygonv2.h"

#include "feature.h"
#include "labelposition.h"

#include <QPicture>

using namespace pal;

Q_GUI_EXPORT extern int qt_defaultDpiX();
Q_GUI_EXPORT extern int qt_defaultDpiY();

static void _fixQPictureDPI( QPainter* p )
{
  // QPicture makes an assumption that we drawing to it with system DPI.
  // Then when being drawn, it scales the painter. The following call
  // negates the effect. There is no way of setting QPicture's DPI.
  // See QTBUG-20361
  p->scale( static_cast< double >( qt_defaultDpiX() ) / p->device()->logicalDpiX(),
            static_cast< double >( qt_defaultDpiY() ) / p->device()->logicalDpiY() );
}



QgsVectorLayerLabelProvider::QgsVectorLayerLabelProvider( QgsVectorLayer* layer, const QString& providerId, bool withFeatureLoop, const QgsPalLayerSettings* settings, const QString& layerName )
    : QgsAbstractLabelProvider( layer->id(), providerId )
    , mSettings( settings ? *settings : QgsPalLayerSettings::fromLayer( layer ) )
    , mLayerGeometryType( layer->geometryType() )
    , mRenderer( layer->rendererV2() )
    , mFields( layer->fields() )
    , mCrs( layer->crs() )
{
  mName = layerName.isEmpty() ? layer->id() : layerName;

  if ( withFeatureLoop )
  {
    mSource = new QgsVectorLayerFeatureSource( layer );
    mOwnsSource = true;
  }
  else
  {
    mSource = nullptr;
    mOwnsSource = false;
  }

  init();
}

QgsVectorLayerLabelProvider::QgsVectorLayerLabelProvider( const QgsPalLayerSettings& settings,
    const QString& layerId,
    const QgsFields& fields,
    const QgsCoordinateReferenceSystem& crs,
    QgsAbstractFeatureSource* source,
    bool ownsSource, QgsFeatureRendererV2* renderer )
    : QgsAbstractLabelProvider( layerId )
    , mSettings( settings )
    , mLayerGeometryType( QGis::UnknownGeometry )
    , mRenderer( renderer )
    , mFields( fields )
    , mCrs( crs )
    , mSource( source )
    , mOwnsSource( ownsSource )
{
  init();
}


void QgsVectorLayerLabelProvider::init()
{
  mPlacement = mSettings.placement;
  mLinePlacementFlags = mSettings.placementFlags;
  mFlags = Flags();
  if ( mSettings.drawLabels ) mFlags |= DrawLabels;
  if ( mSettings.displayAll ) mFlags |= DrawAllLabels;
  if ( mSettings.mergeLines ) mFlags |= MergeConnectedLines;
  if ( mSettings.centroidInside ) mFlags |= CentroidMustBeInside;
  if ( mSettings.fitInPolygonOnly ) mFlags |= FitInPolygonOnly;
  if ( mSettings.labelPerPart ) mFlags |= LabelPerFeaturePart;
  mPriority = 1 - mSettings.priority / 10.0; // convert 0..10 --> 1..0

  if ( mLayerGeometryType == QGis::Point && mRenderer )
  {
    //override obstacle type to treat any intersection of a label with the point symbol as a high cost conflict
    mObstacleType = QgsPalLayerSettings::PolygonWhole;
  }
  else
  {
    mObstacleType = mSettings.obstacleType;
  }

  mUpsidedownLabels = mSettings.upsidedownLabels;
}


QgsVectorLayerLabelProvider::~QgsVectorLayerLabelProvider()
{
  qDeleteAll( mLabels );

  if ( mOwnsSource )
    delete mSource;
}


bool QgsVectorLayerLabelProvider::prepare( const QgsRenderContext& context, QStringList& attributeNames )
{
  QgsPalLayerSettings& lyr = mSettings;
  const QgsMapSettings& mapSettings = mEngine->mapSettings();

  QgsDebugMsgLevel( "PREPARE LAYER " + mLayerId, 4 );

  if ( lyr.drawLabels )
  {
    if ( lyr.fieldName.isEmpty() )
    {
      return false;
    }

    if ( lyr.isExpression )
    {
      QgsExpression exp( lyr.fieldName );
      if ( exp.hasEvalError() )
      {
        QgsDebugMsgLevel( "Prepare error:" + exp.evalErrorString(), 4 );
        return false;
      }
    }
    else
    {
      // If we aren't an expression, we check to see if we can find the column.
      if ( mFields.fieldNameIndex( lyr.fieldName ) == -1 )
      {
        return false;
      }
    }
  }

  lyr.mCurFields = mFields;

  if ( lyr.drawLabels )
  {
    // add field indices for label's text, from expression or field
    if ( lyr.isExpression )
    {
      // prepare expression for use in QgsPalLayerSettings::registerFeature()
      QgsExpression* exp = lyr.getLabelExpression();
      exp->prepare( &context.expressionContext() );
      if ( exp->hasEvalError() )
      {
        QgsDebugMsgLevel( "Prepare error:" + exp->evalErrorString(), 4 );
      }
      Q_FOREACH ( const QString& name, exp->referencedColumns() )
      {
        QgsDebugMsgLevel( "REFERENCED COLUMN = " + name, 4 );
        attributeNames.append( name );
      }
    }
    else
    {
      attributeNames.append( lyr.fieldName );
    }

    // add field indices of data defined expression or field
    QMap< QgsPalLayerSettings::DataDefinedProperties, QgsDataDefined* >::const_iterator dIt = lyr.dataDefinedProperties.constBegin();
    for ( ; dIt != lyr.dataDefinedProperties.constEnd(); ++dIt )
    {
      QgsDataDefined* dd = dIt.value();
      if ( !dd->isActive() )
      {
        continue;
      }

      // NOTE: the following also prepares any expressions for later use

      // store parameters for data defined expressions
      QMap<QString, QVariant> exprParams;
      exprParams.insert( "scale", context.rendererScale() );

      dd->setExpressionParams( exprParams );

      // this will return columns for expressions or field name, depending upon what is set to be used
      QStringList cols = dd->referencedColumns( context.expressionContext() ); // <-- prepares any expressions, too

      //QgsDebugMsgLevel( QString( "Data defined referenced columns:" ) + cols.join( "," ), 4 );
      Q_FOREACH ( const QString& name, cols )
      {
        attributeNames.append( name );
      }
    }
  }

  // NOW INITIALIZE QgsPalLayerSettings

  // TODO: ideally these (non-configuration) members should get out of QgsPalLayerSettings to here
  // (together with registerFeature() & related methods) and QgsPalLayerSettings just stores config

  //raster and vector scale factors
  lyr.vectorScaleFactor = context.scaleFactor();
  lyr.rasterCompressFactor = context.rasterScaleFactor();

  // save the pal layer to our layer context (with some additional info)
  lyr.fieldIndex = mFields.fieldNameIndex( lyr.fieldName );

  lyr.xform = &mapSettings.mapToPixel();
  lyr.ct = nullptr;
  if ( mapSettings.hasCrsTransformEnabled() )
  {
    if ( context.coordinateTransform() )
      // this is context for layer rendering - use its CT as it includes correct datum transform
      lyr.ct = context.coordinateTransform()->clone();
    else
      // otherwise fall back to creating our own CT - this one may not have the correct datum transform!
      lyr.ct = new QgsCoordinateTransform( mCrs, mapSettings.destinationCrs() );
  }
  lyr.ptZero = lyr.xform->toMapCoordinates( 0, 0 );
  lyr.ptOne = lyr.xform->toMapCoordinates( 1, 0 );

  // rect for clipping
  lyr.extentGeom = QgsGeometry::fromRect( mapSettings.visibleExtent() );
  if ( !qgsDoubleNear( mapSettings.rotation(), 0.0 ) )
  {
    //PAL features are prerotated, so extent also needs to be unrotated
    lyr.extentGeom->rotate( -mapSettings.rotation(), mapSettings.visibleExtent().center() );
  }

  lyr.mFeatsSendingToPal = 0;

  return true;
}



QList<QgsLabelFeature*> QgsVectorLayerLabelProvider::labelFeatures( QgsRenderContext &ctx )
{
  if ( !mSource )
  {
    // we have created the provider with "own feature loop" == false
    // so it is assumed that prepare() has been already called followed by registerFeature() calls
    return mLabels;
  }

  QStringList attrNames;
  if ( !prepare( ctx, attrNames ) )
    return QList<QgsLabelFeature*>();

  if ( mRenderer )
    mRenderer->startRender( ctx, mFields );

  QgsRectangle layerExtent = ctx.extent();
  if ( mSettings.ct )
    layerExtent = mSettings.ct->transformBoundingBox( ctx.extent(), QgsCoordinateTransform::ReverseTransform );

  QgsFeatureRequest request;
  request.setFilterRect( layerExtent );
  request.setSubsetOfAttributes( attrNames, mFields );
  QgsFeatureIterator fit = mSource->getFeatures( request );

  QgsExpressionContextScope* symbolScope = new QgsExpressionContextScope();
  ctx.expressionContext().appendScope( symbolScope );
  QgsFeature fet;
  while ( fit.nextFeature( fet ) )
  {
    QScopedPointer<QgsGeometry> obstacleGeometry;
    if ( mRenderer )
    {
      QgsSymbolV2List symbols = mRenderer->originalSymbolsForFeature( fet, ctx );
      if ( !symbols.isEmpty() && fet.constGeometry()->type() == QGis::Point )
      {
        //point feature, use symbol bounds as obstacle
        obstacleGeometry.reset( QgsVectorLayerLabelProvider::getPointObstacleGeometry( fet, ctx, symbols ) );
      }
      if ( !symbols.isEmpty() )
      {
        symbolScope = QgsExpressionContextUtils::updateSymbolScope( symbols.at( 0 ), symbolScope );
      }
    }
    ctx.expressionContext().setFeature( fet );
    registerFeature( fet, ctx, obstacleGeometry.data() );
  }

  if ( ctx.expressionContext().lastScope() == symbolScope )
    delete ctx.expressionContext().popScope();

  if ( mRenderer )
    mRenderer->stopRender( ctx );

  return mLabels;
}

void QgsVectorLayerLabelProvider::registerFeature( QgsFeature& feature, QgsRenderContext& context, QgsGeometry* obstacleGeometry )
{
  QgsLabelFeature* label = nullptr;
  mSettings.registerFeature( feature, context, &label, obstacleGeometry );
  if ( label )
    mLabels << label;
}

QgsGeometry* QgsVectorLayerLabelProvider::getPointObstacleGeometry( QgsFeature& fet, QgsRenderContext& context, const QgsSymbolV2List& symbols )
{
  if ( !fet.constGeometry() || fet.constGeometry()->isEmpty() || fet.constGeometry()->type() != QGis::Point )
    return nullptr;

  bool isMultiPoint = fet.constGeometry()->geometry()->nCoordinates() > 1;
  QgsAbstractGeometryV2* obstacleGeom = nullptr;
  if ( isMultiPoint )
    obstacleGeom = new QgsMultiPolygonV2();

  // for each point
  for ( int i = 0; i < fet.constGeometry()->geometry()->nCoordinates(); ++i )
  {
    QRectF bounds;
    QgsPointV2 p =  fet.constGeometry()->geometry()->vertexAt( QgsVertexId( i, 0, 0 ) );
    double x = p.x();
    double y = p.y();
    double z = 0; // dummy variable for coordinate transforms

    //transform point to pixels
    if ( context.coordinateTransform() )
    {
      context.coordinateTransform()->transformInPlace( x, y, z );
    }
    context.mapToPixel().transformInPlace( x, y );

    QPointF pt( x, y );
    Q_FOREACH ( QgsSymbolV2* symbol, symbols )
    {
      if ( symbol->type() == QgsSymbolV2::Marker )
      {
        if ( bounds.isValid() )
          bounds = bounds.united( static_cast< QgsMarkerSymbolV2* >( symbol )->bounds( pt, context, fet ) );
        else
          bounds = static_cast< QgsMarkerSymbolV2* >( symbol )->bounds( pt, context, fet );
      }
    }

    //convert bounds to a geometry
    QgsLineStringV2* boundLineString = new QgsLineStringV2();
    boundLineString->addVertex( QgsPointV2( bounds.topLeft() ) );
    boundLineString->addVertex( QgsPointV2( bounds.topRight() ) );
    boundLineString->addVertex( QgsPointV2( bounds.bottomRight() ) );
    boundLineString->addVertex( QgsPointV2( bounds.bottomLeft() ) );

    //then transform back to map units
    //TODO - remove when labeling is refactored to use screen units
    for ( int i = 0; i < boundLineString->numPoints(); ++i )
    {
      QgsPoint point = context.mapToPixel().toMapCoordinates( boundLineString->xAt( i ), boundLineString->yAt( i ) );
      boundLineString->setXAt( i, point.x() );
      boundLineString->setYAt( i, point.y() );
    }
    if ( context.coordinateTransform() )
    {
      boundLineString->transform( *context.coordinateTransform(), QgsCoordinateTransform::ReverseTransform );
    }
    boundLineString->close();

    QgsPolygonV2* obstaclePolygon = new QgsPolygonV2();
    obstaclePolygon->setExteriorRing( boundLineString );

    if ( isMultiPoint )
    {
      static_cast<QgsMultiPolygonV2*>( obstacleGeom )->addGeometry( obstaclePolygon );
    }
    else
    {
      obstacleGeom = obstaclePolygon;
    }
  }

  return new QgsGeometry( obstacleGeom );
}

void QgsVectorLayerLabelProvider::drawLabel( QgsRenderContext& context, pal::LabelPosition* label ) const
{
  if ( !mSettings.drawLabels )
    return;

  QgsTextLabelFeature* lf = dynamic_cast<QgsTextLabelFeature*>( label->getFeaturePart()->feature() );

  // Copy to temp, editable layer settings
  // these settings will be changed by any data defined values, then used for rendering label components
  // settings may be adjusted during rendering of components
  QgsPalLayerSettings tmpLyr( mSettings );

  // apply any previously applied data defined settings for the label
  const QMap< QgsPalLayerSettings::DataDefinedProperties, QVariant >& ddValues = lf->dataDefinedValues();

  //font
  QFont dFont = lf->definedFont();
  QgsDebugMsgLevel( QString( "PAL font tmpLyr: %1, Style: %2" ).arg( tmpLyr.textFont.toString(), tmpLyr.textFont.styleName() ), 4 );
  QgsDebugMsgLevel( QString( "PAL font definedFont: %1, Style: %2" ).arg( dFont.toString(), dFont.styleName() ), 4 );
  tmpLyr.textFont = dFont;

  if ( tmpLyr.multilineAlign == QgsPalLayerSettings::MultiFollowPlacement )
  {
    //calculate font alignment based on label quadrant
    switch ( label->getQuadrant() )
    {
      case LabelPosition::QuadrantAboveLeft:
      case LabelPosition::QuadrantLeft:
      case LabelPosition::QuadrantBelowLeft:
        tmpLyr.multilineAlign = QgsPalLayerSettings::MultiRight;
        break;
      case LabelPosition::QuadrantAbove:
      case LabelPosition::QuadrantOver:
      case LabelPosition::QuadrantBelow:
        tmpLyr.multilineAlign = QgsPalLayerSettings::MultiCenter;
        break;
      case LabelPosition::QuadrantAboveRight:
      case LabelPosition::QuadrantRight:
      case LabelPosition::QuadrantBelowRight:
        tmpLyr.multilineAlign = QgsPalLayerSettings::MultiLeft;
        break;
    }
  }

  // update tmpLyr with any data defined text style values
  QgsPalLabeling::dataDefinedTextStyle( tmpLyr, ddValues );

  // update tmpLyr with any data defined text buffer values
  QgsPalLabeling::dataDefinedTextBuffer( tmpLyr, ddValues );

  // update tmpLyr with any data defined text formatting values
  QgsPalLabeling::dataDefinedTextFormatting( tmpLyr, ddValues );

  // update tmpLyr with any data defined shape background values
  QgsPalLabeling::dataDefinedShapeBackground( tmpLyr, ddValues );

  // update tmpLyr with any data defined drop shadow values
  QgsPalLabeling::dataDefinedDropShadow( tmpLyr, ddValues );

  tmpLyr.showingShadowRects = mEngine->testFlag( QgsLabelingEngineV2::DrawShadowRects );

  // Render the components of a label in reverse order
  //   (backgrounds -> text)

  if ( tmpLyr.shadowDraw && tmpLyr.shadowUnder == QgsPalLayerSettings::ShadowLowest )
  {
    if ( tmpLyr.shapeDraw )
    {
      tmpLyr.shadowUnder = QgsPalLayerSettings::ShadowShape;
    }
    else if ( tmpLyr.bufferDraw )
    {
      tmpLyr.shadowUnder = QgsPalLayerSettings::ShadowBuffer;
    }
    else
    {
      tmpLyr.shadowUnder = QgsPalLayerSettings::ShadowText;
    }
  }

  if ( tmpLyr.shapeDraw )
  {
    drawLabelPrivate( label, context, tmpLyr, QgsPalLabeling::LabelShape );
  }

  if ( tmpLyr.bufferDraw )
  {
    drawLabelPrivate( label, context, tmpLyr, QgsPalLabeling::LabelBuffer );
  }

  drawLabelPrivate( label, context, tmpLyr, QgsPalLabeling::LabelText );

  // add to the results
  QString labeltext = label->getFeaturePart()->feature()->labelText();
  mEngine->results()->mLabelSearchTree->insertLabel( label, label->getFeaturePart()->featureId(), mLayerId, labeltext, dFont, false, lf->hasFixedPosition(), mProviderId );
}


void QgsVectorLayerLabelProvider::drawLabelPrivate( pal::LabelPosition* label, QgsRenderContext& context, QgsPalLayerSettings& tmpLyr, QgsPalLabeling::DrawLabelType drawType, double dpiRatio ) const
{
  // NOTE: this is repeatedly called for multi-part labels
  QPainter* painter = context.painter();
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

  QgsLabelComponent component;
  component.setDpiRatio( dpiRatio );

  QgsPoint outPt = xform.transform( label->getX(), label->getY() );
//  QgsPoint outPt2 = xform->transform( label->getX() + label->getWidth(), label->getY() + label->getHeight() );
//  QRectF labelRect( 0, 0, outPt2.x() - outPt.x(), outPt2.y() - outPt.y() );

  component.setOrigin( outPt );
  component.setRotation( label->getAlpha() );

  if ( mEngine->testFlag( QgsLabelingEngineV2::DrawLabelRectOnly ) )  // TODO: this should get directly to labeling engine
  {
    //debugging rect
    if ( drawType != QgsPalLabeling::LabelText )
      return;

    QgsPoint outPt2 = xform.transform( label->getX() + label->getWidth(), label->getY() + label->getHeight() );
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

    if ( label->getNextPart() )
      drawLabelPrivate( label->getNextPart(), context, tmpLyr, drawType, dpiRatio );

    return;
  }

  if ( drawType == QgsPalLabeling::LabelShape )
  {
    // get rotated label's center point
    QgsPoint centerPt( outPt );
    QgsPoint outPt2 = xform.transform( label->getX() + label->getWidth() / 2,
                                       label->getY() + label->getHeight() / 2 );

    double xc = outPt2.x() - outPt.x();
    double yc = outPt2.y() - outPt.y();

    double angle = -label->getAlpha();
    double xd = xc * cos( angle ) - yc * sin( angle );
    double yd = xc * sin( angle ) + yc * cos( angle );

    centerPt.setX( centerPt.x() + xd );
    centerPt.setY( centerPt.y() + yd );

    component.setCenter( centerPt );
    component.setSize( QgsPoint( label->getWidth(), label->getHeight() ) );

    QgsPalLabeling::drawLabelBackground( context, component, tmpLyr );
  }

  else if ( drawType == QgsPalLabeling::LabelBuffer
            || drawType == QgsPalLabeling::LabelText )
  {

    // TODO: optimize access :)
    QgsTextLabelFeature* lf = static_cast<QgsTextLabelFeature*>( label->getFeaturePart()->feature() );
    QString txt = lf->text( label->getPartId() );
    QFontMetricsF* labelfm = lf->labelFontMetrics();

    //add the direction symbol if needed
    if ( !txt.isEmpty() && tmpLyr.placement == QgsPalLayerSettings::Line &&
         tmpLyr.addDirectionSymbol )
    {
      bool prependSymb = false;
      QString symb = tmpLyr.rightDirectionSymbol;

      if ( label->getReversed() )
      {
        prependSymb = true;
        symb = tmpLyr.leftDirectionSymbol;
      }

      if ( tmpLyr.reverseDirectionSymbol )
      {
        if ( symb == tmpLyr.rightDirectionSymbol )
        {
          prependSymb = true;
          symb = tmpLyr.leftDirectionSymbol;
        }
        else
        {
          prependSymb = false;
          symb = tmpLyr.rightDirectionSymbol;
        }
      }

      if ( tmpLyr.placeDirectionSymbol == QgsPalLayerSettings::SymbolAbove )
      {
        prependSymb = true;
        symb = symb + QLatin1String( "\n" );
      }
      else if ( tmpLyr.placeDirectionSymbol == QgsPalLayerSettings::SymbolBelow )
      {
        prependSymb = false;
        symb = QLatin1String( "\n" ) + symb;
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

    //QgsDebugMsgLevel( "drawLabel " + txt, 4 );
    QStringList multiLineList = QgsPalLabeling::splitToLines( txt, tmpLyr.wrapChar );
    int lines = multiLineList.size();

    double labelWidest = 0.0;
    for ( int i = 0; i < lines; ++i )
    {
      double labelWidth = labelfm->width( multiLineList.at( i ) );
      if ( labelWidth > labelWidest )
      {
        labelWidest = labelWidth;
      }
    }

    double labelHeight = labelfm->ascent() + labelfm->descent(); // ignore +1 for baseline
    //  double labelHighest = labelfm->height() + ( double )(( lines - 1 ) * labelHeight * tmpLyr.multilineHeight );

    // needed to move bottom of text's descender to within bottom edge of label
    double ascentOffset = 0.25 * labelfm->ascent(); // labelfm->descent() is not enough

    for ( int i = 0; i < lines; ++i )
    {
      painter->save();
#if 0 // TODO: generalize some of this
      LabelPosition* lp = label;
      double w = lp->getWidth();
      double h = lp->getHeight();
      double cx = lp->getX() + w / 2.0;
      double cy = lp->getY() + h / 2.0;
      double scale = 1.0 / xform->mapUnitsPerPixel();
      double rotation = xform->mapRotation();
      double sw = w * scale;
      double sh = h * scale;
      QRectF rect( -sw / 2, -sh / 2, sw, sh );
      painter->translate( xform->transform( QPointF( cx, cy ) ).toQPointF() );
      if ( rotation )
      {
        // Only if not horizontal
        if ( lp->getFeaturePart()->getLayer()->getArrangement() != P_POINT &&
             lp->getFeaturePart()->getLayer()->getArrangement() != P_POINT_OVER &&
             lp->getFeaturePart()->getLayer()->getArrangement() != P_HORIZ )
        {
          painter->rotate( rotation );
        }
      }
      painter->translate( rect.bottomLeft() );
      painter->rotate( -lp->getAlpha() * 180 / M_PI );
#else
      painter->translate( QPointF( outPt.x(), outPt.y() ) );
      painter->rotate( -label->getAlpha() * 180 / M_PI );
#endif

      // scale down painter: the font size has been multiplied by raster scale factor
      // to workaround a Qt font scaling bug with small font sizes
      painter->scale( 1.0 / tmpLyr.rasterCompressFactor, 1.0 / tmpLyr.rasterCompressFactor );

      // figure x offset for horizontal alignment of multiple lines
      double xMultiLineOffset = 0.0;
      double labelWidth = labelfm->width( multiLineList.at( i ) );
      if ( lines > 1 && tmpLyr.multilineAlign != QgsPalLayerSettings::MultiLeft )
      {
        double labelWidthDiff = labelWidest - labelWidth;
        if ( tmpLyr.multilineAlign == QgsPalLayerSettings::MultiCenter )
        {
          labelWidthDiff /= 2;
        }
        xMultiLineOffset = labelWidthDiff;
        //QgsDebugMsgLevel( QString( "xMultiLineOffset: %1" ).arg( xMultiLineOffset ), 4 );
      }

      double yMultiLineOffset = ( lines - 1 - i ) * labelHeight * tmpLyr.multilineHeight;
      painter->translate( QPointF( xMultiLineOffset, - ascentOffset - yMultiLineOffset ) );

      component.setText( multiLineList.at( i ) );
      component.setSize( QgsPoint( labelWidth, labelHeight ) );
      component.setOffset( QgsPoint( 0.0, -ascentOffset ) );
      component.setRotation( -component.rotation() * 180 / M_PI );
      component.setRotationOffset( 0.0 );

      if ( drawType == QgsPalLabeling::LabelBuffer )
      {
        // draw label's buffer
        QgsPalLabeling::drawLabelBuffer( context, component, tmpLyr );
      }
      else
      {
        // draw label's text, QPainterPath method
        QPainterPath path;
        path.setFillRule( Qt::WindingFill );
        path.addText( 0, 0, tmpLyr.textFont, component.text() );

        // store text's drawing in QPicture for drop shadow call
        QPicture textPict;
        QPainter textp;
        textp.begin( &textPict );
        textp.setPen( Qt::NoPen );
        textp.setBrush( tmpLyr.textColor );
        textp.drawPath( path );
        // TODO: why are some font settings lost on drawPicture() when using drawText() inside QPicture?
        //       e.g. some capitalization options, but not others
        //textp.setFont( tmpLyr.textFont );
        //textp.setPen( tmpLyr.textColor );
        //textp.drawText( 0, 0, component.text() );
        textp.end();

        if ( tmpLyr.shadowDraw && tmpLyr.shadowUnder == QgsPalLayerSettings::ShadowText )
        {
          component.setPicture( &textPict );
          component.setPictureBuffer( 0.0 ); // no pen width to deal with
          component.setOrigin( QgsPoint( 0.0, 0.0 ) );

          QgsPalLabeling::drawLabelShadow( context, component, tmpLyr );
        }

        // paint the text
        if ( context.useAdvancedEffects() )
        {
          painter->setCompositionMode( tmpLyr.blendMode );
        }

        // scale for any print output or image saving @ specific dpi
        painter->scale( component.dpiRatio(), component.dpiRatio() );

        if ( mEngine->testFlag( QgsLabelingEngineV2::RenderOutlineLabels ) )
        {
          // draw outlined text
          _fixQPictureDPI( painter );
          painter->drawPicture( 0, 0, textPict );
        }
        else
        {
          // draw text as text (for SVG and PDF exports)
          painter->setFont( tmpLyr.textFont );
          painter->setPen( tmpLyr.textColor );
          painter->setRenderHint( QPainter::TextAntialiasing );
          painter->drawText( 0, 0, component.text() );
        }
      }
      painter->restore();
    }
  }

  // NOTE: this used to be within above multi-line loop block, at end. (a mistake since 2010? [LS])
  if ( label->getNextPart() )
    drawLabelPrivate( label->getNextPart(), context, tmpLyr, drawType, dpiRatio );
}
