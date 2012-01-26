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
  r->setDisplacementGroups( mDisplacementGroups );
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

bool QgsPointDisplacementRenderer::renderFeature( QgsFeature& feature, QgsRenderContext& context, int layer, bool selected, bool drawVertexMarker )
{
  Q_UNUSED( drawVertexMarker );
  //point position in screen coords
  QgsGeometry* geom = feature.geometry();
  QGis::WkbType geomType = geom->wkbType();
  if ( geomType != QGis::WKBPoint && geomType != QGis::WKBPoint25D )
  {
    //can only render point type
    return false;
  }
  QPointF pt;
  _getPoint( pt, context, geom->asWkb() );


  //get list of labels and symbols
  QStringList labelAttributeList;
  QList<QgsMarkerSymbolV2*> symbolList;

  if ( mDisplacementIds.contains( feature.id() ) )
  {
    //create the symbol for the whole display group if the id is the first entry in a display group
    QList<QMap<QgsFeatureId, QgsFeature> >::iterator it = mDisplacementGroups.begin();
    for ( ; it != mDisplacementGroups.end(); ++it )
    {
      //create the symbol for the whole display group if the id is the first entry in a display group
      if ( feature.id() == it->begin().key() )
      {
        QMap<QgsFeatureId, QgsFeature>::iterator attIt = it->begin();
        for ( ; attIt != it->end(); ++attIt )
        {
          if ( mDrawLabels )
          {
            labelAttributeList << getLabel( attIt.value() );
          }
          else
          {
            labelAttributeList << QString();
          }
          symbolList << dynamic_cast<QgsMarkerSymbolV2*>( mRenderer->symbolForFeature( attIt.value() ) );
        }
      }
    }
  }
  else //only one feature
  {
    symbolList << dynamic_cast<QgsMarkerSymbolV2*>( mRenderer->symbolForFeature( feature ) );
    if ( mDrawLabels )
    {
      labelAttributeList << getLabel( feature );
    }
    else
    {
      labelAttributeList << QString();
    }
  }

  if ( symbolList.isEmpty() && labelAttributeList.isEmpty() )
  {
    return true; //display all point symbols for one posi
  }


  //draw symbol
  double diagonal = 0;
  double currentWidthFactor; //scale symbol size to map unit and output resolution

  QList<QgsMarkerSymbolV2*>::const_iterator it = symbolList.constBegin();
  for ( ; it != symbolList.constEnd(); ++it )
  {
    if ( *it )
    {
      currentWidthFactor = QgsSymbolLayerV2Utils::lineWidthScaleFactor( context, ( *it )->outputUnit() );
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
      mCenterSymbol->renderPoint( pt, &feature, context, layer, selected );
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
  return true;
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

void QgsPointDisplacementRenderer::startRender( QgsRenderContext& context, const QgsVectorLayer *vlayer )
{
  mRenderer->startRender( context, vlayer );

  //create groups with features that have the same position
  createDisplacementGroups( const_cast<QgsVectorLayer*>( vlayer ), context.extent() );
  printInfoDisplacementGroups(); //just for debugging

  if ( mLabelAttributeName.isEmpty() )
  {
    mLabelIndex = -1;
  }
  else
  {
    mLabelIndex = vlayer->fieldNameIndex( mLabelAttributeName );
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
    mCenterSymbol->startRender( context, vlayer );
  }
}

void QgsPointDisplacementRenderer::stopRender( QgsRenderContext& context )
{
  QgsDebugMsg( "QgsPointDisplacementRenderer::stopRender" );
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
  rendererElement.setAttribute( "circleWidth", mCircleWidth );
  rendererElement.setAttribute( "circleColor", QgsSymbolLayerV2Utils::encodeColor( mCircleColor ) );
  rendererElement.setAttribute( "labelColor", QgsSymbolLayerV2Utils::encodeColor( mLabelColor ) );
  rendererElement.setAttribute( "circleRadiusAddition", mCircleRadiusAddition );
  rendererElement.setAttribute( "maxLabelScaleDenominator", mMaxLabelScaleDenominator );

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

QgsLegendSymbolList QgsPointDisplacementRenderer::legendSymbolItems()
{
  if ( mRenderer )
  {
    return mRenderer->legendSymbolItems();
  }
  return QgsLegendSymbolList();
}

void QgsPointDisplacementRenderer::createDisplacementGroups( QgsVectorLayer* vlayer, const QgsRectangle& viewExtent )
{
  if ( !vlayer || ( vlayer->wkbType() != QGis::WKBPoint && vlayer->wkbType() != QGis::WKBPoint25D ) )
  {
    return;
  }

  mDisplacementGroups.clear();
  mDisplacementIds.clear();

  //use a spatial index to check if there is already a point at a position
  QgsSpatialIndex spatialIndex;

  //attributes
  QgsAttributeList attList;
  QList<QString> attributeStrings = usedAttributes();
  QList<QString>::const_iterator attStringIt = attributeStrings.constBegin();
  for ( ; attStringIt != attributeStrings.constEnd(); ++attStringIt )
  {
    attList.push_back( vlayer->fieldNameIndex( *attStringIt ) );
  }

  QgsFeature f;
  QList<QgsFeatureId> intersectList;

  vlayer->select( attList, viewExtent, true, false );
  while ( vlayer->nextFeature( f ) )
  {
    intersectList.clear();

    //check, if there is already a point at that position
    if ( f.geometry() )
    {
      intersectList = spatialIndex.intersects( searchRect( f.geometry()->asPoint() ) );
      if ( intersectList.empty() )
      {
        spatialIndex.insertFeature( f );
      }
      else
      {
        //go through all the displacement group maps and search an entry where the id equals the result of the spatial search
        QgsFeatureId existingEntry = intersectList.at( 0 );
        bool found = false;
        QList< QMap<QgsFeatureId, QgsFeature> >::iterator it = mDisplacementGroups.begin();
        for ( ; it != mDisplacementGroups.end(); ++it )
        {
          if ( it->size() > 0 && it->contains( existingEntry ) )
          {
            found = true;
            QgsFeature feature;
            it->insert( f.id(), f );
            mDisplacementIds.insert( f.id() );
            break;
          }
        }

        if ( !found )//insert the already existing feature and the new one into a map
        {
          QMap<QgsFeatureId, QgsFeature> newMap;
          QgsFeature existingFeature;
          vlayer->featureAtId( existingEntry, existingFeature );
          newMap.insert( existingEntry, existingFeature );
          mDisplacementIds.insert( existingEntry );
          newMap.insert( f.id(), f );
          mDisplacementIds.insert( f.id() );
          mDisplacementGroups.push_back( newMap );
        }
      }
    }
  }
  //refresh the selection because the vector layer is going to step through all features now
  vlayer->select( attList, viewExtent, true, false );
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
  QgsDebugMsg( "********all displacement ids*********" );
  QSet<QgsFeatureId>::const_iterator iIt = mDisplacementIds.constBegin();
  for ( ; iIt != mDisplacementIds.constEnd(); ++iIt )
  {
    QgsDebugMsg( FID_TO_STRING( *iIt ) );
  }
}

void QgsPointDisplacementRenderer::setDisplacementGroups( const QList< QMap<QgsFeatureId, QgsFeature> >& list )
{
  mDisplacementGroups = list;
  mDisplacementIds.clear();

  QList<QMap<QgsFeatureId, QgsFeature> >::const_iterator list_it = mDisplacementGroups.constBegin();
  for ( ; list_it != mDisplacementGroups.constEnd(); ++list_it )
  {
    QMap<QgsFeatureId, QgsFeature>::const_iterator map_it = list_it->constBegin();
    for ( ; map_it != list_it->constEnd(); ++map_it )
    {
      mDisplacementIds.insert( map_it.key() );
    }
  }
}

QString QgsPointDisplacementRenderer::getLabel( const QgsFeature& f )
{
  QString attribute;
  QgsAttributeMap attMap = f.attributeMap();
  if ( attMap.size() > 0 )
  {
    QgsAttributeMap::const_iterator valIt = attMap.find( mLabelIndex );
    if ( valIt != attMap.constEnd() )
    {
      attribute = valIt->toString();
    }
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

void QgsPointDisplacementRenderer::drawSymbols( QgsFeature& f, QgsRenderContext& context, const QList<QgsMarkerSymbolV2*>& symbolList, const QList<QPointF>& symbolPositions, bool selected )
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
