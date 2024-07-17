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
#include "qgssymbollayerutils.h"
#include "qgsfontutils.h"
#include "qgspointclusterrenderer.h"
#include "qgsstyleentityvisitor.h"
#include "qgsrenderedfeaturehandlerinterface.h"
#include "qgsmarkersymbol.h"
#include "qgsunittypes.h"
#include "qgscolorutils.h"

#include <QPainter>
#include <cmath>

QgsPointDisplacementRenderer::QgsPointDisplacementRenderer( const QString &labelAttributeName )
  : QgsPointDistanceRenderer( QStringLiteral( "pointDisplacement" ), labelAttributeName )
  , mCircleColor( QColor( 125, 125, 125 ) )
{
  mCenterSymbol.reset( new QgsMarkerSymbol() );
}

QgsPointDisplacementRenderer *QgsPointDisplacementRenderer::clone() const
{
  QgsPointDisplacementRenderer *r = new QgsPointDisplacementRenderer( mLabelAttributeName );
  if ( mRenderer )
    r->setEmbeddedRenderer( mRenderer->clone() );
  r->setCircleWidth( mCircleWidth );
  r->setCircleColor( mCircleColor );
  r->setLabelFont( mLabelFont );
  r->setLabelColor( mLabelColor );
  r->setPlacement( mPlacement );
  r->setCircleRadiusAddition( mCircleRadiusAddition );
  r->setLabelDistanceFactor( mLabelDistanceFactor );
  r->setMinimumLabelScale( mMinLabelScale );
  r->setTolerance( mTolerance );
  r->setToleranceUnit( mToleranceUnit );
  r->setToleranceMapUnitScale( mToleranceMapUnitScale );
  if ( mCenterSymbol )
  {
    r->setCenterSymbol( mCenterSymbol->clone() );
  }
  copyRendererData( r );
  return r;
}

void QgsPointDisplacementRenderer::drawGroup( QPointF centerPoint, QgsRenderContext &context, const ClusteredGroup &group ) const
{

  //calculate max diagonal size from all symbols in group
  double diagonal = 0;
  QVector<double> diagonals( group.size() );
  double currentDiagonal;

  int groupPosition = 0;
  for ( const GroupedFeature &feature : group )
  {
    if ( QgsMarkerSymbol *symbol = feature.symbol() )
    {
      currentDiagonal = M_SQRT2 * symbol->size( context );
      diagonals[groupPosition] = currentDiagonal;
      diagonal = std::max( diagonal, currentDiagonal );

    }
    else
    {
      diagonals[groupPosition] = 0.0;
    }
    groupPosition++;
  }

  QgsSymbolRenderContext symbolContext( context, Qgis::RenderUnit::Millimeters, 1.0, false );

  QList<QPointF> symbolPositions;
  QList<QPointF> labelPositions;
  double circleRadius = -1.0;
  double gridRadius = -1.0;
  int gridSize = -1;

  calculateSymbolAndLabelPositions( symbolContext, centerPoint, group.size(), diagonal, symbolPositions, labelPositions, circleRadius, gridRadius, gridSize, diagonals );

  //only draw circle/grid if there's a pen present - otherwise skip drawing transparent grids
  if ( mCircleColor.isValid() && mCircleColor.alpha() > 0 )
  {
    switch ( mPlacement )
    {
      case Ring:
      case ConcentricRings:
        drawCircle( circleRadius, symbolContext, centerPoint, group.size() );
        break;
      case Grid:
        drawGrid( gridSize, symbolContext, symbolPositions, group.size() );
        break;
    }
  }

  if ( group.size() > 1 )
  {
    //draw mid point
    const QgsFeature firstFeature = group.at( 0 ).feature;
    if ( mCenterSymbol )
    {
      mCenterSymbol->renderPoint( centerPoint, &firstFeature, context, -1, false );
    }
    else
    {
      const double rectSize = symbolContext.renderContext().convertToPainterUnits( 1, Qgis::RenderUnit::Millimeters );
      context.painter()->drawRect( QRectF( centerPoint.x() - rectSize, centerPoint.y() - rectSize, rectSize * 2, rectSize * 2 ) );
    }
  }

  //draw symbols on the circle
  drawSymbols( group, context, symbolPositions );
  //and also the labels
  if ( mLabelIndex >= 0 )
  {
    drawLabels( centerPoint, symbolContext, labelPositions, group );
  }
}


