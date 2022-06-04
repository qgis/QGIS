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

#include "qgssymbol.h"
#include "qgssymbollayerutils.h"

#include "qgsdatadefinedsizelegend.h"
#include "qgslogger.h"
#include "qgsfeature.h"
#include "qgsvectorlayer.h"
#include "qgssymbollayer.h"
#include "qgsogcutils.h"
#include "qgspointdisplacementrenderer.h"
#include "qgsinvertedpolygonrenderer.h"
#include "qgspainteffect.h"
#include "qgspainteffectregistry.h"
#include "qgsproperty.h"
#include "qgsstyleentityvisitor.h"
#include "qgslinesymbol.h"
#include "qgsmarkersymbol.h"
#include "qgsfillsymbol.h"

#include <QDomDocument>
#include <QDomElement>

QgsSingleSymbolRenderer::QgsSingleSymbolRenderer( QgsSymbol *symbol )
  : QgsFeatureRenderer( QStringLiteral( "singleSymbol" ) )
  , mSymbol( symbol )
{
  Q_ASSERT( symbol );
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
  return mSymbol ? QStringLiteral( "SINGLE: %1" ).arg( mSymbol->dump() ) : QString();
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
  QVariantMap newProps = props;

  QDomElement ruleElem = doc.createElement( QStringLiteral( "se:Rule" ) );
  element.appendChild( ruleElem );

  QDomElement nameElem = doc.createElement( QStringLiteral( "se:Name" ) );
  nameElem.appendChild( doc.createTextNode( QStringLiteral( "Single symbol" ) ) );
  ruleElem.appendChild( nameElem );

  QgsSymbolLayerUtils::applyScaleDependency( doc, ruleElem, newProps );

  if ( mSymbol ) mSymbol->toSld( doc, ruleElem, newProps );
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
  QDomElement symbolsElem = element.firstChildElement( QStringLiteral( "symbols" ) );
  if ( symbolsElem.isNull() )
    return nullptr;

  QgsSymbolMap symbolMap = QgsSymbolLayerUtils::loadSymbols( symbolsElem, context );

  if ( !symbolMap.contains( QStringLiteral( "0" ) ) )
    return nullptr;

  QgsSingleSymbolRenderer *r = new QgsSingleSymbolRenderer( symbolMap.take( QStringLiteral( "0" ) ) );

  // delete symbols if there are any more
  QgsSymbolLayerUtils::clearSymbolMap( symbolMap );

  const QDomElement rotationElem = element.firstChildElement( QStringLiteral( "rotation" ) );
  if ( !rotationElem.isNull() && !rotationElem.attribute( QStringLiteral( "field" ) ).isEmpty() )
  {
    convertSymbolRotation( r->mSymbol.get(), rotationElem.attribute( QStringLiteral( "field" ) ) );
  }

  const QDomElement sizeScaleElem = element.firstChildElement( QStringLiteral( "sizescale" ) );
  if ( !sizeScaleElem.isNull() && !sizeScaleElem.attribute( QStringLiteral( "field" ) ).isEmpty() )
  {
    convertSymbolSizeScale( r->mSymbol.get(),
                            QgsSymbolLayerUtils::decodeScaleMethod( sizeScaleElem.attribute( QStringLiteral( "scalemethod" ) ) ),
                            sizeScaleElem.attribute( QStringLiteral( "field" ) ) );
  }

  const QDomElement ddsLegendSizeElem = element.firstChildElement( QStringLiteral( "data-defined-size-legend" ) );
  if ( !ddsLegendSizeElem.isNull() )
  {
    r->mDataDefinedSizeLegend.reset( QgsDataDefinedSizeLegend::readXml( ddsLegendSizeElem, context ) );
  }

  // TODO: symbol levels
  return r;
}

