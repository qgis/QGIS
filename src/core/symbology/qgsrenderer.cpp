/***************************************************************************
    qgsrenderer.cpp
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

#include "qgsrenderer.h"
#include "qgssymbol.h"
#include "qgssymbollayerutils.h"
#include "qgsrulebasedrenderer.h"

#include "qgssinglesymbolrenderer.h" // for default renderer

#include "qgsrendererregistry.h"

#include "qgsrendercontext.h"
#include "qgsclipper.h"
#include "qgsgeometry.h"
#include "qgsgeometrycollection.h"
#include "qgsfeature.h"
#include "qgslogger.h"
#include "qgsvectorlayer.h"
#include "qgspainteffect.h"
#include "qgseffectstack.h"
#include "qgspainteffectregistry.h"
#include "qgswkbptr.h"
#include "qgspoint.h"
#include "qgsproperty.h"
#include "qgsapplication.h"
#include "qgsmarkersymbol.h"
#include "qgslinesymbol.h"

#include <QDomElement>
#include <QDomDocument>
#include <QPolygonF>
#include <QThread>

QPointF QgsFeatureRenderer::_getPoint( QgsRenderContext &context, const QgsPoint &point )
{
  return QgsSymbol::_getPoint( context, point );
}

void QgsFeatureRenderer::copyRendererData( QgsFeatureRenderer *destRenderer ) const
{
  if ( !destRenderer )
    return;

  if ( mPaintEffect )
    destRenderer->setPaintEffect( mPaintEffect->clone() );

  destRenderer->setForceRasterRender( mForceRaster );
  destRenderer->setUsingSymbolLevels( mUsingSymbolLevels );
  destRenderer->mOrderBy = mOrderBy;
  destRenderer->mOrderByEnabled = mOrderByEnabled;
  destRenderer->mReferenceScale = mReferenceScale;
}

QgsFeatureRenderer::QgsFeatureRenderer( const QString &type )
  : mType( type )
{
  mPaintEffect = QgsPaintEffectRegistry::defaultStack();
  mPaintEffect->setEnabled( false );
}

QgsFeatureRenderer::~QgsFeatureRenderer()
{
  delete mPaintEffect;
}

QgsFeatureRenderer *QgsFeatureRenderer::defaultRenderer( QgsWkbTypes::GeometryType geomType )
{
  return new QgsSingleSymbolRenderer( QgsSymbol::defaultSymbol( geomType ) );
}

QgsSymbol *QgsFeatureRenderer::originalSymbolForFeature( const QgsFeature &feature, QgsRenderContext &context ) const
{
  return symbolForFeature( feature, context );
}

QSet< QString > QgsFeatureRenderer::legendKeysForFeature( const QgsFeature &feature, QgsRenderContext &context ) const
{
  Q_UNUSED( feature )
  Q_UNUSED( context )
  return QSet< QString >();
}

void QgsFeatureRenderer::startRender( QgsRenderContext &, const QgsFields & )
{
#ifdef QGISDEBUG
  if ( !mThread )
  {
    mThread = QThread::currentThread();
  }
  else
  {
    Q_ASSERT_X( mThread == QThread::currentThread(), "QgsFeatureRenderer::startRender", "startRender called in a different thread - use a cloned renderer instead" );
  }
#endif
}

void QgsFeatureRenderer::stopRender( QgsRenderContext & )
{
#ifdef QGISDEBUG
  Q_ASSERT_X( mThread == QThread::currentThread(), "QgsFeatureRenderer::stopRender", "stopRender called in a different thread - use a cloned renderer instead" );
#endif
}

bool QgsFeatureRenderer::usesEmbeddedSymbols() const
{
  return false;
}

bool QgsFeatureRenderer::filterNeedsGeometry() const
{
  return false;
}

bool QgsFeatureRenderer::renderFeature( const QgsFeature &feature, QgsRenderContext &context, int layer, bool selected, bool drawVertexMarker )
{
#ifdef QGISDEBUG
  Q_ASSERT_X( mThread == QThread::currentThread(), "QgsFeatureRenderer::renderFeature", "renderFeature called in a different thread - use a cloned renderer instead" );
#endif

  QgsSymbol *symbol = symbolForFeature( feature, context );
  if ( !symbol )
    return false;

  renderFeatureWithSymbol( feature, symbol, context, layer, selected, drawVertexMarker );
  return true;
}

void QgsFeatureRenderer::renderFeatureWithSymbol( const QgsFeature &feature, QgsSymbol *symbol, QgsRenderContext &context, int layer, bool selected, bool drawVertexMarker )
{
  symbol->renderFeature( feature, context, layer, selected, drawVertexMarker, mCurrentVertexMarkerType, mCurrentVertexMarkerSize );
}

QString QgsFeatureRenderer::dump() const
{
  return QStringLiteral( "UNKNOWN RENDERER\n" );
}

QgsSymbolList QgsFeatureRenderer::symbols( QgsRenderContext &context ) const
{
  Q_UNUSED( context )
  return QgsSymbolList();
}

QgsFeatureRenderer *QgsFeatureRenderer::load( QDomElement &element, const QgsReadWriteContext &context )
{
  // <renderer-v2 type=""> ... </renderer-v2>

  if ( element.isNull() )
    return nullptr;

  // load renderer
  const QString rendererType = element.attribute( QStringLiteral( "type" ) );

  QgsRendererAbstractMetadata *m = QgsApplication::rendererRegistry()->rendererMetadata( rendererType );
  if ( !m )
    return nullptr;

  QgsFeatureRenderer *r = m->createRenderer( element, context );
  if ( r )
  {
    r->setUsingSymbolLevels( element.attribute( QStringLiteral( "symbollevels" ), QStringLiteral( "0" ) ).toInt() );
    r->setForceRasterRender( element.attribute( QStringLiteral( "forceraster" ), QStringLiteral( "0" ) ).toInt() );
    r->setReferenceScale( element.attribute( QStringLiteral( "referencescale" ), QStringLiteral( "-1" ) ).toDouble() );

    //restore layer effect
    const QDomElement effectElem = element.firstChildElement( QStringLiteral( "effect" ) );
    if ( !effectElem.isNull() )
    {
      r->setPaintEffect( QgsApplication::paintEffectRegistry()->createEffect( effectElem ) );
    }

    // restore order by
    const QDomElement orderByElem = element.firstChildElement( QStringLiteral( "orderby" ) );
    r->mOrderBy.load( orderByElem );
    r->setOrderByEnabled( element.attribute( QStringLiteral( "enableorderby" ), QStringLiteral( "0" ) ).toInt() );
  }
  return r;
}

QDomElement QgsFeatureRenderer::save( QDomDocument &doc, const QgsReadWriteContext &context )
{
  Q_UNUSED( context )
  // create empty renderer element
  QDomElement rendererElem = doc.createElement( RENDERER_TAG_NAME );

  saveRendererData( doc, rendererElem, context );

  return rendererElem;
}

void QgsFeatureRenderer::saveRendererData( QDomDocument &doc, QDomElement &rendererElem, const QgsReadWriteContext & )
{
  rendererElem.setAttribute( QStringLiteral( "forceraster" ), ( mForceRaster ? QStringLiteral( "1" ) : QStringLiteral( "0" ) ) );
  rendererElem.setAttribute( QStringLiteral( "symbollevels" ), ( mUsingSymbolLevels ? QStringLiteral( "1" ) : QStringLiteral( "0" ) ) );
  rendererElem.setAttribute( QStringLiteral( "referencescale" ), mReferenceScale );

  if ( mPaintEffect && !QgsPaintEffectRegistry::isDefaultStack( mPaintEffect ) )
    mPaintEffect->saveProperties( doc, rendererElem );

  if ( !mOrderBy.isEmpty() )
  {
    QDomElement orderBy = doc.createElement( QStringLiteral( "orderby" ) );
    mOrderBy.save( orderBy );
    rendererElem.appendChild( orderBy );
  }
  rendererElem.setAttribute( QStringLiteral( "enableorderby" ), ( mOrderByEnabled ? QStringLiteral( "1" ) : QStringLiteral( "0" ) ) );
}

QgsFeatureRenderer *QgsFeatureRenderer::loadSld( const QDomNode &node, QgsWkbTypes::GeometryType geomType, QString &errorMessage )
{
  const QDomElement element = node.toElement();
  if ( element.isNull() )
    return nullptr;

  // get the UserStyle element
  const QDomElement userStyleElem = element.firstChildElement( QStringLiteral( "UserStyle" ) );
  if ( userStyleElem.isNull() )
  {
    // UserStyle element not found, nothing will be rendered
    errorMessage = QStringLiteral( "Info: UserStyle element not found." );
    return nullptr;
  }

  // get the FeatureTypeStyle element
  QDomElement featTypeStyleElem = userStyleElem.firstChildElement( QStringLiteral( "FeatureTypeStyle" ) );
  if ( featTypeStyleElem.isNull() )
  {
    errorMessage = QStringLiteral( "Info: FeatureTypeStyle element not found." );
    return nullptr;
  }

  // create empty FeatureTypeStyle element to merge Rule's from all FeatureTypeStyle's
  QDomElement mergedFeatTypeStyle = featTypeStyleElem.cloneNode( false ).toElement();

  // use the RuleRenderer when more rules are present or the rule
  // has filters or min/max scale denominators set,
  // otherwise use the SingleSymbol renderer
  bool needRuleRenderer = false;
  int ruleCount = 0;

  while ( !featTypeStyleElem.isNull() )
  {
    QDomElement ruleElem = featTypeStyleElem.firstChildElement( QStringLiteral( "Rule" ) );
    while ( !ruleElem.isNull() )
    {
      // test rule children element to check if we need to create RuleRenderer
      // and if the rule has a symbolizer
      bool hasRendererSymbolizer = false;
      bool hasRuleRenderer = false;
      QDomElement ruleChildElem = ruleElem.firstChildElement();
      while ( !ruleChildElem.isNull() )
      {
        // rule has filter or min/max scale denominator, use the RuleRenderer
        if ( ruleChildElem.localName() == QLatin1String( "Filter" ) ||
             ruleChildElem.localName() == QLatin1String( "ElseFilter" ) ||
             ruleChildElem.localName() == QLatin1String( "MinScaleDenominator" ) ||
             ruleChildElem.localName() == QLatin1String( "MaxScaleDenominator" ) )
        {
          hasRuleRenderer = true;
        }
        // rule has a renderer symbolizer, not a text symbolizer
        else if ( ruleChildElem.localName().endsWith( QLatin1String( "Symbolizer" ) ) &&
                  ruleChildElem.localName() != QLatin1String( "TextSymbolizer" ) )
        {
          QgsDebugMsgLevel( QStringLiteral( "Symbolizer element found and not a TextSymbolizer" ), 2 );
          hasRendererSymbolizer = true;
        }

        ruleChildElem = ruleChildElem.nextSiblingElement();
      }

      if ( hasRendererSymbolizer )
      {
        ruleCount++;

        // append a clone of all Rules to the merged FeatureTypeStyle element
        mergedFeatTypeStyle.appendChild( ruleElem.cloneNode().toElement() );

        if ( hasRuleRenderer )
        {
          QgsDebugMsgLevel( QStringLiteral( "Filter or Min/MaxScaleDenominator element found: need a RuleRenderer" ), 2 );
          needRuleRenderer = true;
        }
      }

      // more rules present, use the RuleRenderer
      if ( ruleCount > 1 )
      {
        QgsDebugMsgLevel( QStringLiteral( "more Rule elements found: need a RuleRenderer" ), 2 );
        needRuleRenderer = true;
      }

      ruleElem = ruleElem.nextSiblingElement( QStringLiteral( "Rule" ) );
    }
    featTypeStyleElem = featTypeStyleElem.nextSiblingElement( QStringLiteral( "FeatureTypeStyle" ) );
  }

  QString rendererType;
  if ( needRuleRenderer )
  {
    rendererType = QStringLiteral( "RuleRenderer" );
  }
  else
  {
    rendererType = QStringLiteral( "singleSymbol" );
  }
  QgsDebugMsgLevel( QStringLiteral( "Instantiating a '%1' renderer..." ).arg( rendererType ), 2 );

  // create the renderer and return it
  QgsRendererAbstractMetadata *m = QgsApplication::rendererRegistry()->rendererMetadata( rendererType );
  if ( !m )
  {
    errorMessage = QStringLiteral( "Error: Unable to get metadata for '%1' renderer." ).arg( rendererType );
    return nullptr;
  }

  QgsFeatureRenderer *r = m->createRendererFromSld( mergedFeatTypeStyle, geomType );
  return r;
}

QDomElement QgsFeatureRenderer::writeSld( QDomDocument &doc, const QString &styleName, const QVariantMap &props ) const
{
  QDomElement userStyleElem = doc.createElement( QStringLiteral( "UserStyle" ) );

  QDomElement nameElem = doc.createElement( QStringLiteral( "se:Name" ) );
  nameElem.appendChild( doc.createTextNode( styleName ) );
  userStyleElem.appendChild( nameElem );

  QDomElement featureTypeStyleElem = doc.createElement( QStringLiteral( "se:FeatureTypeStyle" ) );
  toSld( doc, featureTypeStyleElem, props );
  userStyleElem.appendChild( featureTypeStyleElem );

  return userStyleElem;
}

bool QgsFeatureRenderer::legendSymbolItemsCheckable() const
{
  return false;
}

bool QgsFeatureRenderer::legendSymbolItemChecked( const QString &key )
{
  Q_UNUSED( key )
  return false;
}

void QgsFeatureRenderer::checkLegendSymbolItem( const QString &key, bool state )
{
  Q_UNUSED( key )
  Q_UNUSED( state )
}

void QgsFeatureRenderer::setLegendSymbolItem( const QString &key, QgsSymbol *symbol )
{
  Q_UNUSED( key )
  delete symbol;
}

QString QgsFeatureRenderer::legendKeyToExpression( const QString &, QgsVectorLayer *, bool &ok ) const
{
  ok = false;
  return QString();
}

QgsLegendSymbolList QgsFeatureRenderer::legendSymbolItems() const
{
  return QgsLegendSymbolList();
}

void QgsFeatureRenderer::setVertexMarkerAppearance( Qgis::VertexMarkerType type, double size )
{
  mCurrentVertexMarkerType = type;
  mCurrentVertexMarkerSize = size;
}

bool QgsFeatureRenderer::willRenderFeature( const QgsFeature &feature, QgsRenderContext &context ) const
{
  return nullptr != symbolForFeature( feature, context );
}

void QgsFeatureRenderer::renderVertexMarker( QPointF pt, QgsRenderContext &context )
{
  const int markerSize = context.convertToPainterUnits( mCurrentVertexMarkerSize, QgsUnitTypes::RenderMillimeters );
  QgsSymbolLayerUtils::drawVertexMarker( pt.x(), pt.y(), *context.painter(),
                                         mCurrentVertexMarkerType,
                                         markerSize );
}

void QgsFeatureRenderer::renderVertexMarkerPolyline( QPolygonF &pts, QgsRenderContext &context )
{
  const auto constPts = pts;
  for ( const QPointF pt : constPts )
    renderVertexMarker( pt, context );
}

void QgsFeatureRenderer::renderVertexMarkerPolygon( QPolygonF &pts, QList<QPolygonF> *rings, QgsRenderContext &context )
{
  const auto constPts = pts;
  for ( const QPointF pt : constPts )
    renderVertexMarker( pt, context );

  if ( rings )
  {
    const auto constRings = *rings;
    for ( const QPolygonF &ring : constRings )
    {
      const auto constRing = ring;
      for ( const QPointF pt : constRing )
        renderVertexMarker( pt, context );
    }
  }
}

QgsSymbolList QgsFeatureRenderer::symbolsForFeature( const QgsFeature &feature, QgsRenderContext &context ) const
{
  QgsSymbolList lst;
  QgsSymbol *s = symbolForFeature( feature, context );
  if ( s ) lst.append( s );
  return lst;
}

void QgsFeatureRenderer::modifyRequestExtent( QgsRectangle &extent, QgsRenderContext &context )
{
  Q_UNUSED( extent )
  Q_UNUSED( context )
}

QgsSymbolList QgsFeatureRenderer::originalSymbolsForFeature( const QgsFeature &feature, QgsRenderContext &context ) const
{
  QgsSymbolList lst;
  QgsSymbol *s = originalSymbolForFeature( feature, context );
  if ( s ) lst.append( s );
  return lst;
}

QgsPaintEffect *QgsFeatureRenderer::paintEffect() const
{
  return mPaintEffect;
}

void QgsFeatureRenderer::setPaintEffect( QgsPaintEffect *effect )
{
  delete mPaintEffect;
  mPaintEffect = effect;
}

QgsFeatureRequest::OrderBy QgsFeatureRenderer::orderBy() const
{
  return mOrderBy;
}

void QgsFeatureRenderer::setOrderBy( const QgsFeatureRequest::OrderBy &orderBy )
{
  mOrderBy = orderBy;
}

bool QgsFeatureRenderer::orderByEnabled() const
{
  return mOrderByEnabled;
}

void QgsFeatureRenderer::setOrderByEnabled( bool enabled )
{
  mOrderByEnabled = enabled;
}

void QgsFeatureRenderer::setEmbeddedRenderer( QgsFeatureRenderer *subRenderer )
{
  delete subRenderer;
}

const QgsFeatureRenderer *QgsFeatureRenderer::embeddedRenderer() const
{
  return nullptr;
}

bool QgsFeatureRenderer::accept( QgsStyleEntityVisitorInterface * ) const
{
  return true;
}

void QgsFeatureRenderer::convertSymbolSizeScale( QgsSymbol *symbol, Qgis::ScaleMethod method, const QString &field )
{
  if ( symbol->type() == Qgis:: SymbolType::Marker )
  {
    QgsMarkerSymbol *s = static_cast<QgsMarkerSymbol *>( symbol );
    if ( Qgis::ScaleMethod::ScaleArea == method )
    {
      s->setDataDefinedSize( QgsProperty::fromExpression( "coalesce(sqrt(" + QString::number( s->size() ) + " * (" + field + ")),0)" ) );
    }
    else
    {
      s->setDataDefinedSize( QgsProperty::fromExpression( "coalesce(" + QString::number( s->size() ) + " * (" + field + "),0)" ) );
    }
    s->setScaleMethod( Qgis::ScaleMethod::ScaleDiameter );
  }
  else if ( symbol->type() == Qgis::SymbolType::Line )
  {
    QgsLineSymbol *s = static_cast<QgsLineSymbol *>( symbol );
    s->setDataDefinedWidth( QgsProperty::fromExpression( "coalesce(" + QString::number( s->width() ) + " * (" + field + "),0)" ) );
  }
}

void QgsFeatureRenderer::convertSymbolRotation( QgsSymbol *symbol, const QString &field )
{
  if ( symbol->type() == Qgis::SymbolType::Marker )
  {
    QgsMarkerSymbol *s = static_cast<QgsMarkerSymbol *>( symbol );
    const QgsProperty dd = QgsProperty::fromExpression( ( s->angle()
                           ? QString::number( s->angle() ) + " + "
                           : QString() ) + field );
    s->setDataDefinedAngle( dd );
  }
}

QgsSymbol *QgsSymbolLevelItem::symbol() const
{
  return mSymbol;
}

int QgsSymbolLevelItem::layer() const
{
  return mLayer;
}