void QgsPointDisplacementRenderer::startRender( QgsRenderContext &context, const QgsFields &fields )
{
  if ( mCenterSymbol )
  {
    mCenterSymbol->startRender( context, fields );
  }

  QgsPointDistanceRenderer::startRender( context, fields );
}

void QgsPointDisplacementRenderer::stopRender( QgsRenderContext &context )
{
  QgsPointDistanceRenderer::stopRender( context );
  if ( mCenterSymbol )
  {
    mCenterSymbol->stopRender( context );
  }
}

QgsFeatureRenderer *QgsPointDisplacementRenderer::create( QDomElement &symbologyElem, const QgsReadWriteContext &context )
{
  QgsPointDisplacementRenderer *r = new QgsPointDisplacementRenderer();
  r->setLabelAttributeName( symbologyElem.attribute( QStringLiteral( "labelAttributeName" ) ) );
  QFont labelFont;
  if ( !QgsFontUtils::setFromXmlChildNode( labelFont, symbologyElem, QStringLiteral( "labelFontProperties" ) ) )
  {
    labelFont.fromString( symbologyElem.attribute( QStringLiteral( "labelFont" ), QString() ) );
  }
  r->setLabelFont( labelFont );
  r->setPlacement( static_cast< Placement >( symbologyElem.attribute( QStringLiteral( "placement" ), QStringLiteral( "0" ) ).toInt() ) );
  r->setCircleWidth( symbologyElem.attribute( QStringLiteral( "circleWidth" ), QStringLiteral( "0.4" ) ).toDouble() );
  r->setCircleColor( QgsColorUtils::colorFromString( symbologyElem.attribute( QStringLiteral( "circleColor" ), QString() ) ) );
  r->setLabelColor( QgsColorUtils::colorFromString( symbologyElem.attribute( QStringLiteral( "labelColor" ), QString() ) ) );
  r->setCircleRadiusAddition( symbologyElem.attribute( QStringLiteral( "circleRadiusAddition" ), QStringLiteral( "0.0" ) ).toDouble() );
  r->setLabelDistanceFactor( symbologyElem.attribute( QStringLiteral( "labelDistanceFactor" ), QStringLiteral( "0.5" ) ).toDouble() );
  r->setMinimumLabelScale( symbologyElem.attribute( QStringLiteral( "maxLabelScaleDenominator" ), QStringLiteral( "-1" ) ).toDouble() );
  r->setTolerance( symbologyElem.attribute( QStringLiteral( "tolerance" ), QStringLiteral( "0.00001" ) ).toDouble() );
  r->setToleranceUnit( QgsUnitTypes::decodeRenderUnit( symbologyElem.attribute( QStringLiteral( "toleranceUnit" ), QStringLiteral( "MapUnit" ) ) ) );
  r->setToleranceMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( symbologyElem.attribute( QStringLiteral( "toleranceUnitScale" ) ) ) );

  //look for an embedded renderer <renderer-v2>
  QDomElement embeddedRendererElem = symbologyElem.firstChildElement( QStringLiteral( "renderer-v2" ) );
  if ( !embeddedRendererElem.isNull() )
  {
    r->setEmbeddedRenderer( QgsFeatureRenderer::load( embeddedRendererElem, context ) );
  }

  //center symbol
  const QDomElement centerSymbolElem = symbologyElem.firstChildElement( QStringLiteral( "symbol" ) );
  if ( !centerSymbolElem.isNull() )
  {
    r->setCenterSymbol( QgsSymbolLayerUtils::loadSymbol<QgsMarkerSymbol>( centerSymbolElem, context ) );
  }
  return r;
}

