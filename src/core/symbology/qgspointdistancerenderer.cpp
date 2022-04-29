/***************************************************************************
         qgspointdistancerenderer.cpp
         ----------------------------
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

#include "qgspointdistancerenderer.h"
#include "qgsgeometry.h"
#include "qgssymbollayerutils.h"
#include "qgsspatialindex.h"
#include "qgsmultipoint.h"
#include "qgslogger.h"
#include "qgsstyleentityvisitor.h"
#include "qgsexpressioncontextutils.h"
#include "qgsmarkersymbol.h"

#include <QDomElement>
#include <QPainter>

#include <cmath>

QgsPointDistanceRenderer::QgsPointDistanceRenderer( const QString &rendererName, const QString &labelAttributeName )
  : QgsFeatureRenderer( rendererName )
  , mLabelAttributeName( labelAttributeName )
  , mLabelIndex( -1 )
  , mTolerance( 3 )
  , mToleranceUnit( QgsUnitTypes::RenderMillimeters )
  , mDrawLabels( true )

{
  mRenderer.reset( QgsFeatureRenderer::defaultRenderer( QgsWkbTypes::PointGeometry ) );
}

void QgsPointDistanceRenderer::toSld( QDomDocument &doc, QDomElement &element, const QVariantMap &props ) const
{
  mRenderer->toSld( doc, element, props );
}


bool QgsPointDistanceRenderer::renderFeature( const QgsFeature &feature, QgsRenderContext &context, int layer, bool selected, bool drawVertexMarker )
{
  Q_UNUSED( drawVertexMarker )
  Q_UNUSED( context )
  Q_UNUSED( layer )

  /*
   * IMPORTANT: This algorithm is ported to Python in the processing "Points Displacement" algorithm.
   * Please port any changes/improvements to that algorithm too!
   */

  //check if there is already a point at that position
  if ( !feature.hasGeometry() )
    return false;

  QgsMarkerSymbol *symbol = firstSymbolForFeature( feature, context );

  //if the feature has no symbol (e.g., no matching rule in a rule-based renderer), skip it
  if ( !symbol )
    return false;

  //point position in screen coords
  QgsGeometry geom = feature.geometry();
  const QgsWkbTypes::Type geomType = geom.wkbType();
  if ( QgsWkbTypes::flatType( geomType ) != QgsWkbTypes::Point )
  {
    //can only render point type
    return false;
  }

  QString label;
  if ( mDrawLabels )
  {
    label = getLabel( feature );
  }

  const QgsCoordinateTransform xform = context.coordinateTransform();
  QgsFeature transformedFeature = feature;
  if ( xform.isValid() )
  {
    geom.transform( xform );
    transformedFeature.setGeometry( geom );
  }

  const double searchDistance = context.convertToMapUnits( mTolerance, mToleranceUnit, mToleranceMapUnitScale );
  const QgsPointXY point = transformedFeature.geometry().asPoint();
  const QList<QgsFeatureId> intersectList = mSpatialIndex->intersects( searchRect( point, searchDistance ) );
  if ( intersectList.empty() )
  {
    mSpatialIndex->addFeature( transformedFeature );
    // create new group
    ClusteredGroup newGroup;
    newGroup << GroupedFeature( transformedFeature, symbol->clone(), selected, label );
    mClusteredGroups.push_back( newGroup );
    // add to group index
    mGroupIndex.insert( transformedFeature.id(), mClusteredGroups.count() - 1 );
    mGroupLocations.insert( transformedFeature.id(), point );
  }
  else
  {
    // find group with closest location to this point (may be more than one within search tolerance)
    QgsFeatureId minDistFeatureId = intersectList.at( 0 );
    double minDist = mGroupLocations.value( minDistFeatureId ).distance( point );
    for ( int i = 1; i < intersectList.count(); ++i )
    {
      const QgsFeatureId candidateId = intersectList.at( i );
      const double newDist = mGroupLocations.value( candidateId ).distance( point );
      if ( newDist < minDist )
      {
        minDist = newDist;
        minDistFeatureId = candidateId;
      }
    }

    const int groupIdx = mGroupIndex[ minDistFeatureId ];
    ClusteredGroup &group = mClusteredGroups[groupIdx];

    // calculate new centroid of group
    const QgsPointXY oldCenter = mGroupLocations.value( minDistFeatureId );
    mGroupLocations[ minDistFeatureId ] = QgsPointXY( ( oldCenter.x() * group.size() + point.x() ) / ( group.size() + 1.0 ),
                                          ( oldCenter.y() * group.size() + point.y() ) / ( group.size() + 1.0 ) );

    // add to a group
    group << GroupedFeature( transformedFeature, symbol->clone(), selected, label );
    // add to group index
    mGroupIndex.insert( transformedFeature.id(), groupIdx );
  }

  return true;
}