QgsFeatureRenderer *QgsSingleSymbolRenderer::createFromSld( QDomElement &element, QgsWkbTypes::GeometryType geomType )
{
  // XXX this renderer can handle only one Rule!

  // get the first Rule element
  const QDomElement ruleElem = element.firstChildElement( QStringLiteral( "Rule" ) );
  if ( ruleElem.isNull() )
  {
    QgsDebugMsg( QStringLiteral( "no Rule elements found!" ) );
    return nullptr;
  }

  QString label, description;
  QgsSymbolLayerList layers;

  // retrieve the Rule element child nodes
  QDomElement childElem = ruleElem.firstChildElement();
  while ( !childElem.isNull() )
  {
    if ( childElem.localName() == QLatin1String( "Name" ) )
    {
      // <se:Name> tag contains the rule identifier,
      // so prefer title tag for the label property value
      if ( label.isEmpty() )
        label = childElem.firstChild().nodeValue();
    }
    else if ( childElem.localName() == QLatin1String( "Description" ) )
    {
      // <se:Description> can contains a title and an abstract
      const QDomElement titleElem = childElem.firstChildElement( QStringLiteral( "Title" ) );
      if ( !titleElem.isNull() )
      {
        label = titleElem.firstChild().nodeValue();
      }

      const QDomElement abstractElem = childElem.firstChildElement( QStringLiteral( "Abstract" ) );
      if ( !abstractElem.isNull() )
      {
        description = abstractElem.firstChild().nodeValue();
      }
    }
    else if ( childElem.localName() == QLatin1String( "Abstract" ) )
    {
      // <sld:Abstract> (v1.0)
      description = childElem.firstChild().nodeValue();
    }
    else if ( childElem.localName() == QLatin1String( "Title" ) )
    {
      // <sld:Title> (v1.0)
      label = childElem.firstChild().nodeValue();
    }
    else if ( childElem.localName().endsWith( QLatin1String( "Symbolizer" ) ) )
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
    case QgsWkbTypes::LineGeometry:
      symbol = std::make_unique< QgsLineSymbol >( layers );
      break;

    case QgsWkbTypes::PolygonGeometry:
      symbol = std::make_unique< QgsFillSymbol >( layers );
      break;

    case QgsWkbTypes::PointGeometry:
      symbol = std::make_unique< QgsMarkerSymbol >( layers );
      break;

    default:
      QgsDebugMsg( QStringLiteral( "invalid geometry type: found %1" ).arg( geomType ) );
      return nullptr;
  }

  // and finally return the new renderer
  return new QgsSingleSymbolRenderer( symbol.release() );
}

QDomElement QgsSingleSymbolRenderer::save( QDomDocument &doc, const QgsReadWriteContext &context )
{
  QDomElement rendererElem = doc.createElement( RENDERER_TAG_NAME );
  rendererElem.setAttribute( QStringLiteral( "type" ), QStringLiteral( "singleSymbol" ) );

  QgsSymbolMap symbols;
  symbols[QStringLiteral( "0" )] = mSymbol.get();
  const QDomElement symbolsElem = QgsSymbolLayerUtils::saveSymbols( symbols, QStringLiteral( "symbols" ), doc, context );
  rendererElem.appendChild( symbolsElem );

  const QDomElement rotationElem = doc.createElement( QStringLiteral( "rotation" ) );
  rendererElem.appendChild( rotationElem );

  const QDomElement sizeScaleElem = doc.createElement( QStringLiteral( "sizescale" ) );
  rendererElem.appendChild( sizeScaleElem );

  if ( mDataDefinedSizeLegend )
  {
    QDomElement ddsLegendElem = doc.createElement( QStringLiteral( "data-defined-size-legend" ) );
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
  lst << QgsLegendSymbolItem( mSymbol.get(), QString(), QStringLiteral( "0" ) );
  return lst;
}

QSet< QString > QgsSingleSymbolRenderer::legendKeysForFeature( const QgsFeature &feature, QgsRenderContext &context ) const
{
  Q_UNUSED( feature )
  Q_UNUSED( context )
  return QSet< QString >() << QStringLiteral( "0" );
}

QString QgsSingleSymbolRenderer::legendKeyToExpression( const QString &key, QgsVectorLayer *, bool &ok ) const
{
  if ( key == QLatin1String( "0" ) )
  {
    ok = true;
    return QStringLiteral( "TRUE" );
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
  if ( renderer->type() == QLatin1String( "singleSymbol" ) )
  {
    r = dynamic_cast<QgsSingleSymbolRenderer *>( renderer->clone() );
  }
  else if ( renderer->type() == QLatin1String( "pointDisplacement" ) || renderer->type() == QLatin1String( "pointCluster" ) )
  {
    const QgsPointDistanceRenderer *pointDistanceRenderer = dynamic_cast<const QgsPointDistanceRenderer *>( renderer );
    if ( pointDistanceRenderer )
      r = convertFromRenderer( pointDistanceRenderer->embeddedRenderer() );
  }
  else if ( renderer->type() == QLatin1String( "invertedPolygonRenderer" ) )
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