QgsMarkerSymbol *QgsPointDisplacementRenderer::centerSymbol()
{
  return mCenterSymbol.get();
}

QDomElement QgsPointDisplacementRenderer::save( QDomDocument &doc, const QgsReadWriteContext &context )
{
  QDomElement rendererElement = doc.createElement( RENDERER_TAG_NAME );
  rendererElement.setAttribute( QStringLiteral( "type" ), QStringLiteral( "pointDisplacement" ) );
  rendererElement.setAttribute( QStringLiteral( "labelAttributeName" ), mLabelAttributeName );
  rendererElement.appendChild( QgsFontUtils::toXmlElement( mLabelFont, doc, QStringLiteral( "labelFontProperties" ) ) );
  rendererElement.setAttribute( QStringLiteral( "circleWidth" ), QString::number( mCircleWidth ) );
  rendererElement.setAttribute( QStringLiteral( "circleColor" ), QgsColorUtils::colorToString( mCircleColor ) );
  rendererElement.setAttribute( QStringLiteral( "labelColor" ), QgsColorUtils::colorToString( mLabelColor ) );
  rendererElement.setAttribute( QStringLiteral( "circleRadiusAddition" ), QString::number( mCircleRadiusAddition ) );
  rendererElement.setAttribute( QStringLiteral( "labelDistanceFactor" ), QString::number( mLabelDistanceFactor ) );
  rendererElement.setAttribute( QStringLiteral( "placement" ), static_cast< int >( mPlacement ) );
  rendererElement.setAttribute( QStringLiteral( "maxLabelScaleDenominator" ), QString::number( mMinLabelScale ) );
  rendererElement.setAttribute( QStringLiteral( "tolerance" ), QString::number( mTolerance ) );
  rendererElement.setAttribute( QStringLiteral( "toleranceUnit" ), QgsUnitTypes::encodeUnit( mToleranceUnit ) );
  rendererElement.setAttribute( QStringLiteral( "toleranceUnitScale" ), QgsSymbolLayerUtils::encodeMapUnitScale( mToleranceMapUnitScale ) );

  if ( mRenderer )
  {
    const QDomElement embeddedRendererElem = mRenderer->save( doc, context );
    rendererElement.appendChild( embeddedRendererElem );
  }
  if ( mCenterSymbol )
  {
    const QDomElement centerSymbolElem = QgsSymbolLayerUtils::saveSymbol( QStringLiteral( "centerSymbol" ), mCenterSymbol.get(), doc, context );
    rendererElement.appendChild( centerSymbolElem );
  }

  saveRendererData( doc, rendererElement, context );

  return rendererElement;
}

QSet<QString> QgsPointDisplacementRenderer::usedAttributes( const QgsRenderContext &context ) const
{
  QSet<QString> attr = QgsPointDistanceRenderer::usedAttributes( context );
  if ( mCenterSymbol )
    attr.unite( mCenterSymbol->usedAttributes( context ) );
  return attr;
}

bool QgsPointDisplacementRenderer::accept( QgsStyleEntityVisitorInterface *visitor ) const
{
  if ( !QgsPointDistanceRenderer::accept( visitor ) )
    return false;

  if ( mCenterSymbol )
  {
    QgsStyleSymbolEntity entity( mCenterSymbol.get() );
    if ( !visitor->visit( QgsStyleEntityVisitorInterface::StyleLeaf( &entity, QStringLiteral( "center" ), QObject::tr( "Center Symbol" ) ) ) )
      return false;
  }

  return true;
}

void QgsPointDisplacementRenderer::setCenterSymbol( QgsMarkerSymbol *symbol )
{
  mCenterSymbol.reset( symbol );
}

