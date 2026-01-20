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

#include <algorithm>

#include "qgsapplication.h"
#include "qgsfeature.h"
#include "qgslayertreemodellegendnode.h"
#include "qgslinesymbol.h"
#include "qgslogger.h"
#include "qgsmarkersymbol.h"
#include "qgspainteffect.h"
#include "qgspainteffectregistry.h"
#include "qgspoint.h"
#include "qgsproperty.h"
#include "qgsrendercontext.h"
#include "qgsrendererregistry.h"
#include "qgssinglesymbolrenderer.h"
#include "qgssldexportcontext.h"
#include "qgssymbol.h"
#include "qgssymbollayerutils.h"
#include "qgsvectorlayer.h"

#include <QDomDocument>
#include <QDomElement>
#include <QPolygonF>
#include <QThread>

QgsPropertiesDefinition QgsFeatureRenderer::sPropertyDefinitions;

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
  destRenderer->mDataDefinedProperties = mDataDefinedProperties;
}

QgsFeatureRenderer::QgsFeatureRenderer( const QString &type )
  : mType( type )
{
  mPaintEffect.reset( QgsPaintEffectRegistry::defaultStack() );
  mPaintEffect->setEnabled( false );
}

QgsFeatureRenderer::~QgsFeatureRenderer()
{

}

const QgsPropertiesDefinition &QgsFeatureRenderer::propertyDefinitions()
{
  QgsFeatureRenderer::initPropertyDefinitions();
  return sPropertyDefinitions;
}

QgsFeatureRenderer *QgsFeatureRenderer::defaultRenderer( Qgis::GeometryType geomType )
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

void QgsFeatureRenderer::startRender( QgsRenderContext &context, const QgsFields & )
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

  mDataDefinedProperties.prepare( context.expressionContext() );
}

bool QgsFeatureRenderer::canSkipRender()
{
  return false;
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
  return u"UNKNOWN RENDERER\n"_s;
}