void QgsPointDistanceRenderer::drawGroup( const ClusteredGroup &group, QgsRenderContext &context ) const
{
  //calculate centroid of all points, this will be center of group
  QgsMultiPoint *groupMultiPoint = new QgsMultiPoint();
  const auto constGroup = group;
  for ( const GroupedFeature &f : constGroup )
  {
    groupMultiPoint->addGeometry( f.feature.geometry().constGet()->clone() );
  }
  const QgsGeometry groupGeom( groupMultiPoint );
  const QgsGeometry centroid = groupGeom.centroid();
  QPointF pt = centroid.asQPointF();
  context.mapToPixel().transformInPlace( pt.rx(), pt.ry() );

  const QgsExpressionContextScopePopper scopePopper( context.expressionContext(), createGroupScope( group ) );
  drawGroup( pt, context, group );
}

void QgsPointDistanceRenderer::setEmbeddedRenderer( QgsFeatureRenderer *r )
{
  mRenderer.reset( r );
}

const QgsFeatureRenderer *QgsPointDistanceRenderer::embeddedRenderer() const
{
  return mRenderer.get();
}

void QgsPointDistanceRenderer::setLegendSymbolItem( const QString &key, QgsSymbol *symbol )
{
  if ( !mRenderer )
    return;

  mRenderer->setLegendSymbolItem( key, symbol );
}

bool QgsPointDistanceRenderer::legendSymbolItemsCheckable() const
{
  if ( !mRenderer )
    return false;

  return mRenderer->legendSymbolItemsCheckable();
}

bool QgsPointDistanceRenderer::legendSymbolItemChecked( const QString &key )
{
  if ( !mRenderer )
    return false;

  return mRenderer->legendSymbolItemChecked( key );
}

void QgsPointDistanceRenderer::checkLegendSymbolItem( const QString &key, bool state )
{
  if ( !mRenderer )
    return;

  mRenderer->checkLegendSymbolItem( key, state );
}

QString QgsPointDistanceRenderer::filter( const QgsFields &fields )
{
  if ( !mRenderer )
    return QgsFeatureRenderer::filter( fields );
  else
    return mRenderer->filter( fields );
}

bool QgsPointDistanceRenderer::accept( QgsStyleEntityVisitorInterface *visitor ) const
{
  if ( mRenderer )
    if ( !mRenderer->accept( visitor ) )
      return false;

  return true;
}

QSet<QString> QgsPointDistanceRenderer::usedAttributes( const QgsRenderContext &context ) const
{
  QSet<QString> attributeList;
  if ( !mLabelAttributeName.isEmpty() )
  {
    attributeList.insert( mLabelAttributeName );
  }
  if ( mRenderer )
  {
    attributeList += mRenderer->usedAttributes( context );
  }
  return attributeList;
}

bool QgsPointDistanceRenderer::filterNeedsGeometry() const
{
  return mRenderer ? mRenderer->filterNeedsGeometry() : false;
}

QgsFeatureRenderer::Capabilities QgsPointDistanceRenderer::capabilities()
{
  if ( !mRenderer )
  {
    return Capabilities();
  }
  return mRenderer->capabilities();
}

QgsSymbolList QgsPointDistanceRenderer::symbols( QgsRenderContext &context ) const
{
  if ( !mRenderer )
  {
    return QgsSymbolList();
  }
  return mRenderer->symbols( context );
}

QgsSymbol *QgsPointDistanceRenderer::symbolForFeature( const QgsFeature &feature, QgsRenderContext &context ) const
{
  if ( !mRenderer )
  {
    return nullptr;
  }
  return mRenderer->symbolForFeature( feature, context );
}

QgsSymbol *QgsPointDistanceRenderer::originalSymbolForFeature( const QgsFeature &feature, QgsRenderContext &context ) const
{
  if ( !mRenderer )
    return nullptr;
  return mRenderer->originalSymbolForFeature( feature, context );
}