void QgsPointDisplacementRenderer::calculateSymbolAndLabelPositions( QgsSymbolRenderContext &symbolContext, QPointF centerPoint, int nPosition,
    double symbolDiagonal, QList<QPointF> &symbolPositions, QList<QPointF> &labelShifts, double &circleRadius, double &gridRadius,
    int &gridSize, QVector<double> &diagonals ) const
{
  symbolPositions.clear();
  labelShifts.clear();

  if ( nPosition < 1 )
  {
    return;
  }
  else if ( nPosition == 1 ) //If there is only one feature, draw it exactly at the center position
  {
    const double side = std::sqrt( std::pow( symbolDiagonal, 2 ) / 2.0 );
    symbolPositions.append( centerPoint );
    labelShifts.append( QPointF( side * mLabelDistanceFactor, -side * mLabelDistanceFactor ) );
    return;
  }

  const double circleAdditionPainterUnits = symbolContext.renderContext().convertToPainterUnits( mCircleRadiusAddition, Qgis::RenderUnit::Millimeters );

  switch ( mPlacement )
  {
    case Ring:
    {
      const double minDiameterToFitSymbols = nPosition * symbolDiagonal / ( 2.0 * M_PI );
      const double radius = std::max( symbolDiagonal / 2, minDiameterToFitSymbols ) + circleAdditionPainterUnits;

      const double angleStep = 2 * M_PI / nPosition;
      double currentAngle = 0.0;
      for ( int featureIndex = 0; featureIndex < nPosition; currentAngle += angleStep, featureIndex++ )
      {
        const double sinusCurrentAngle = std::sin( currentAngle );
        const double cosinusCurrentAngle = std::cos( currentAngle );
        const QPointF positionShift( radius * sinusCurrentAngle, radius * cosinusCurrentAngle );

        const QPointF labelShift( ( radius + diagonals.at( featureIndex ) * mLabelDistanceFactor ) * sinusCurrentAngle, ( radius + diagonals.at( featureIndex ) * mLabelDistanceFactor ) * cosinusCurrentAngle );
        symbolPositions.append( centerPoint + positionShift );
        labelShifts.append( labelShift );
      }
      circleRadius = radius;
      break;
    }
    case ConcentricRings:
    {
      const double centerDiagonal = mCenterSymbol->size( symbolContext.renderContext() ) * M_SQRT2;

      int pointsRemaining = nPosition;
      int ringNumber = 1;
      const double firstRingRadius = centerDiagonal / 2.0 + symbolDiagonal / 2.0;
      int featureIndex = 0;
      while ( pointsRemaining > 0 )
      {
        const double radiusCurrentRing = std::max( firstRingRadius + ( ringNumber - 1 ) * symbolDiagonal + ringNumber * circleAdditionPainterUnits, 0.0 );
        const int maxPointsCurrentRing = std::max( std::floor( 2 * M_PI * radiusCurrentRing / symbolDiagonal ), 1.0 );
        const int actualPointsCurrentRing = std::min( maxPointsCurrentRing, pointsRemaining );

        const double angleStep = 2 * M_PI / actualPointsCurrentRing;
        double currentAngle = 0.0;
        for ( int i = 0; i < actualPointsCurrentRing; ++i )
        {
          const double sinusCurrentAngle = std::sin( currentAngle );
          const double cosinusCurrentAngle = std::cos( currentAngle );
          const QPointF positionShift( radiusCurrentRing * sinusCurrentAngle, radiusCurrentRing * cosinusCurrentAngle );
          const QPointF labelShift( ( radiusCurrentRing + diagonals.at( featureIndex ) * mLabelDistanceFactor ) * sinusCurrentAngle, ( radiusCurrentRing + diagonals.at( featureIndex ) * mLabelDistanceFactor ) * cosinusCurrentAngle );
          symbolPositions.append( centerPoint + positionShift );
          labelShifts.append( labelShift );
          currentAngle += angleStep;
          featureIndex++;
        }

        pointsRemaining -= actualPointsCurrentRing;
        ringNumber++;
        circleRadius = radiusCurrentRing;
      }
      break;
    }
    case Grid:
    {
      const double centerDiagonal = mCenterSymbol->size( symbolContext.renderContext() ) * M_SQRT2;
      int pointsRemaining = nPosition;
      gridSize = std::ceil( std::sqrt( pointsRemaining ) );
      if ( pointsRemaining - std::pow( gridSize - 1, 2 ) < gridSize )
        gridSize -= 1;
      const double originalPointRadius = ( ( centerDiagonal / 2.0 + symbolDiagonal / 2.0 ) + symbolDiagonal ) / 2;
      const double userPointRadius =  originalPointRadius + circleAdditionPainterUnits;

      int yIndex = 0;
      while ( pointsRemaining > 0 )
      {
        for ( int xIndex = 0; xIndex < gridSize && pointsRemaining > 0; ++xIndex )
        {
          const QPointF positionShift( userPointRadius * xIndex, userPointRadius * yIndex );
          symbolPositions.append( centerPoint + positionShift );
          pointsRemaining--;
        }
        yIndex++;
      }

      centralizeGrid( symbolPositions, userPointRadius, gridSize );

      int xFactor;
      int yFactor;
      double side = 0;
      for ( int symbolIndex = 0; symbolIndex < symbolPositions.size(); ++symbolIndex )
      {
        if ( symbolPositions.at( symbolIndex ).x() < centerPoint.x() )
        {
          xFactor = -1;
        }
        else
        {
          xFactor = 1;
        }

        if ( symbolPositions.at( symbolIndex ).y() < centerPoint.y() )
        {
          yFactor = 1;
        }
        else
        {
          yFactor = -1;
        }

        side = std::sqrt( std::pow( diagonals.at( symbolIndex ), 2 ) / 2.0 );
        const QPointF labelShift( ( side * mLabelDistanceFactor * xFactor ), ( -side * mLabelDistanceFactor * yFactor ) );
        labelShifts.append( symbolPositions.at( symbolIndex ) - centerPoint + labelShift );
      }

      gridRadius = userPointRadius;
      break;
    }
  }
}