Qgis::FeatureRendererFlags QgsFeatureRenderer::flags() const
{
  return Qgis::FeatureRendererFlags();
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
  const QString rendererType = element.attribute( u"type"_s );

  QgsRendererAbstractMetadata *m = QgsApplication::rendererRegistry()->rendererMetadata( rendererType );
  if ( !m )
    return nullptr;

  QgsFeatureRenderer *r = m->createRenderer( element, context );
  if ( r )
  {
    r->setUsingSymbolLevels( element.attribute( u"symbollevels"_s, u"0"_s ).toInt() );
    r->setForceRasterRender( element.attribute( u"forceraster"_s, u"0"_s ).toInt() );
    r->setReferenceScale( element.attribute( u"referencescale"_s, u"-1"_s ).toDouble() );

    //restore layer effect
    const QDomElement effectElem = element.firstChildElement( u"effect"_s );
    if ( !effectElem.isNull() )
    {
      r->setPaintEffect( QgsApplication::paintEffectRegistry()->createEffect( effectElem ) );
    }

    // restore order by
    const QDomElement orderByElem = element.firstChildElement( u"orderby"_s );
    r->mOrderBy.load( orderByElem );
    r->setOrderByEnabled( element.attribute( u"enableorderby"_s, u"0"_s ).toInt() );

    const QDomElement elemDataDefinedProperties = element.firstChildElement( u"data-defined-properties"_s );
    if ( !elemDataDefinedProperties.isNull() )
      r->mDataDefinedProperties.readXml( elemDataDefinedProperties, propertyDefinitions() );
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
  rendererElem.setAttribute( u"forceraster"_s, ( mForceRaster ? u"1"_s : u"0"_s ) );
  rendererElem.setAttribute( u"symbollevels"_s, ( mUsingSymbolLevels ? u"1"_s : u"0"_s ) );
  rendererElem.setAttribute( u"referencescale"_s, mReferenceScale );

  QDomElement elemDataDefinedProperties = doc.createElement( u"data-defined-properties"_s );
  mDataDefinedProperties.writeXml( elemDataDefinedProperties, propertyDefinitions() );
  rendererElem.appendChild( elemDataDefinedProperties );

  if ( mPaintEffect && !QgsPaintEffectRegistry::isDefaultStack( mPaintEffect.get() ) )
    mPaintEffect->saveProperties( doc, rendererElem );

  if ( !mOrderBy.isEmpty() )
  {
    QDomElement orderBy = doc.createElement( u"orderby"_s );
    mOrderBy.save( orderBy );
    rendererElem.appendChild( orderBy );
  }
  rendererElem.setAttribute( u"enableorderby"_s, ( mOrderByEnabled ? u"1"_s : u"0"_s ) );
}

QgsFeatureRenderer *QgsFeatureRenderer::loadSld( const QDomNode &node, Qgis::GeometryType geomType, QString &errorMessage )
{
  const QDomElement element = node.toElement();
  if ( element.isNull() )
    return nullptr;

  // get the UserStyle element
  const QDomElement userStyleElem = element.firstChildElement( u"UserStyle"_s );
  if ( userStyleElem.isNull() )
  {
    // UserStyle element not found, nothing will be rendered
    errorMessage = u"Info: UserStyle element not found."_s;
    return nullptr;
  }

  // get the FeatureTypeStyle element
  QDomElement featTypeStyleElem = userStyleElem.firstChildElement( u"FeatureTypeStyle"_s );
  if ( featTypeStyleElem.isNull() )
  {
    errorMessage = u"Info: FeatureTypeStyle element not found."_s;
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
    QDomElement ruleElem = featTypeStyleElem.firstChildElement( u"Rule"_s );
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
        if ( ruleChildElem.localName() == "Filter"_L1 ||
             ruleChildElem.localName() == "ElseFilter"_L1 ||
             ruleChildElem.localName() == "MinScaleDenominator"_L1 ||
             ruleChildElem.localName() == "MaxScaleDenominator"_L1 )
        {
          hasRuleRenderer = true;
        }
        // rule has a renderer symbolizer, not a text symbolizer
        else if ( ruleChildElem.localName().endsWith( "Symbolizer"_L1 ) &&
                  ruleChildElem.localName() != "TextSymbolizer"_L1 )
        {
          QgsDebugMsgLevel( u"Symbolizer element found and not a TextSymbolizer"_s, 2 );
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
          QgsDebugMsgLevel( u"Filter or Min/MaxScaleDenominator element found: need a RuleRenderer"_s, 2 );
          needRuleRenderer = true;
        }
      }

      // more rules present, use the RuleRenderer
      if ( ruleCount > 1 )
      {
        QgsDebugMsgLevel( u"more Rule elements found: need a RuleRenderer"_s, 2 );
        needRuleRenderer = true;
      }

      ruleElem = ruleElem.nextSiblingElement( u"Rule"_s );
    }
    featTypeStyleElem = featTypeStyleElem.nextSiblingElement( u"FeatureTypeStyle"_s );
  }

  QString rendererType;
  if ( needRuleRenderer )
  {
    rendererType = u"RuleRenderer"_s;
  }
  else
  {
    rendererType = u"singleSymbol"_s;
  }
  QgsDebugMsgLevel( u"Instantiating a '%1' renderer..."_s.arg( rendererType ), 2 );

  // create the renderer and return it
  QgsRendererAbstractMetadata *m = QgsApplication::rendererRegistry()->rendererMetadata( rendererType );
  if ( !m )
  {
    errorMessage = u"Error: Unable to get metadata for '%1' renderer."_s.arg( rendererType );
    return nullptr;
  }

  QgsFeatureRenderer *r = m->createRendererFromSld( mergedFeatTypeStyle, geomType );
  return r;
}

void QgsFeatureRenderer::toSld( QDomDocument &doc, QDomElement &element, const QVariantMap &props ) const
{
  QgsSldExportContext context;
  context.setExtraProperties( props );
  toSld( doc, element, context );
}

bool QgsFeatureRenderer::toSld( QDomDocument &, QDomElement &, QgsSldExportContext &context ) const
{
  context.pushError( QObject::tr( "Vector %1 renderer cannot be converted to SLD" ).arg( type() ) );
  return false;
}

QSet<QString> QgsFeatureRenderer::legendKeys() const
{
  // build up a list of unique legend keys
  const QgsLegendSymbolList allLegendSymbols = legendSymbolItems();
  QSet< QString > keys;
  keys.reserve( allLegendSymbols.size() );
  for ( const QgsLegendSymbolItem &symbol : allLegendSymbols )
  {
    keys.insert( symbol.ruleKey() );
  }
  return keys;
}

