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

#include <cmath>
#include <memory>

#include "qgscolorutils.h"
#include "qgsfontutils.h"
#include "qgsmarkersymbol.h"
#include "qgspointclusterrenderer.h"
#include "qgsrenderedfeaturehandlerinterface.h"
#include "qgsstyleentityvisitor.h"
#include "qgssymbollayerutils.h"
#include "qgsunittypes.h"

#include <QPainter>

QgsPointDisplacementRenderer::QgsPointDisplacementRenderer( const QString &labelAttributeName )
  : QgsPointDistanceRenderer( u"pointDisplacement"_s, labelAttributeName )
  , mCircleColor( QColor( 125, 125, 125 ) )
{
  mCenterSymbol = std::make_unique<QgsMarkerSymbol>( );
}

Qgis::FeatureRendererFlags QgsPointDisplacementRenderer::flags() const
{
  Qgis::FeatureRendererFlags res;
  if ( mCenterSymbol && mCenterSymbol->flags().testFlag( Qgis::SymbolFlag::AffectsLabeling ) )
    res.setFlag( Qgis::FeatureRendererFlag::AffectsLabeling );
  if ( mRenderer && mRenderer->flags().testFlag( Qgis::FeatureRendererFlag::AffectsLabeling ) )
    res.setFlag( Qgis::FeatureRendererFlag::AffectsLabeling );
  return res;
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
  r->setLabelAttributeName( symbologyElem.attribute( u"labelAttributeName"_s ) );
  QFont labelFont;
  if ( !QgsFontUtils::setFromXmlChildNode( labelFont, symbologyElem, u"labelFontProperties"_s ) )
  {
    labelFont.fromString( symbologyElem.attribute( u"labelFont"_s, QString() ) );
  }
  r->setLabelFont( labelFont );
  r->setPlacement( static_cast< Placement >( symbologyElem.attribute( u"placement"_s, u"0"_s ).toInt() ) );
  r->setCircleWidth( symbologyElem.attribute( u"circleWidth"_s, u"0.4"_s ).toDouble() );
  r->setCircleColor( QgsColorUtils::colorFromString( symbologyElem.attribute( u"circleColor"_s, QString() ) ) );
  r->setLabelColor( QgsColorUtils::colorFromString( symbologyElem.attribute( u"labelColor"_s, QString() ) ) );
  r->setCircleRadiusAddition( symbologyElem.attribute( u"circleRadiusAddition"_s, u"0.0"_s ).toDouble() );
  r->setLabelDistanceFactor( symbologyElem.attribute( u"labelDistanceFactor"_s, u"0.5"_s ).toDouble() );
  r->setMinimumLabelScale( symbologyElem.attribute( u"maxLabelScaleDenominator"_s, u"-1"_s ).toDouble() );
  r->setTolerance( symbologyElem.attribute( u"tolerance"_s, u"0.00001"_s ).toDouble() );
  r->setToleranceUnit( QgsUnitTypes::decodeRenderUnit( symbologyElem.attribute( u"toleranceUnit"_s, u"MapUnit"_s ) ) );
  r->setToleranceMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( symbologyElem.attribute( u"toleranceUnitScale"_s ) ) );

  //look for an embedded renderer <renderer-v2>
  QDomElement embeddedRendererElem = symbologyElem.firstChildElement( u"renderer-v2"_s );
  if ( !embeddedRendererElem.isNull() )
  {
    r->setEmbeddedRenderer( QgsFeatureRenderer::load( embeddedRendererElem, context ) );
  }

  //center symbol
  const QDomElement centerSymbolElem = symbologyElem.firstChildElement( u"symbol"_s );
  if ( !centerSymbolElem.isNull() )
  {
    r->setCenterSymbol( QgsSymbolLayerUtils::loadSymbol<QgsMarkerSymbol>( centerSymbolElem, context ).release() );
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
  rendererElement.setAttribute( u"type"_s, u"pointDisplacement"_s );
  rendererElement.setAttribute( u"labelAttributeName"_s, mLabelAttributeName );
  rendererElement.appendChild( QgsFontUtils::toXmlElement( mLabelFont, doc, u"labelFontProperties"_s ) );
  rendererElement.setAttribute( u"circleWidth"_s, QString::number( mCircleWidth ) );
  rendererElement.setAttribute( u"circleColor"_s, QgsColorUtils::colorToString( mCircleColor ) );
  rendererElement.setAttribute( u"labelColor"_s, QgsColorUtils::colorToString( mLabelColor ) );
  rendererElement.setAttribute( u"circleRadiusAddition"_s, QString::number( mCircleRadiusAddition ) );
  rendererElement.setAttribute( u"labelDistanceFactor"_s, QString::number( mLabelDistanceFactor ) );
  rendererElement.setAttribute( u"placement"_s, static_cast< int >( mPlacement ) );
  rendererElement.setAttribute( u"maxLabelScaleDenominator"_s, QString::number( mMinLabelScale ) );
  rendererElement.setAttribute( u"tolerance"_s, QString::number( mTolerance ) );
  rendererElement.setAttribute( u"toleranceUnit"_s, QgsUnitTypes::encodeUnit( mToleranceUnit ) );
  rendererElement.setAttribute( u"toleranceUnitScale"_s, QgsSymbolLayerUtils::encodeMapUnitScale( mToleranceMapUnitScale ) );

  if ( mRenderer )
  {
    const QDomElement embeddedRendererElem = mRenderer->save( doc, context );
    rendererElement.appendChild( embeddedRendererElem );
  }
  if ( mCenterSymbol )
  {
    const QDomElement centerSymbolElem = QgsSymbolLayerUtils::saveSymbol( u"centerSymbol"_s, mCenterSymbol.get(), doc, context );
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
    if ( !visitor->visit( QgsStyleEntityVisitorInterface::StyleLeaf( &entity, u"center"_s, QObject::tr( "Center Symbol" ) ) ) )
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
  if ( renderer->type() == "pointDisplacement"_L1 )
  {
    return dynamic_cast<QgsPointDisplacementRenderer *>( renderer->clone() );
  }
  else if ( renderer->type() == "singleSymbol"_L1 ||
            renderer->type() == "categorizedSymbol"_L1 ||
            renderer->type() == "graduatedSymbol"_L1 ||
            renderer->type() == "RuleRenderer"_L1 )
  {
    QgsPointDisplacementRenderer *pointRenderer = new QgsPointDisplacementRenderer();
    pointRenderer->setEmbeddedRenderer( renderer->clone() );
    renderer->copyRendererData( pointRenderer );
    return pointRenderer;
  }
  else if ( renderer->type() == "pointCluster"_L1 )
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
