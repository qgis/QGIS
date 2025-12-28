/***************************************************************************
    qgssinglesymbolrenderer.cpp
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

#include "qgssinglesymbolrenderer.h"

#include "qgsdatadefinedsizelegend.h"
#include "qgsfeature.h"
#include "qgsfillsymbol.h"
#include "qgsinvertedpolygonrenderer.h"
#include "qgslinesymbol.h"
#include "qgslogger.h"
#include "qgsmarkersymbol.h"
#include "qgsogcutils.h"
#include "qgspainteffect.h"
#include "qgspainteffectregistry.h"
#include "qgspointdisplacementrenderer.h"
#include "qgsproperty.h"
#include "qgssldexportcontext.h"
#include "qgsstyleentityvisitor.h"
#include "qgssymbol.h"
#include "qgssymbollayer.h"
#include "qgssymbollayerutils.h"
#include "qgsvectorlayer.h"

#include <QDomDocument>
#include <QDomElement>

QgsSingleSymbolRenderer::QgsSingleSymbolRenderer( QgsSymbol *symbol )
  : QgsFeatureRenderer( u"singleSymbol"_s )
  , mSymbol( symbol )
{
  Q_ASSERT( symbol );
}

Qgis::FeatureRendererFlags QgsSingleSymbolRenderer::flags() const
{
  Qgis::FeatureRendererFlags res;
  if ( mSymbol && mSymbol->flags().testFlag( Qgis::SymbolFlag::AffectsLabeling ) )
    res.setFlag( Qgis::FeatureRendererFlag::AffectsLabeling );
  return res;
}

QgsSingleSymbolRenderer::~QgsSingleSymbolRenderer() = default;

QgsSymbol *QgsSingleSymbolRenderer::symbolForFeature( const QgsFeature &, QgsRenderContext & ) const
{
  return mSymbol.get();
}

QgsSymbol *QgsSingleSymbolRenderer::originalSymbolForFeature( const QgsFeature &feature, QgsRenderContext &context ) const
{
  Q_UNUSED( context )
  Q_UNUSED( feature )
  return mSymbol.get();
}

void QgsSingleSymbolRenderer::startRender( QgsRenderContext &context, const QgsFields &fields )
{
  QgsFeatureRenderer::startRender( context, fields );

  if ( !mSymbol )
    return;

  mSymbol->startRender( context, fields );
}

void QgsSingleSymbolRenderer::stopRender( QgsRenderContext &context )
{
  QgsFeatureRenderer::stopRender( context );

  if ( !mSymbol )
    return;

  mSymbol->stopRender( context );
}

QSet<QString> QgsSingleSymbolRenderer::usedAttributes( const QgsRenderContext &context ) const
{
  QSet<QString> attributes;
  if ( mSymbol )
    attributes.unite( mSymbol->usedAttributes( context ) );
  return attributes;
}

bool QgsSingleSymbolRenderer::accept( QgsStyleEntityVisitorInterface *visitor ) const
{
  if ( mSymbol )
  {
    QgsStyleSymbolEntity entity( mSymbol.get() );
    return visitor->visit( QgsStyleEntityVisitorInterface::StyleLeaf( &entity ) );
  }
  return true;
}

QgsSymbol *QgsSingleSymbolRenderer::symbol() const
{
  return mSymbol.get();
}

void QgsSingleSymbolRenderer::setSymbol( QgsSymbol *s )
{
  Q_ASSERT( s );
  mSymbol.reset( s );
}

QString QgsSingleSymbolRenderer::dump() const
{
  return mSymbol ? u"SINGLE: %1"_s.arg( mSymbol->dump() ) : QString();
}

QgsSingleSymbolRenderer *QgsSingleSymbolRenderer::clone() const
{
  QgsSingleSymbolRenderer *r = new QgsSingleSymbolRenderer( mSymbol->clone() );
  r->setDataDefinedSizeLegend( mDataDefinedSizeLegend ? new QgsDataDefinedSizeLegend( *mDataDefinedSizeLegend ) : nullptr );
  copyRendererData( r );
  return r;
}

void QgsSingleSymbolRenderer::toSld( QDomDocument &doc, QDomElement &element, const QVariantMap &props ) const
{
  QgsSldExportContext context;
  context.setExtraProperties( props );
  toSld( doc, element, context );
}

bool QgsSingleSymbolRenderer::toSld( QDomDocument &doc, QDomElement &element, QgsSldExportContext &context ) const
{
  const QVariantMap oldProps = context.extraProperties();
  QVariantMap newProps = oldProps;

  QDomElement ruleElem = doc.createElement( u"se:Rule"_s );
  element.appendChild( ruleElem );

  QDomElement nameElem = doc.createElement( u"se:Name"_s );
  nameElem.appendChild( doc.createTextNode( u"Single symbol"_s ) );
  ruleElem.appendChild( nameElem );

  QgsSymbolLayerUtils::applyScaleDependency( doc, ruleElem, newProps );

  context.setExtraProperties( newProps );
  if ( mSymbol )
    mSymbol->toSld( doc, ruleElem, context );

  context.setExtraProperties( oldProps );
  return true;
}

QgsSymbolList QgsSingleSymbolRenderer::symbols( QgsRenderContext &context ) const
{
  Q_UNUSED( context )
  QgsSymbolList lst;
  lst.append( mSymbol.get() );
  return lst;
}


QgsFeatureRenderer *QgsSingleSymbolRenderer::create( QDomElement &element, const QgsReadWriteContext &context )
{
  QDomElement symbolsElem = element.firstChildElement( u"symbols"_s );
  if ( symbolsElem.isNull() )
    return nullptr;

  QgsSymbolMap symbolMap = QgsSymbolLayerUtils::loadSymbols( symbolsElem, context );

  if ( !symbolMap.contains( u"0"_s ) )
    return nullptr;

  QgsSingleSymbolRenderer *r = new QgsSingleSymbolRenderer( symbolMap.take( u"0"_s ) );

  // delete symbols if there are any more
  QgsSymbolLayerUtils::clearSymbolMap( symbolMap );

  const QDomElement rotationElem = element.firstChildElement( u"rotation"_s );
  if ( !rotationElem.isNull() && !rotationElem.attribute( u"field"_s ).isEmpty() )
  {
    convertSymbolRotation( r->mSymbol.get(), rotationElem.attribute( u"field"_s ) );
  }

  const QDomElement sizeScaleElem = element.firstChildElement( u"sizescale"_s );
  if ( !sizeScaleElem.isNull() && !sizeScaleElem.attribute( u"field"_s ).isEmpty() )
  {
    convertSymbolSizeScale( r->mSymbol.get(),
                            QgsSymbolLayerUtils::decodeScaleMethod( sizeScaleElem.attribute( u"scalemethod"_s ) ),
                            sizeScaleElem.attribute( u"field"_s ) );
  }

  const QDomElement ddsLegendSizeElem = element.firstChildElement( u"data-defined-size-legend"_s );
  if ( !ddsLegendSizeElem.isNull() )
  {
    r->mDataDefinedSizeLegend.reset( QgsDataDefinedSizeLegend::readXml( ddsLegendSizeElem, context ) );
  }

  // TODO: symbol levels
  return r;
}

QgsFeatureRenderer *QgsSingleSymbolRenderer::createFromSld( QDomElement &element, Qgis::GeometryType geomType )
{
  // XXX this renderer can handle only one Rule!

  // get the first Rule element
  const QDomElement ruleElem = element.firstChildElement( u"Rule"_s );
  if ( ruleElem.isNull() )
  {
    QgsDebugError( u"no Rule elements found!"_s );
    return nullptr;
  }

  QString label, description;
  QgsSymbolLayerList layers;

  // retrieve the Rule element child nodes
  QDomElement childElem = ruleElem.firstChildElement();
  while ( !childElem.isNull() )
  {
    if ( childElem.localName() == "Name"_L1 )
    {
      // <se:Name> tag contains the rule identifier,
      // so prefer title tag for the label property value
      if ( label.isEmpty() )
        label = childElem.firstChild().nodeValue();
    }
    else if ( childElem.localName() == "Description"_L1 )
    {
      // <se:Description> can contains a title and an abstract
      const QDomElement titleElem = childElem.firstChildElement( u"Title"_s );
      if ( !titleElem.isNull() )
      {
        label = titleElem.firstChild().nodeValue();
      }

      const QDomElement abstractElem = childElem.firstChildElement( u"Abstract"_s );
      if ( !abstractElem.isNull() )
      {
        description = abstractElem.firstChild().nodeValue();
      }
    }
    else if ( childElem.localName() == "Abstract"_L1 )
    {
      // <sld:Abstract> (v1.0)
      description = childElem.firstChild().nodeValue();
    }
    else if ( childElem.localName() == "Title"_L1 )
    {
      // <sld:Title> (v1.0)
      label = childElem.firstChild().nodeValue();
    }
    else if ( childElem.localName().endsWith( "Symbolizer"_L1 ) )
    {
      // create symbol layers for this symbolizer
      QgsSymbolLayerUtils::createSymbolLayerListFromSld( childElem, geomType, layers );
    }

    childElem = childElem.nextSiblingElement();
  }

  if ( layers.isEmpty() )
    return nullptr;

  // now create the symbol
  std::unique_ptr< QgsSymbol > symbol;
  switch ( geomType )
  {
    case Qgis::GeometryType::Line:
      symbol = std::make_unique< QgsLineSymbol >( layers );
      break;

    case Qgis::GeometryType::Polygon:
      symbol = std::make_unique< QgsFillSymbol >( layers );
      break;

    case Qgis::GeometryType::Point:
      symbol = std::make_unique< QgsMarkerSymbol >( layers );
      break;

    default:
      QgsDebugError( u"invalid geometry type: found %1"_s.arg( qgsEnumValueToKey( geomType ) ) );
      return nullptr;
  }

  // and finally return the new renderer
  return new QgsSingleSymbolRenderer( symbol.release() );
}

QDomElement QgsSingleSymbolRenderer::save( QDomDocument &doc, const QgsReadWriteContext &context )
{
  QDomElement rendererElem = doc.createElement( RENDERER_TAG_NAME );
  rendererElem.setAttribute( u"type"_s, u"singleSymbol"_s );

  QgsSymbolMap symbols;
  symbols[u"0"_s] = mSymbol.get();
  const QDomElement symbolsElem = QgsSymbolLayerUtils::saveSymbols( symbols, u"symbols"_s, doc, context );
  rendererElem.appendChild( symbolsElem );

  const QDomElement rotationElem = doc.createElement( u"rotation"_s );
  rendererElem.appendChild( rotationElem );

  const QDomElement sizeScaleElem = doc.createElement( u"sizescale"_s );
  rendererElem.appendChild( sizeScaleElem );

  if ( mDataDefinedSizeLegend )
  {
    QDomElement ddsLegendElem = doc.createElement( u"data-defined-size-legend"_s );
    mDataDefinedSizeLegend->writeXml( ddsLegendElem, context );
    rendererElem.appendChild( ddsLegendElem );
  }

  saveRendererData( doc, rendererElem, context );

  return rendererElem;
}

QgsLegendSymbolList QgsSingleSymbolRenderer::legendSymbolItems() const
{
  if ( mDataDefinedSizeLegend && mSymbol->type() == Qgis::SymbolType::Marker )
  {
    const QgsMarkerSymbol *symbol = static_cast<const QgsMarkerSymbol *>( mSymbol.get() );
    const QgsProperty sizeDD( symbol->dataDefinedSize() );
    if ( sizeDD && sizeDD.isActive() )
    {
      QgsDataDefinedSizeLegend ddSizeLegend( *mDataDefinedSizeLegend );
      ddSizeLegend.updateFromSymbolAndProperty( static_cast<const QgsMarkerSymbol *>( mSymbol.get() ), sizeDD );
      return ddSizeLegend.legendSymbolList();
    }
  }

  QgsLegendSymbolList lst;
  lst << QgsLegendSymbolItem( mSymbol.get(), QString(), u"0"_s );
  return lst;
}

QSet< QString > QgsSingleSymbolRenderer::legendKeysForFeature( const QgsFeature &feature, QgsRenderContext &context ) const
{
  Q_UNUSED( feature )
  Q_UNUSED( context )
  return QSet< QString >() << u"0"_s;
}

QString QgsSingleSymbolRenderer::legendKeyToExpression( const QString &key, QgsVectorLayer *, bool &ok ) const
{
  if ( key == "0"_L1 )
  {
    ok = true;
    return u"TRUE"_s;
  }
  else
  {
    ok = false;
    return QString();
  }
}

void QgsSingleSymbolRenderer::setLegendSymbolItem( const QString &key, QgsSymbol *symbol )
{
  Q_UNUSED( key )
  setSymbol( symbol );
}

QgsSingleSymbolRenderer *QgsSingleSymbolRenderer::convertFromRenderer( const QgsFeatureRenderer *renderer )
{
  QgsSingleSymbolRenderer *r = nullptr;
  if ( renderer->type() == "singleSymbol"_L1 )
  {
    r = dynamic_cast<QgsSingleSymbolRenderer *>( renderer->clone() );
  }
  else if ( renderer->type() == "pointDisplacement"_L1 || renderer->type() == "pointCluster"_L1 )
  {
    const QgsPointDistanceRenderer *pointDistanceRenderer = dynamic_cast<const QgsPointDistanceRenderer *>( renderer );
    if ( pointDistanceRenderer )
      r = convertFromRenderer( pointDistanceRenderer->embeddedRenderer() );
  }
  else if ( renderer->type() == "invertedPolygonRenderer"_L1 )
  {
    const QgsInvertedPolygonRenderer *invertedPolygonRenderer = dynamic_cast<const QgsInvertedPolygonRenderer *>( renderer );
    if ( invertedPolygonRenderer )
      r = convertFromRenderer( invertedPolygonRenderer->embeddedRenderer() );
  }

  if ( !r )
  {
    QgsRenderContext context;
    const QgsSymbolList symbols = const_cast<QgsFeatureRenderer *>( renderer )->symbols( context );
    if ( !symbols.isEmpty() )
    {
      r = new QgsSingleSymbolRenderer( symbols.at( 0 )->clone() );
    }
  }

  if ( r )
  {
    renderer->copyRendererData( r );
  }

  return r;
}

void QgsSingleSymbolRenderer::setDataDefinedSizeLegend( QgsDataDefinedSizeLegend *settings )
{
  mDataDefinedSizeLegend.reset( settings );
}

QgsDataDefinedSizeLegend *QgsSingleSymbolRenderer::dataDefinedSizeLegend() const
{
  return mDataDefinedSizeLegend.get();
}
