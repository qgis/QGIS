/***************************************************************************
    qgsrendererv2.cpp
    ---------------------
    begin                : November 2009
    copyright            : (C) 2009 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsrendererv2.h"
#include "qgssymbolv2.h"
#include "qgssymbollayerv2utils.h"
#include "qgsrulebasedrendererv2.h"
#include "qgsdatadefined.h"

#include "qgssinglesymbolrendererv2.h" // for default renderer

#include "qgsrendererv2registry.h"

#include "qgsrendercontext.h"
#include "qgsclipper.h"
#include "qgsgeometry.h"
#include "qgsgeometrycollectionv2.h"
#include "qgsfeature.h"
#include "qgslogger.h"
#include "qgsvectorlayer.h"
#include "qgspainteffect.h"
#include "qgseffectstack.h"
#include "qgspainteffectregistry.h"
#include "qgswkbptr.h"
#include "qgspointv2.h"

#include <QDomElement>
#include <QDomDocument>
#include <QPolygonF>



QgsConstWkbPtr QgsFeatureRendererV2::_getPoint( QPointF& pt, QgsRenderContext& context, QgsConstWkbPtr& wkbPtr )
{
  return QgsSymbolV2::_getPoint( pt, context, wkbPtr );
}

QgsConstWkbPtr QgsFeatureRendererV2::_getLineString( QPolygonF& pts, QgsRenderContext& context, QgsConstWkbPtr& wkbPtr, bool clipToExtent )
{
  return QgsSymbolV2::_getLineString( pts, context, wkbPtr, clipToExtent );
}

QgsConstWkbPtr QgsFeatureRendererV2::_getPolygon( QPolygonF& pts, QList<QPolygonF>& holes, QgsRenderContext& context, QgsConstWkbPtr& wkbPtr, bool clipToExtent )
{
  return QgsSymbolV2::_getPolygon( pts, holes, context, wkbPtr, clipToExtent );
}

void QgsFeatureRendererV2::setScaleMethodToSymbol( QgsSymbolV2* symbol, int scaleMethod )
{
  if ( symbol )
  {
    if ( symbol->type() == QgsSymbolV2::Marker )
    {
      QgsMarkerSymbolV2* ms = static_cast<QgsMarkerSymbolV2*>( symbol );
      if ( ms )
      {
        ms->setScaleMethod( static_cast< QgsSymbolV2::ScaleMethod >( scaleMethod ) );
      }
    }
  }
}

void QgsFeatureRendererV2::copyRendererData( QgsFeatureRendererV2* destRenderer ) const
{
  if ( !destRenderer || !mPaintEffect )
    return;

  destRenderer->setPaintEffect( mPaintEffect->clone() );
  destRenderer->mOrderBy = mOrderBy;
  destRenderer->mOrderByEnabled = mOrderByEnabled;
}

void QgsFeatureRendererV2::copyPaintEffect( QgsFeatureRendererV2 *destRenderer ) const
{
  if ( !destRenderer || !mPaintEffect )
    return;

  destRenderer->setPaintEffect( mPaintEffect->clone() );
}

QgsFeatureRendererV2::QgsFeatureRendererV2( const QString& type )
    : mType( type )
    , mUsingSymbolLevels( false )
    , mCurrentVertexMarkerType( QgsVectorLayer::Cross )
    , mCurrentVertexMarkerSize( 3 )
    , mPaintEffect( nullptr )
    , mForceRaster( false )
    , mOrderByEnabled( false )
{
  mPaintEffect = QgsPaintEffectRegistry::defaultStack();
  mPaintEffect->setEnabled( false );
}

QgsFeatureRendererV2::~QgsFeatureRendererV2()
{
  delete mPaintEffect;
}

QgsFeatureRendererV2* QgsFeatureRendererV2::defaultRenderer( QGis::GeometryType geomType )
{
  return new QgsSingleSymbolRendererV2( QgsSymbolV2::defaultSymbol( geomType ) );
}

QgsSymbolV2* QgsFeatureRendererV2::symbolForFeature( QgsFeature& feature )
{
  QgsRenderContext context;
  context.setExpressionContext( QgsExpressionContextUtils::createFeatureBasedContext( feature, QgsFields() ) );
  return symbolForFeature( feature, context );
}

QgsSymbolV2* QgsFeatureRendererV2::symbolForFeature( QgsFeature &feature, QgsRenderContext &context )
{
  Q_UNUSED( context );
  // base method calls deprecated symbolForFeature to maintain API
  Q_NOWARN_DEPRECATED_PUSH
  return symbolForFeature( feature );
  Q_NOWARN_DEPRECATED_POP
}

QgsSymbolV2 *QgsFeatureRendererV2::originalSymbolForFeature( QgsFeature &feature )
{
  Q_NOWARN_DEPRECATED_PUSH
  return symbolForFeature( feature );
  Q_NOWARN_DEPRECATED_POP
}

QgsSymbolV2 *QgsFeatureRendererV2::originalSymbolForFeature( QgsFeature &feature, QgsRenderContext &context )
{
  return symbolForFeature( feature, context );
}

QSet< QString > QgsFeatureRendererV2::legendKeysForFeature( QgsFeature& feature, QgsRenderContext& context )
{
  Q_UNUSED( feature );
  Q_UNUSED( context );
  return QSet< QString >();
}

void QgsFeatureRendererV2::startRender( QgsRenderContext& context, const QgsVectorLayer* vlayer )
{
  startRender( context, vlayer->fields() );
}

bool QgsFeatureRendererV2::filterNeedsGeometry() const
{
  return false;
}

bool QgsFeatureRendererV2::renderFeature( QgsFeature& feature, QgsRenderContext& context, int layer, bool selected, bool drawVertexMarker )
{
  QgsSymbolV2* symbol = symbolForFeature( feature, context );
  if ( !symbol )
    return false;

  renderFeatureWithSymbol( feature, symbol, context, layer, selected, drawVertexMarker );
  return true;
}

void QgsFeatureRendererV2::renderFeatureWithSymbol( QgsFeature& feature, QgsSymbolV2* symbol, QgsRenderContext& context, int layer, bool selected, bool drawVertexMarker )
{
  symbol->renderFeature( feature, context, layer, selected, drawVertexMarker, mCurrentVertexMarkerType, mCurrentVertexMarkerSize );
}

QString QgsFeatureRendererV2::dump() const
{
  return "UNKNOWN RENDERER\n";
}

QgsSymbolV2List QgsFeatureRendererV2::symbols()
{
  QgsRenderContext context;
  return symbols( context );
}

QgsSymbolV2List QgsFeatureRendererV2::symbols( QgsRenderContext &context )
{
  Q_UNUSED( context );

  //base implementation calls deprecated method to maintain API
  Q_NOWARN_DEPRECATED_PUSH
  return symbols();
  Q_NOWARN_DEPRECATED_POP
}


QgsFeatureRendererV2* QgsFeatureRendererV2::load( QDomElement& element )
{
  // <renderer-v2 type=""> ... </renderer-v2>

  if ( element.isNull() )
    return nullptr;

  // load renderer
  QString rendererType = element.attribute( "type" );

  QgsRendererV2AbstractMetadata* m = QgsRendererV2Registry::instance()->rendererMetadata( rendererType );
  if ( !m )
    return nullptr;

  QgsFeatureRendererV2* r = m->createRenderer( element );
  if ( r )
  {
    r->setUsingSymbolLevels( element.attribute( "symbollevels", "0" ).toInt() );
    r->setForceRasterRender( element.attribute( "forceraster", "0" ).toInt() );

    //restore layer effect
    QDomElement effectElem = element.firstChildElement( "effect" );
    if ( !effectElem.isNull() )
    {
      r->setPaintEffect( QgsPaintEffectRegistry::instance()->createEffect( effectElem ) );
    }

    // restore order by
    QDomElement orderByElem = element.firstChildElement( "orderby" );
    r->mOrderBy.load( orderByElem );
    r->setOrderByEnabled( element.attribute( "enableorderby", "0" ).toInt() );
  }
  return r;
}

QDomElement QgsFeatureRendererV2::save( QDomDocument& doc )
{
  // create empty renderer element
  QDomElement rendererElem = doc.createElement( RENDERER_TAG_NAME );
  rendererElem.setAttribute( "forceraster", ( mForceRaster ? "1" : "0" ) );

  if ( mPaintEffect && !QgsPaintEffectRegistry::isDefaultStack( mPaintEffect ) )
    mPaintEffect->saveProperties( doc, rendererElem );

  if ( !mOrderBy.isEmpty() )
  {
    QDomElement orderBy = doc.createElement( "orderby" );
    mOrderBy.save( orderBy );
    rendererElem.appendChild( orderBy );
  }
  rendererElem.setAttribute( "enableorderby", ( mOrderByEnabled ? "1" : "0" ) );
  return rendererElem;
}

QgsFeatureRendererV2* QgsFeatureRendererV2::loadSld( const QDomNode &node, QGis::GeometryType geomType, QString &errorMessage )
{
  QDomElement element = node.toElement();
  if ( element.isNull() )
    return nullptr;

  // get the UserStyle element
  QDomElement userStyleElem = element.firstChildElement( "UserStyle" );
  if ( userStyleElem.isNull() )
  {
    // UserStyle element not found, nothing will be rendered
    errorMessage = "Info: UserStyle element not found.";
    return nullptr;
  }

  // get the FeatureTypeStyle element
  QDomElement featTypeStyleElem = userStyleElem.firstChildElement( "FeatureTypeStyle" );
  if ( featTypeStyleElem.isNull() )
  {
    errorMessage = "Info: FeatureTypeStyle element not found.";
    return nullptr;
  }

  // use the RuleRenderer when more rules are present or the rule
  // has filters or min/max scale denominators set,
  // otherwise use the SingleSymbol renderer
  bool needRuleRenderer = false;
  int ruleCount = 0;

  QDomElement ruleElem = featTypeStyleElem.firstChildElement( "Rule" );
  while ( !ruleElem.isNull() )
  {
    ruleCount++;

    // more rules present, use the RuleRenderer
    if ( ruleCount > 1 )
    {
      QgsDebugMsg( "more Rule elements found: need a RuleRenderer" );
      needRuleRenderer = true;
      break;
    }

    QDomElement ruleChildElem = ruleElem.firstChildElement();
    while ( !ruleChildElem.isNull() )
    {
      // rule has filter or min/max scale denominator, use the RuleRenderer
      if ( ruleChildElem.localName() == "Filter" ||
           ruleChildElem.localName() == "MinScaleDenominator" ||
           ruleChildElem.localName() == "MaxScaleDenominator" )
      {
        QgsDebugMsg( "Filter or Min/MaxScaleDenominator element found: need a RuleRenderer" );
        needRuleRenderer = true;
        break;
      }

      ruleChildElem = ruleChildElem.nextSiblingElement();
    }

    if ( needRuleRenderer )
    {
      break;
    }

    ruleElem = ruleElem.nextSiblingElement( "Rule" );
  }

  QString rendererType;
  if ( needRuleRenderer )
  {
    rendererType = "RuleRenderer";
  }
  else
  {
    rendererType = "singleSymbol";
  }
  QgsDebugMsg( QString( "Instantiating a '%1' renderer..." ).arg( rendererType ) );

  // create the renderer and return it
  QgsRendererV2AbstractMetadata* m = QgsRendererV2Registry::instance()->rendererMetadata( rendererType );
  if ( !m )
  {
    errorMessage = QString( "Error: Unable to get metadata for '%1' renderer." ).arg( rendererType );
    return nullptr;
  }

  QgsFeatureRendererV2* r = m->createRendererFromSld( featTypeStyleElem, geomType );
  return r;
}

QDomElement QgsFeatureRendererV2::writeSld( QDomDocument& doc, const QgsVectorLayer &layer ) const
{
  return writeSld( doc, layer.name() );
}

QDomElement QgsFeatureRendererV2::writeSld( QDomDocument& doc, const QString& styleName ) const
{
  QDomElement userStyleElem = doc.createElement( "UserStyle" );

  QDomElement nameElem = doc.createElement( "se:Name" );
  nameElem.appendChild( doc.createTextNode( styleName ) );
  userStyleElem.appendChild( nameElem );

  QDomElement featureTypeStyleElem = doc.createElement( "se:FeatureTypeStyle" );
  toSld( doc, featureTypeStyleElem );
  userStyleElem.appendChild( featureTypeStyleElem );

  return userStyleElem;
}

QgsLegendSymbologyList QgsFeatureRendererV2::legendSymbologyItems( QSize iconSize )
{
  Q_UNUSED( iconSize );
  // empty list by default
  return QgsLegendSymbologyList();
}

bool QgsFeatureRendererV2::legendSymbolItemsCheckable() const
{
  return false;
}

bool QgsFeatureRendererV2::legendSymbolItemChecked( const QString& key )
{
  Q_UNUSED( key );
  return false;
}

void QgsFeatureRendererV2::checkLegendSymbolItem( const QString& key, bool state )
{
  Q_UNUSED( key );
  Q_UNUSED( state );
}

void QgsFeatureRendererV2::setLegendSymbolItem( const QString& key, QgsSymbolV2* symbol )
{
  Q_UNUSED( key );
  delete symbol;
}

QgsLegendSymbolList QgsFeatureRendererV2::legendSymbolItems( double scaleDenominator, const QString& rule )
{
  Q_UNUSED( scaleDenominator );
  Q_UNUSED( rule );
  return QgsLegendSymbolList();
}

QgsLegendSymbolListV2 QgsFeatureRendererV2::legendSymbolItemsV2() const
{
  QgsLegendSymbolList lst = const_cast<QgsFeatureRendererV2*>( this )->legendSymbolItems();
  QgsLegendSymbolListV2 lst2;
  int i = 0;
  for ( QgsLegendSymbolList::const_iterator it = lst.begin(); it != lst.end(); ++it, ++i )
  {
    lst2 << QgsLegendSymbolItemV2( it->second, it->first, QString::number( i ), legendSymbolItemsCheckable() );
  }
  return lst2;
}

void QgsFeatureRendererV2::setVertexMarkerAppearance( int type, int size )
{
  mCurrentVertexMarkerType = type;
  mCurrentVertexMarkerSize = size;
}

bool QgsFeatureRendererV2::willRenderFeature( QgsFeature &feat )
{
  Q_NOWARN_DEPRECATED_PUSH
  return nullptr != symbolForFeature( feat );
  Q_NOWARN_DEPRECATED_POP
}

bool QgsFeatureRendererV2::willRenderFeature( QgsFeature &feat, QgsRenderContext &context )
{
  return nullptr != symbolForFeature( feat, context );
}

void QgsFeatureRendererV2::renderVertexMarker( QPointF pt, QgsRenderContext& context )
{
  QgsVectorLayer::drawVertexMarker( pt.x(), pt.y(), *context.painter(),
                                    static_cast< QgsVectorLayer::VertexMarkerType >( mCurrentVertexMarkerType ),
                                    mCurrentVertexMarkerSize );
}

void QgsFeatureRendererV2::renderVertexMarkerPolyline( QPolygonF& pts, QgsRenderContext& context )
{
  Q_FOREACH ( QPointF pt, pts )
    renderVertexMarker( pt, context );
}

void QgsFeatureRendererV2::renderVertexMarkerPolygon( QPolygonF& pts, QList<QPolygonF>* rings, QgsRenderContext& context )
{
  Q_FOREACH ( QPointF pt, pts )
    renderVertexMarker( pt, context );

  if ( rings )
  {
    Q_FOREACH ( const QPolygonF& ring, *rings )
    {
      Q_FOREACH ( QPointF pt, ring )
        renderVertexMarker( pt, context );
    }
  }
}

QgsSymbolV2List QgsFeatureRendererV2::symbolsForFeature( QgsFeature& feat )
{
  QgsSymbolV2List lst;
  Q_NOWARN_DEPRECATED_PUSH
  QgsSymbolV2* s = symbolForFeature( feat );
  Q_NOWARN_DEPRECATED_POP
  if ( s ) lst.append( s );
  return lst;
}

QgsSymbolV2List QgsFeatureRendererV2::symbolsForFeature( QgsFeature &feat, QgsRenderContext &context )
{
  QgsSymbolV2List lst;
  QgsSymbolV2* s = symbolForFeature( feat, context );
  if ( s ) lst.append( s );
  return lst;
}

QgsSymbolV2List QgsFeatureRendererV2::originalSymbolsForFeature( QgsFeature& feat )
{
  QgsSymbolV2List lst;
  Q_NOWARN_DEPRECATED_PUSH
  QgsSymbolV2* s = originalSymbolForFeature( feat );
  Q_NOWARN_DEPRECATED_POP
  if ( s ) lst.append( s );
  return lst;
}

QgsSymbolV2List QgsFeatureRendererV2::originalSymbolsForFeature( QgsFeature &feat, QgsRenderContext &context )
{
  QgsSymbolV2List lst;
  QgsSymbolV2* s = originalSymbolForFeature( feat, context );
  if ( s ) lst.append( s );
  return lst;
}

QgsPaintEffect *QgsFeatureRendererV2::paintEffect() const
{
  return mPaintEffect;
}

void QgsFeatureRendererV2::setPaintEffect( QgsPaintEffect *effect )
{
  delete mPaintEffect;
  mPaintEffect = effect;
}

QgsFeatureRequest::OrderBy QgsFeatureRendererV2::orderBy() const
{
  return mOrderBy;
}

void QgsFeatureRendererV2::setOrderBy( const QgsFeatureRequest::OrderBy& orderBy )
{
  mOrderBy = orderBy;
}

bool QgsFeatureRendererV2::orderByEnabled() const
{
  return mOrderByEnabled;
}

void QgsFeatureRendererV2::setOrderByEnabled( bool enabled )
{
  mOrderByEnabled = enabled;
}

void QgsFeatureRendererV2::convertSymbolSizeScale( QgsSymbolV2 * symbol, QgsSymbolV2::ScaleMethod method, const QString & field )
{
  if ( symbol->type() == QgsSymbolV2::Marker )
  {
    QgsMarkerSymbolV2 * s = static_cast<QgsMarkerSymbolV2 *>( symbol );
    if ( QgsSymbolV2::ScaleArea == QgsSymbolV2::ScaleMethod( method ) )
    {
      const QgsDataDefined dd( "coalesce(sqrt(" + QString::number( s->size() ) + " * (" + field + ")),0)" );
      s->setDataDefinedSize( dd );
    }
    else
    {
      const QgsDataDefined dd( "coalesce(" + QString::number( s->size() ) + " * (" + field + "),0)" );
      s->setDataDefinedSize( dd );
    }
    s->setScaleMethod( QgsSymbolV2::ScaleDiameter );
  }
  else if ( symbol->type() == QgsSymbolV2::Line )
  {
    QgsLineSymbolV2 * s = static_cast<QgsLineSymbolV2 *>( symbol );
    const QgsDataDefined dd( "coalesce(" + QString::number( s->width() ) + " * (" + field + "),0)" );
    s->setDataDefinedWidth( dd );
  }
}

void QgsFeatureRendererV2::convertSymbolRotation( QgsSymbolV2 * symbol, const QString & field )
{
  if ( symbol->type() == QgsSymbolV2::Marker )
  {
    QgsMarkerSymbolV2 * s = static_cast<QgsMarkerSymbolV2 *>( symbol );
    const QgsDataDefined dd(( s->angle()
                              ? QString::number( s->angle() ) + " + "
                              : QString() ) + field );
    s->setDataDefinedAngle( dd );
  }
}