void QgsPointDisplacementRenderer::centralizeGrid( QList<QPointF> &pointSymbolPositions, double radius, int size ) const
{
  const double shiftAmount = -radius * ( size - 1.0 ) / 2.0;
  const QPointF centralShift( shiftAmount, shiftAmount );
  for ( int i = 0; i < pointSymbolPositions.size(); ++i )
  {
    pointSymbolPositions[i] += centralShift;
  }
}

void QgsPointDisplacementRenderer::drawGrid( int gridSizeUnits, QgsSymbolRenderContext &context,
    QList<QPointF> pointSymbolPositions, int nSymbols ) const
{
  QPainter *p = context.renderContext().painter();
  if ( nSymbols < 2 || !p ) //draw grid only if multiple features
  {
    return;
  }

  QPen gridPen( mCircleColor );
  gridPen.setWidthF( context.renderContext().convertToPainterUnits( mCircleWidth, Qgis::RenderUnit::Millimeters ) );
  p->setPen( gridPen );

  for ( int i = 0; i < pointSymbolPositions.size(); ++i )
  {
    if ( i + 1 < pointSymbolPositions.size() && 0 != ( i + 1 ) % gridSizeUnits )
    {
      const QLineF gridLineRow( pointSymbolPositions[i], pointSymbolPositions[i + 1] );
      p->drawLine( gridLineRow );
    }

    if ( i + gridSizeUnits < pointSymbolPositions.size() )
    {
      const QLineF gridLineColumn( pointSymbolPositions[i], pointSymbolPositions[i + gridSizeUnits] );
      p->drawLine( gridLineColumn );
    }
  }
}