QDomElement QgsFeatureRenderer::writeSld( QDomDocument &doc, const QString &styleName, const QVariantMap &props ) const
{
  QDomElement userStyleElem = doc.createElement( u"UserStyle"_s );

  QDomElement nameElem = doc.createElement( u"se:Name"_s );
  nameElem.appendChild( doc.createTextNode( styleName ) );
  userStyleElem.appendChild( nameElem );

  QDomElement featureTypeStyleElem = doc.createElement( u"se:FeatureTypeStyle"_s );
  QgsSldExportContext context;
  context.setExtraProperties( props );

  toSld( doc, featureTypeStyleElem, context );
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

double QgsFeatureRenderer::maximumExtentBuffer( QgsRenderContext &context ) const
{
  const QgsSymbolList symbolList = symbols( context );

  if ( symbolList.empty() )
    return 0;

  QgsExpressionContext &expContext = context.expressionContext();

  auto getValueFromSymbol = [ &expContext, &context ]( const QgsSymbol * sym ) -> double
  {
    const QgsProperty property = sym->dataDefinedProperties().property( QgsSymbol::Property::ExtentBuffer );

    double value = 0.0;

    if ( property.isActive() )
    {
      expContext.setOriginalValueVariable( sym->extentBuffer() );

      value = sym->dataDefinedProperties().valueAsDouble( QgsSymbol::Property::ExtentBuffer, expContext, sym->extentBuffer() );
      if ( value < 0 )
        value = 0;
    }
    else
    {
      value = sym->extentBuffer();
    }

    if ( sym->extentBufferSizeUnit() != Qgis::RenderUnit::MapUnits )
    {
      value = context.convertToMapUnits( value, sym->extentBufferSizeUnit(), sym->mapUnitScale() );
    }

    return value;
  };

  if ( symbolList.size() == 1 )
    return getValueFromSymbol( symbolList[0] );

  auto it = std::max_element( symbolList.constBegin(), symbolList.constEnd(), [ &getValueFromSymbol ]( const QgsSymbol * a, const QgsSymbol * b ) -> bool
  {
    return getValueFromSymbol( a ) < getValueFromSymbol( b );
  } );

  return getValueFromSymbol( *it );
}

QList<QgsLayerTreeModelLegendNode *> QgsFeatureRenderer::createLegendNodes( QgsLayerTreeLayer *nodeLayer ) const
{
  QList<QgsLayerTreeModelLegendNode *> nodes;

  const QgsLegendSymbolList symbolItems = legendSymbolItems();
  nodes.reserve( symbolItems.size() );

  for ( const QgsLegendSymbolItem &item : symbolItems )
  {
    if ( const QgsDataDefinedSizeLegend *dataDefinedSizeLegendSettings = item.dataDefinedSizeLegendSettings() )
    {
      nodes << new QgsDataDefinedSizeLegendNode( nodeLayer, *dataDefinedSizeLegendSettings );
    }
    else
    {
      nodes << new QgsSymbolLegendNode( nodeLayer, item );
    }
  }
  return nodes;
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
  const int markerSize = context.convertToPainterUnits( mCurrentVertexMarkerSize, Qgis::RenderUnit::Millimeters );
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
  double extentBuffer = maximumExtentBuffer( context );

  extent.grow( extentBuffer );
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
  return mPaintEffect.get();
}

void QgsFeatureRenderer::setPaintEffect( QgsPaintEffect *effect )
{
  mPaintEffect.reset( effect );

}

void QgsFeatureRenderer::setDataDefinedProperty( Property key, const QgsProperty &property )
{
  mDataDefinedProperties.setProperty( key, property );
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

void QgsFeatureRenderer::initPropertyDefinitions()
{
  if ( !sPropertyDefinitions.isEmpty() )
    return;

  QString origin = u"renderer"_s;

  sPropertyDefinitions = QgsPropertiesDefinition
  {
    { static_cast< int >( QgsFeatureRenderer::Property::HeatmapRadius ), QgsPropertyDefinition( "heatmapRadius", QObject::tr( "Radius" ), QgsPropertyDefinition::DoublePositive, origin )},
    { static_cast< int >( QgsFeatureRenderer::Property::HeatmapMaximum ), QgsPropertyDefinition( "heatmapMaximum", QObject::tr( "Maximum" ), QgsPropertyDefinition::DoublePositive, origin )},
  };
}

QgsSymbol *QgsSymbolLevelItem::symbol() const
{
  return mSymbol;
}

int QgsSymbolLevelItem::layer() const
{
  return mLayer;
}