QgsSymbolList QgsPointDistanceRenderer::symbolsForFeature( const QgsFeature &feature, QgsRenderContext &context ) const
{
  if ( !mRenderer )
  {
    return QgsSymbolList();
  }
  return mRenderer->symbolsForFeature( feature, context );
}

QgsSymbolList QgsPointDistanceRenderer::originalSymbolsForFeature( const QgsFeature &feature, QgsRenderContext &context ) const
{
  if ( !mRenderer )
    return QgsSymbolList();
  return mRenderer->originalSymbolsForFeature( feature, context );
}

QSet< QString > QgsPointDistanceRenderer::legendKeysForFeature( const QgsFeature &feature, QgsRenderContext &context ) const
{
  if ( !mRenderer )
    return QSet< QString >() << QString();
  return mRenderer->legendKeysForFeature( feature, context );
}

QString QgsPointDistanceRenderer::legendKeyToExpression( const QString &key, QgsVectorLayer *layer, bool &ok ) const
{
  ok = false;
  if ( !mRenderer )
    return QString();
  return mRenderer->legendKeyToExpression( key, layer, ok );
}

bool QgsPointDistanceRenderer::willRenderFeature( const QgsFeature &feature, QgsRenderContext &context ) const
{
  if ( !mRenderer )
  {
    return false;
  }
  return mRenderer->willRenderFeature( feature, context );
}


void QgsPointDistanceRenderer::startRender( QgsRenderContext &context, const QgsFields &fields )
{
  QgsFeatureRenderer::startRender( context, fields );

  mRenderer->startRender( context, fields );

  mClusteredGroups.clear();
  mGroupIndex.clear();
  mGroupLocations.clear();
  mSpatialIndex = new QgsSpatialIndex;

  if ( mLabelAttributeName.isEmpty() )
  {
    mLabelIndex = -1;
  }
  else
  {
    mLabelIndex = fields.lookupField( mLabelAttributeName );
  }

  if ( mMinLabelScale <= 0 || context.rendererScale() < mMinLabelScale )
  {
    mDrawLabels = true;
  }
  else
  {
    mDrawLabels = false;
  }
}

void QgsPointDistanceRenderer::stopRender( QgsRenderContext &context )
{
  QgsFeatureRenderer::stopRender( context );

  //printInfoDisplacementGroups(); //just for debugging

  if ( !context.renderingStopped() )
  {
    const auto constMClusteredGroups = mClusteredGroups;
    for ( const ClusteredGroup &group : constMClusteredGroups )
    {
      drawGroup( group, context );
    }
  }

  mClusteredGroups.clear();
  mGroupIndex.clear();
  mGroupLocations.clear();
  delete mSpatialIndex;
  mSpatialIndex = nullptr;

  mRenderer->stopRender( context );
}

QgsLegendSymbolList QgsPointDistanceRenderer::legendSymbolItems() const
{
  if ( mRenderer )
  {
    return mRenderer->legendSymbolItems();
  }
  return QgsLegendSymbolList();
}

QgsRectangle QgsPointDistanceRenderer::searchRect( const QgsPointXY &p, double distance ) const
{
  return QgsRectangle( p.x() - distance, p.y() - distance, p.x() + distance, p.y() + distance );
}

void QgsPointDistanceRenderer::printGroupInfo() const
{
#ifdef QGISDEBUG
  const int nGroups = mClusteredGroups.size();
  QgsDebugMsgLevel( "number of displacement groups:" + QString::number( nGroups ), 3 );
  for ( int i = 0; i < nGroups; ++i )
  {
    QgsDebugMsgLevel( "***************displacement group " + QString::number( i ), 3 );
    const auto constAt = mClusteredGroups.at( i );
    for ( const GroupedFeature &feature : constAt )
    {
      QgsDebugMsgLevel( FID_TO_STRING( feature.feature.id() ), 3 );
    }
  }
#endif
}

QString QgsPointDistanceRenderer::getLabel( const QgsFeature &feature ) const
{
  QString attribute;
  const QgsAttributes attrs = feature.attributes();
  if ( mLabelIndex >= 0 && mLabelIndex < attrs.count() )
  {
    attribute = attrs.at( mLabelIndex ).toString();
  }
  return attribute;
}