void QgsPointDisplacementRenderer::drawCircle( double radiusPainterUnits, QgsSymbolRenderContext &context, QPointF centerPoint, int nSymbols ) const
{
  QPainter *p = context.renderContext().painter();
  if ( nSymbols < 2 || !p ) //draw circle only if multiple features
  {
    return;
  }

  //draw Circle
  QPen circlePen( mCircleColor );
  circlePen.setWidthF( context.renderContext().convertToPainterUnits( mCircleWidth, Qgis::RenderUnit::Millimeters ) );
  p->setPen( circlePen );
  p->drawArc( QRectF( centerPoint.x() - radiusPainterUnits, centerPoint.y() - radiusPainterUnits, 2 * radiusPainterUnits, 2 * radiusPainterUnits ), 0, 5760 );
}

void QgsPointDisplacementRenderer::drawSymbols( const ClusteredGroup &group, QgsRenderContext &context, const QList<QPointF> &symbolPositions ) const
{
  QList<QPointF>::const_iterator symbolPosIt = symbolPositions.constBegin();
  ClusteredGroup::const_iterator groupIt = group.constBegin();
  for ( ; symbolPosIt != symbolPositions.constEnd() && groupIt != group.constEnd();
        ++symbolPosIt, ++groupIt )
  {
    context.expressionContext().setFeature( groupIt->feature );
    groupIt->symbol()->startRender( context );
    groupIt->symbol()->renderPoint( *symbolPosIt, &( groupIt->feature ), context, -1, groupIt->isSelected );
    if ( context.hasRenderedFeatureHandlers() )
    {
      const QgsGeometry bounds( QgsGeometry::fromRect( QgsRectangle( groupIt->symbol()->bounds( *symbolPosIt, context, groupIt->feature ) ) ) );
      const QList< QgsRenderedFeatureHandlerInterface * > handlers = context.renderedFeatureHandlers();
      const QgsRenderedFeatureHandlerInterface::RenderedFeatureContext featureContext( context );
      for ( QgsRenderedFeatureHandlerInterface *handler : handlers )
        handler->handleRenderedFeature( groupIt->feature, bounds, featureContext );
    }
    groupIt->symbol()->stopRender( context );
  }
}

QgsPointDisplacementRenderer *QgsPointDisplacementRenderer::convertFromRenderer( const QgsFeatureRenderer *renderer )
{
  if ( renderer->type() == QLatin1String( "pointDisplacement" ) )
  {
    return dynamic_cast<QgsPointDisplacementRenderer *>( renderer->clone() );
  }
  else if ( renderer->type() == QLatin1String( "singleSymbol" ) ||
            renderer->type() == QLatin1String( "categorizedSymbol" ) ||
            renderer->type() == QLatin1String( "graduatedSymbol" ) ||
            renderer->type() == QLatin1String( "RuleRenderer" ) )
  {
    QgsPointDisplacementRenderer *pointRenderer = new QgsPointDisplacementRenderer();
    pointRenderer->setEmbeddedRenderer( renderer->clone() );
    renderer->copyRendererData( pointRenderer );
    return pointRenderer;
  }
  else if ( renderer->type() == QLatin1String( "pointCluster" ) )
  {
    QgsPointDisplacementRenderer *pointRenderer = new QgsPointDisplacementRenderer();
    const QgsPointClusterRenderer *clusterRenderer = static_cast< const QgsPointClusterRenderer * >( renderer );
    if ( clusterRenderer->embeddedRenderer() )
      pointRenderer->setEmbeddedRenderer( clusterRenderer->embeddedRenderer()->clone() );
    pointRenderer->setTolerance( clusterRenderer->tolerance() );
    pointRenderer->setToleranceUnit( clusterRenderer->toleranceUnit() );
    pointRenderer->setToleranceMapUnitScale( clusterRenderer->toleranceMapUnitScale() );
    renderer->copyRendererData( pointRenderer );
    return pointRenderer;
  }
  else
  {
    return nullptr;
  }
}
