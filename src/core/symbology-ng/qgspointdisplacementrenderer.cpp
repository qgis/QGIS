/***************************************************************************
         qgspointdisplacementrenderer.cpp
         --------------------------------
  begin                : January 26, 2010
  copyright            : (C) 2010 by Marco Hugentobler
  email                : marco at hugis dot net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspointdisplacementrenderer.h"
#include "qgsgeometry.h"
#include "qgslogger.h"
#include "qgsspatialindex.h"
#include "qgssymbolv2.h"
#include "qgssymbollayerv2utils.h"
#include "qgsvectorlayer.h"

#include <QDomElement>
#include <QPainter>

#include <cmath>

QgsPointDisplacementRenderer::QgsPointDisplacementRenderer( const QString& labelAttributeName )
    : QgsFeatureRendererV2( "pointDisplacement" )
    , mLabelAttributeName( labelAttributeName )
    , mLabelIndex( -1 )
    , mTolerance( 0.00001 )
    , mCircleWidth( 0.4 )
    , mCircleColor( QColor( 125, 125, 125 ) )
    , mCircleRadiusAddition( 0 )
    , mMaxLabelScaleDenominator( -1 )
{
  mRenderer = QgsFeatureRendererV2::defaultRenderer( QGis::Point );
  mCenterSymbol = new QgsMarkerSymbolV2(); //the symbol for the center of a displacement group
  mDrawLabels = true;
}

QgsPointDisplacementRenderer::~QgsPointDisplacementRenderer()
{
  delete mCenterSymbol;
  delete mRenderer;
}

QgsFeatureRendererV2* QgsPointDisplacementRenderer::clone()
{
  QgsPointDisplacementRenderer* r = new QgsPointDisplacementRenderer( mLabelAttributeName );
  r->setEmbeddedRenderer( mRenderer->clone() );
  r->setCircleWidth( mCircleWidth );
  r->setCircleColor( mCircleColor );
  r->setLabelFont( mLabelFont );
  r->setLabelColor( mLabelColor );
  r->setCircleRadiusAddition( mCircleRadiusAddition );
  r->setMaxLabelScaleDenominator( mMaxLabelScaleDenominator );
  r->setTolerance( mTolerance );
  if ( mCenterSymbol )
  {
    r->setCenterSymbol( dynamic_cast<QgsMarkerSymbolV2*>( mCenterSymbol->clone() ) );
  }
  return r;
}

void QgsPointDisplacementRenderer::toSld( QDomDocument& doc, QDomElement &element ) const
{
  mRenderer->toSld( doc, element );
}


bool QgsPointDisplacementRenderer::renderFeature( QgsFeature& feature, QgsRenderContext& context, int layer, bool selected, bool drawVertexMarker )
{
  Q_UNUSED( drawVertexMarker );
  Q_UNUSED( context );
  Q_UNUSED( layer );

  //check, if there is already a point at that position
  if ( !feature.geometry() )
    return false;

  //point position in screen coords
  QgsGeometry* geom = feature.geometry();
  QGis::WkbType geomType = geom->wkbType();
  if ( geomType != QGis::WKBPoint && geomType != QGis::WKBPoint25D )
  {
    //can only render point type
    return false;
  }

  if ( selected )
    mSelectedFeatures.insert( feature.id() );

  QList<QgsFeatureId> intersectList = mSpatialIndex->intersects( searchRect( feature.geometry()->asPoint() ) );
  if ( intersectList.empty() )
  {
    mSpatialIndex->insertFeature( feature );
    // create new group
    DisplacementGroup newGroup;
    newGroup.insert( feature.id(), feature );
    mDisplacementGroups.push_back( newGroup );
    // add to group index
    mGroupIndex.insert( feature.id(), mDisplacementGroups.count() - 1 );
    return true;
  }

  //go through all the displacement group maps and search an entry where the id equals the result of the spatial search
  QgsFeatureId existingEntry = intersectList.at( 0 );

  int groupIdx = mGroupIndex[ existingEntry ];
  DisplacementGroup& group = mDisplacementGroups[groupIdx];

  // add to a group
  group.insert( feature.id(), feature );
  // add to group index
  mGroupIndex.insert( feature.id(), groupIdx );
  return true;
}

void QgsPointDisplacementRenderer::drawGroup( const DisplacementGroup& group, QgsRenderContext& context )
{
  const QgsFeature& feature = group.begin().value();
  bool selected = mSelectedFeatures.contains( feature.id() ); // maybe we should highlight individual features instead of the whole group?

  QPointF pt;
  _getPoint( pt, context, feature.geometry()->asWkb() );

  //get list of labels and symbols
  QStringList labelAttributeList;
  QList<QgsMarkerSymbolV2*> symbolList;

  for ( DisplacementGroup::const_iterator attIt = group.constBegin(); attIt != group.constEnd(); ++attIt )
  {
    labelAttributeList << ( mDrawLabels ? getLabel( attIt.value() ) : QString() );
    QgsFeature& f = const_cast<QgsFeature&>( attIt.value() ); // other parts of API use non-const ref to QgsFeature :-/
    symbolList << dynamic_cast<QgsMarkerSymbolV2*>( firstSymbolForFeature( mRenderer, f ) );
  }

  //draw symbol
  double diagonal = 0;
  double currentWidthFactor; //scale symbol size to map unit and output resolution

  QList<QgsMarkerSymbolV2*>::const_iterator it = symbolList.constBegin();
  for ( ; it != symbolList.constEnd(); ++it )
  {
    if ( *it )
    {
      currentWidthFactor = QgsSymbolLayerV2Utils::lineWidthScaleFactor( context, ( *it )->outputUnit(), ( *it )->mapUnitScale() );
      double currentDiagonal = sqrt( 2 * (( *it )->size() * ( *it )->size() ) ) * currentWidthFactor;
      if ( currentDiagonal > diagonal )
      {
        diagonal = currentDiagonal;
      }
    }
  }


  QgsSymbolV2RenderContext symbolContext( context, QgsSymbolV2::MM, 1.0, selected );
  double circleAdditionPainterUnits = symbolContext.outputLineWidth( mCircleRadiusAddition );
  double radius = qMax(( diagonal / 2 ), labelAttributeList.size() * diagonal / 2 / M_PI ) + circleAdditionPainterUnits;

  //draw Circle
  drawCircle( radius, symbolContext, pt, symbolList.size() );

  QList<QPointF> symbolPositions;
  QList<QPointF> labelPositions;
  calculateSymbolAndLabelPositions( pt, labelAttributeList.size(), radius, diagonal, symbolPositions, labelPositions );

  //draw mid point
  if ( labelAttributeList.size() > 1 )
  {
    if ( mCenterSymbol )
    {
      mCenterSymbol->renderPoint( pt, &feature, context, -1, selected );
    }
    else
    {
      context.painter()->drawRect( QRectF( pt.x() - symbolContext.outputLineWidth( 1 ), pt.y() - symbolContext.outputLineWidth( 1 ), symbolContext.outputLineWidth( 2 ), symbolContext.outputLineWidth( 2 ) ) );
    }
  }

  //draw symbols on the circle
  drawSymbols( feature, context, symbolList, symbolPositions, selected );
  //and also the labels
  drawLabels( pt, symbolContext, labelPositions, labelAttributeList );
}

void QgsPointDisplacementRenderer::setEmbeddedRenderer( QgsFeatureRendererV2* r )
{
  delete mRenderer;
  mRenderer = r;
}

QgsSymbolV2* QgsPointDisplacementRenderer::symbolForFeature( QgsFeature& feature )
{
  Q_UNUSED( feature );
  return 0; //not used any more
}

void QgsPointDisplacementRenderer::startRender( QgsRenderContext& context, const QgsFields& fields )
{
  mRenderer->startRender( context, fields );

  mDisplacementGroups.clear();
  mGroupIndex.clear();
  mSpatialIndex = new QgsSpatialIndex;
  mSelectedFeatures.clear();

  if ( mLabelAttributeName.isEmpty() )
  {
    mLabelIndex = -1;
  }
  else
  {
    mLabelIndex = fields.fieldNameIndex( mLabelAttributeName );
  }

  if ( mMaxLabelScaleDenominator > 0 && context.rendererScale() > mMaxLabelScaleDenominator )
  {
    mDrawLabels = false;
  }
  else
  {
    mDrawLabels = true;
  }

  if ( mCenterSymbol )
  {
    mCenterSymbol->startRender( context, &fields );
  }
}

void QgsPointDisplacementRenderer::stopRender( QgsRenderContext& context )
{
  QgsDebugMsg( "QgsPointDisplacementRenderer::stopRender" );

  //printInfoDisplacementGroups(); //just for debugging

  for ( QList<DisplacementGroup>::const_iterator it = mDisplacementGroups.begin(); it != mDisplacementGroups.end(); ++it )
    drawGroup( *it, context );

  mDisplacementGroups.clear();
  mGroupIndex.clear();
  delete mSpatialIndex;
  mSpatialIndex = 0;
  mSelectedFeatures.clear();

  mRenderer->stopRender( context );
  if ( mCenterSymbol )
  {
    mCenterSymbol->stopRender( context );
  }
}

QList<QString> QgsPointDisplacementRenderer::usedAttributes()
{
  QList<QString> attributeList;
  if ( !mLabelAttributeName.isEmpty() )
  {
    attributeList.push_back( mLabelAttributeName );
  }
  if ( mRenderer )
  {
    attributeList += mRenderer->usedAttributes();
  }
  return attributeList;
}

QgsSymbolV2List QgsPointDisplacementRenderer::symbols()
{
  if ( mRenderer )
  {
    return mRenderer->symbols();
  }
  else
  {
    return QgsSymbolV2List();
  }
}

QgsFeatureRendererV2* QgsPointDisplacementRenderer::create( QDomElement& symbologyElem )
{
  QgsPointDisplacementRenderer* r = new QgsPointDisplacementRenderer();
  r->setLabelAttributeName( symbologyElem.attribute( "labelAttributeName" ) );
  QFont labelFont;
  labelFont.fromString( symbologyElem.attribute( "labelFont", "" ) );
  r->setLabelFont( labelFont );
  r->setCircleWidth( symbologyElem.attribute( "circleWidth", "0.4" ).toDouble() );
  r->setCircleColor( QgsSymbolLayerV2Utils::decodeColor( symbologyElem.attribute( "circleColor", "" ) ) );
  r->setLabelColor( QgsSymbolLayerV2Utils::decodeColor( symbologyElem.attribute( "labelColor", "" ) ) );
  r->setCircleRadiusAddition( symbologyElem.attribute( "circleRadiusAddition", "0.0" ).toDouble() );
  r->setMaxLabelScaleDenominator( symbologyElem.attribute( "maxLabelScaleDenominator", "-1" ).toDouble() );
  r->setTolerance( symbologyElem.attribute( "tolerance", "0.00001" ).toDouble() );

  //look for an embedded renderer <renderer-v2>
  QDomElement embeddedRendererElem = symbologyElem.firstChildElement( "renderer-v2" );
  if ( !embeddedRendererElem.isNull() )
  {
    r->setEmbeddedRenderer( QgsFeatureRendererV2::load( embeddedRendererElem ) );
  }

  //center symbol
  QDomElement centerSymbolElem = symbologyElem.firstChildElement( "symbol" );
  if ( !centerSymbolElem.isNull() )
  {
    r->setCenterSymbol( dynamic_cast<QgsMarkerSymbolV2*>( QgsSymbolLayerV2Utils::loadSymbol( centerSymbolElem ) ) );
  }
  return r;
}

QDomElement QgsPointDisplacementRenderer::save( QDomDocument& doc )
{
  QDomElement rendererElement = doc.createElement( RENDERER_TAG_NAME );
  rendererElement.setAttribute( "type", "pointDisplacement" );
  rendererElement.setAttribute( "labelAttributeName", mLabelAttributeName );
  rendererElement.setAttribute( "labelFont", mLabelFont.toString() );
  rendererElement.setAttribute( "circleWidth", QString::number( mCircleWidth ) );
  rendererElement.setAttribute( "circleColor", QgsSymbolLayerV2Utils::encodeColor( mCircleColor ) );
  rendererElement.setAttribute( "labelColor", QgsSymbolLayerV2Utils::encodeColor( mLabelColor ) );
  rendererElement.setAttribute( "circleRadiusAddition", QString::number( mCircleRadiusAddition ) );
  rendererElement.setAttribute( "maxLabelScaleDenominator", QString::number( mMaxLabelScaleDenominator ) );
  rendererElement.setAttribute( "tolerance", QString::number( mTolerance ) );

  if ( mRenderer )
  {
    QDomElement embeddedRendererElem = mRenderer->save( doc );
    rendererElement.appendChild( embeddedRendererElem );
  }
  if ( mCenterSymbol )
  {
    QDomElement centerSymbolElem = QgsSymbolLayerV2Utils::saveSymbol( "centerSymbol", mCenterSymbol, doc );
    rendererElement.appendChild( centerSymbolElem );
  }
  return rendererElement;
}

QgsLegendSymbologyList QgsPointDisplacementRenderer::legendSymbologyItems( QSize iconSize )
{
  if ( mRenderer )
  {
    return mRenderer->legendSymbologyItems( iconSize );
  }
  return QgsLegendSymbologyList();
}

QgsLegendSymbolList QgsPointDisplacementRenderer::legendSymbolItems( double scaleDenominator, QString rule )
{
  if ( mRenderer )
  {
    return mRenderer->legendSymbolItems( scaleDenominator, rule );
  }
  return QgsLegendSymbolList();
}


QgsRectangle QgsPointDisplacementRenderer::searchRect( const QgsPoint& p ) const
{
  return QgsRectangle( p.x() - mTolerance, p.y() - mTolerance, p.x() + mTolerance, p.y() + mTolerance );
}

void QgsPointDisplacementRenderer::printInfoDisplacementGroups()
{
  int nGroups = mDisplacementGroups.size();
  QgsDebugMsg( "number of displacement groups:" + QString::number( nGroups ) );
  for ( int i = 0; i < nGroups; ++i )
  {
    QgsDebugMsg( "***************displacement group " + QString::number( i ) );
    QMap<QgsFeatureId, QgsFeature>::const_iterator it = mDisplacementGroups.at( i ).constBegin();
    for ( ; it != mDisplacementGroups.at( i ).constEnd(); ++it )
    {
      QgsDebugMsg( FID_TO_STRING( it.key() ) );
    }
  }
}

QString QgsPointDisplacementRenderer::getLabel( const QgsFeature& f )
{
  QString attribute;
  const QgsAttributes& attrs = f.attributes();
  if ( mLabelIndex >= 0 && mLabelIndex < attrs.count() )
  {
    attribute = attrs[mLabelIndex].toString();
  }
  return attribute;
}

void QgsPointDisplacementRenderer::setCenterSymbol( QgsMarkerSymbolV2* symbol )
{
  delete mCenterSymbol;
  mCenterSymbol = symbol;
}



void QgsPointDisplacementRenderer::calculateSymbolAndLabelPositions( const QPointF& centerPoint, int nPosition, double radius,
    double symbolDiagonal, QList<QPointF>& symbolPositions, QList<QPointF>& labelShifts ) const
{
  symbolPositions.clear();
  labelShifts.clear();

  if ( nPosition < 1 )
  {
    return;
  }
  else if ( nPosition == 1 ) //If there is only one feature, draw it exactly at the center position
  {
    symbolPositions.append( centerPoint );
    labelShifts.append( QPointF( symbolDiagonal / 2.0, -symbolDiagonal / 2.0 ) );
    return;
  }

  double fullPerimeter = 2 * M_PI;
  double angleStep = fullPerimeter / nPosition;
  double currentAngle;

  for ( currentAngle = 0.0; currentAngle < fullPerimeter; currentAngle += angleStep )
  {
    double sinusCurrentAngle = sin( currentAngle );
    double cosinusCurrentAngle = cos( currentAngle );
    QPointF positionShift( radius * sinusCurrentAngle, radius * cosinusCurrentAngle );
    QPointF labelShift(( radius + symbolDiagonal / 2 ) * sinusCurrentAngle, ( radius + symbolDiagonal / 2 ) * cosinusCurrentAngle );
    symbolPositions.append( centerPoint + positionShift );
    labelShifts.append( labelShift );
  }
}

void QgsPointDisplacementRenderer::drawCircle( double radiusPainterUnits, QgsSymbolV2RenderContext& context, const QPointF& centerPoint, int nSymbols )
{
  QPainter* p = context.renderContext().painter();
  if ( nSymbols < 2 || !p ) //draw circle only if multiple features
  {
    return;
  }

  //draw Circle
  QPen circlePen( mCircleColor );
  circlePen.setWidthF( context.outputLineWidth( mCircleWidth ) );
  p->setPen( circlePen );
  p->drawArc( QRectF( centerPoint.x() - radiusPainterUnits, centerPoint.y() - radiusPainterUnits, 2 * radiusPainterUnits, 2 * radiusPainterUnits ), 0, 5760 );
}

void QgsPointDisplacementRenderer::drawSymbols( const QgsFeature& f, QgsRenderContext& context, const QList<QgsMarkerSymbolV2*>& symbolList, const QList<QPointF>& symbolPositions, bool selected )
{
  QList<QPointF>::const_iterator symbolPosIt = symbolPositions.constBegin();
  QList<QgsMarkerSymbolV2*>::const_iterator symbolIt = symbolList.constBegin();
  for ( ; symbolPosIt != symbolPositions.constEnd() && symbolIt != symbolList.constEnd(); ++symbolPosIt, ++symbolIt )
  {
    if ( *symbolIt )
    {
      ( *symbolIt )->renderPoint( *symbolPosIt, &f, context, -1, selected );
    }
  }
}

void QgsPointDisplacementRenderer::drawLabels( const QPointF& centerPoint, QgsSymbolV2RenderContext& context, const QList<QPointF>& labelShifts, const QStringList& labelList )
{
  QPainter* p = context.renderContext().painter();
  if ( !p )
  {
    return;
  }

  QPen labelPen( mLabelColor );
  p->setPen( labelPen );

  //scale font (for printing)
  QFont pixelSizeFont = mLabelFont;
  pixelSizeFont.setPixelSize( context.outputLineWidth( mLabelFont.pointSizeF() * 0.3527 ) );
  QFont scaledFont = pixelSizeFont;
  scaledFont.setPixelSize( pixelSizeFont.pixelSize() * context.renderContext().rasterScaleFactor() );
  p->setFont( scaledFont );

  QFontMetricsF fontMetrics( pixelSizeFont );
  QPointF currentLabelShift; //considers the signs to determine the label position

  QList<QPointF>::const_iterator labelPosIt = labelShifts.constBegin();
  QStringList::const_iterator text_it = labelList.constBegin();

  for ( ; labelPosIt != labelShifts.constEnd() && text_it != labelList.constEnd(); ++labelPosIt, ++text_it )
  {
    currentLabelShift = *labelPosIt;
    if ( currentLabelShift.x() < 0 )
    {
      currentLabelShift.setX( currentLabelShift.x() - fontMetrics.width( *text_it ) );
    }
    if ( currentLabelShift.y() > 0 )
    {
      currentLabelShift.setY( currentLabelShift.y() + fontMetrics.ascent() );
    }

    QPointF drawingPoint( centerPoint + currentLabelShift );
    p->save();
    p->translate( drawingPoint.x(), drawingPoint.y() );
    p->scale( 1.0 / context.renderContext().rasterScaleFactor(), 1.0 / context.renderContext().rasterScaleFactor() );
    p->drawText( QPointF( 0, 0 ), *text_it );
    p->restore();
  }
}

QgsSymbolV2* QgsPointDisplacementRenderer::firstSymbolForFeature( QgsFeatureRendererV2* r, QgsFeature& f )
{
  if ( !r )
  {
    return 0;
  }

  QgsSymbolV2List symbolList = r->symbolsForFeature( f );
  if ( symbolList.size() < 1 )
  {
    return 0;
  }

  return symbolList.at( 0 );
}