void QgsPointDistanceRenderer::drawLabels( QPointF centerPoint, QgsSymbolRenderContext &context, const QList<QPointF> &labelShifts, const ClusteredGroup &group ) const
{
  QPainter *p = context.renderContext().painter();
  if ( !p )
  {
    return;
  }

  const QPen labelPen( mLabelColor );
  p->setPen( labelPen );

  //scale font (for printing)
  QFont pixelSizeFont = mLabelFont;

  const double fontSizeInPixels = context.renderContext().convertToPainterUnits( mLabelFont.pointSizeF(), QgsUnitTypes::RenderPoints );
  pixelSizeFont.setPixelSize( static_cast< int >( std::round( fontSizeInPixels ) ) );
  QFont scaledFont = pixelSizeFont;
  scaledFont.setPixelSize( pixelSizeFont.pixelSize() );
  p->setFont( scaledFont );

  const QFontMetricsF fontMetrics( pixelSizeFont );
  QPointF currentLabelShift; //considers the signs to determine the label position

  QList<QPointF>::const_iterator labelPosIt = labelShifts.constBegin();
  ClusteredGroup::const_iterator groupIt = group.constBegin();

  for ( ; labelPosIt != labelShifts.constEnd() && groupIt != group.constEnd(); ++labelPosIt, ++groupIt )
  {
    currentLabelShift = *labelPosIt;
    if ( currentLabelShift.x() < 0 )
    {
      currentLabelShift.setX( currentLabelShift.x() - fontMetrics.horizontalAdvance( groupIt->label ) );
    }
    if ( currentLabelShift.y() > 0 )
    {
      currentLabelShift.setY( currentLabelShift.y() + fontMetrics.ascent() );
    }

    const QPointF drawingPoint( centerPoint + currentLabelShift );
    const QgsScopedQPainterState painterState( p );
    p->translate( drawingPoint.x(), drawingPoint.y() );
    p->drawText( QPointF( 0, 0 ), groupIt->label );
  }
}

QgsExpressionContextScope *QgsPointDistanceRenderer::createGroupScope( const ClusteredGroup &group ) const
{
  QgsExpressionContextScope *clusterScope = new QgsExpressionContextScope();
  if ( group.size() > 1 )
  {
    //scan through symbols to check color, e.g., if all clustered symbols are same color
    QColor groupColor;
    ClusteredGroup::const_iterator groupIt = group.constBegin();
    for ( ; groupIt != group.constEnd(); ++groupIt )
    {
      if ( !groupIt->symbol() )
        continue;

      if ( !groupColor.isValid() )
      {
        groupColor = groupIt->symbol()->color();
      }
      else
      {
        if ( groupColor != groupIt->symbol()->color() )
        {
          groupColor = QColor();
          break;
        }
      }
    }

    if ( groupColor.isValid() )
    {
      clusterScope->addVariable( QgsExpressionContextScope::StaticVariable( QgsExpressionContext::EXPR_CLUSTER_COLOR, QgsSymbolLayerUtils::encodeColor( groupColor ), true ) );
    }
    else
    {
      //mixed colors
      clusterScope->addVariable( QgsExpressionContextScope::StaticVariable( QgsExpressionContext::EXPR_CLUSTER_COLOR, QVariant(), true ) );
    }

    clusterScope->addVariable( QgsExpressionContextScope::StaticVariable( QgsExpressionContext::EXPR_CLUSTER_SIZE, group.size(), true ) );
  }
  if ( !group.empty() )
  {
    // data defined properties may require a feature in the expression context, so just use first feature in group
    clusterScope->setFeature( group.at( 0 ).feature );
  }
  return clusterScope;
}

QgsMarkerSymbol *QgsPointDistanceRenderer::firstSymbolForFeature( const QgsFeature &feature, QgsRenderContext &context )
{
  if ( !mRenderer )
  {
    return nullptr;
  }

  const QgsSymbolList symbolList = mRenderer->symbolsForFeature( feature, context );
  if ( symbolList.isEmpty() )
  {
    return nullptr;
  }

  return dynamic_cast< QgsMarkerSymbol * >( symbolList.at( 0 ) );
}

QgsPointDistanceRenderer::GroupedFeature::GroupedFeature( const QgsFeature &feature, QgsMarkerSymbol *symbol, bool isSelected, const QString &label )
  : feature( feature )
  , isSelected( isSelected )
  , label( label )
  , mSymbol( symbol )
{}

QgsPointDistanceRenderer::GroupedFeature::~GroupedFeature() = default;
