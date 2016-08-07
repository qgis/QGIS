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

#include "qgslogger.h"
#include "qgsfeature.h"
#include "qgsvectorlayer.h"
#include "qgssymbollayer.h"
#include "qgsogcutils.h"
#include "qgspointdisplacementrenderer.h"
#include "qgsinvertedpolygonrenderer.h"
#include "qgspainteffect.h"
#include "qgspainteffectregistry.h"
#include "qgsscaleexpression.h"
#include "qgsdatadefined.h"

#include <QDomDocument>
#include <QDomElement>

QgsSingleSymbolRenderer::QgsSingleSymbolRenderer( QgsSymbol* symbol )
    : QgsFeatureRenderer( "singleSymbol" )
    , mSymbol( symbol )
    , mScaleMethod( DEFAULT_SCALE_METHOD )
    , mOrigSize( 0.0 )
{
  Q_ASSERT( symbol );
}

QgsSingleSymbolRenderer::~QgsSingleSymbolRenderer()
{
}

QgsSymbol* QgsSingleSymbolRenderer::symbolForFeature( QgsFeature& feature, QgsRenderContext &context )
{
  context.expressionContext().setFeature( feature );
  if ( !mRotation.data() && !mSizeScale.data() ) return mSymbol.data();

  const double rotation = mRotation.data() ? mRotation->evaluate( &context.expressionContext() ).toDouble() : 0;
  const double sizeScale = mSizeScale.data() ? mSizeScale->evaluate( &context.expressionContext() ).toDouble() : 1.;

  if ( mTempSymbol->type() == QgsSymbol::Marker )
  {
    QgsMarkerSymbol* markerSymbol = static_cast<QgsMarkerSymbol*>( mTempSymbol.data() );
    if ( mRotation.data() ) markerSymbol->setAngle( rotation );
    markerSymbol->setSize( sizeScale * mOrigSize );
    markerSymbol->setScaleMethod( mScaleMethod );
  }
  else if ( mTempSymbol->type() == QgsSymbol::Line )
  {
    QgsLineSymbol* lineSymbol = static_cast<QgsLineSymbol*>( mTempSymbol.data() );
    lineSymbol->setWidth( sizeScale * mOrigSize );
  }
  else if ( mTempSymbol->type() == QgsSymbol::Fill )
  {
    QgsFillSymbol* fillSymbol = static_cast<QgsFillSymbol*>( mTempSymbol.data() );
    if ( mRotation.data() ) fillSymbol->setAngle( rotation );
  }

  return mTempSymbol.data();
}

QgsSymbol* QgsSingleSymbolRenderer::originalSymbolForFeature( QgsFeature& feature, QgsRenderContext &context )
{
  Q_UNUSED( context );
  Q_UNUSED( feature );
  return mSymbol.data();
}

void QgsSingleSymbolRenderer::startRender( QgsRenderContext& context, const QgsFields& fields )
{
  if ( !mSymbol.data() )
    return;

  mSymbol->startRender( context, fields );

  if ( mRotation.data() || mSizeScale.data() )
  {
    // we are going to need a temporary symbol
    mTempSymbol.reset( mSymbol->clone() );

    int hints = 0;
    if ( mRotation.data() )
      hints |= QgsSymbol::DataDefinedRotation;
    if ( mSizeScale.data() )
      hints |= QgsSymbol::DataDefinedSizeScale;
    mTempSymbol->setRenderHints( hints );

    mTempSymbol->startRender( context, fields );

    if ( mSymbol->type() == QgsSymbol::Marker )
    {
      mOrigSize = static_cast<QgsMarkerSymbol*>( mSymbol.data() )->size();
    }
    else if ( mSymbol->type() == QgsSymbol::Line )
    {
      mOrigSize = static_cast<QgsLineSymbol*>( mSymbol.data() )->width();
    }
    else
    {
      mOrigSize = 0;
    }
  }

  return;
}

void QgsSingleSymbolRenderer::stopRender( QgsRenderContext& context )
{
  if ( !mSymbol.data() ) return;

  mSymbol->stopRender( context );

  if ( mRotation.data() || mSizeScale.data() )
  {
    // we are going to need a temporary symbol
    mTempSymbol->stopRender( context );
    mTempSymbol.reset();
  }
}

QList<QString> QgsSingleSymbolRenderer::usedAttributes()
{
  QSet<QString> attributes;
  if ( mSymbol.data() ) attributes.unite( mSymbol->usedAttributes() );
  if ( mRotation.data() ) attributes.unite( mRotation->referencedColumns().toSet() );
  if ( mSizeScale.data() ) attributes.unite( mSizeScale->referencedColumns().toSet() );
  return attributes.toList();
}

QgsSymbol* QgsSingleSymbolRenderer::symbol() const
{
  return mSymbol.data();
}

void QgsSingleSymbolRenderer::setSymbol( QgsSymbol* s )
{
  Q_ASSERT( s );
  mSymbol.reset( s );
}

void QgsSingleSymbolRenderer::setRotationField( const QString& fieldOrExpression )
{
  if ( mSymbol->type() == QgsSymbol::Marker )
  {
    QgsMarkerSymbol * s = static_cast<QgsMarkerSymbol *>( mSymbol.data() );
    s->setDataDefinedAngle( QgsDataDefined( fieldOrExpression ) );
  }
}

QString QgsSingleSymbolRenderer::rotationField() const
{
  if ( mSymbol->type() == QgsSymbol::Marker )
  {
    QgsMarkerSymbol * s = static_cast<QgsMarkerSymbol *>( mSymbol.data() );
    QgsDataDefined ddAngle = s->dataDefinedAngle();
    return ddAngle.useExpression() ? ddAngle.expressionString() : ddAngle.field();
  }

  return QString();
}

void QgsSingleSymbolRenderer::setSizeScaleField( const QString& fieldOrExpression )
{
  mSizeScale.reset( QgsSymbolLayerUtils::fieldOrExpressionToExpression( fieldOrExpression ) );
}

QString QgsSingleSymbolRenderer::sizeScaleField() const
{
  return mSizeScale.data() ? QgsSymbolLayerUtils::fieldOrExpressionFromExpression( mSizeScale.data() ) : QString();
}

void QgsSingleSymbolRenderer::setScaleMethod( QgsSymbol::ScaleMethod scaleMethod )
{
  mScaleMethod = scaleMethod;
  setScaleMethodToSymbol( mSymbol.data(), scaleMethod );
}

QString QgsSingleSymbolRenderer::dump() const
{
  return mSymbol.data() ? QString( "SINGLE: %1" ).arg( mSymbol->dump() ) : "";
}

QgsSingleSymbolRenderer* QgsSingleSymbolRenderer::clone() const
{
  QgsSingleSymbolRenderer* r = new QgsSingleSymbolRenderer( mSymbol->clone() );
  r->setUsingSymbolLevels( usingSymbolLevels() );
  r->setSizeScaleField( sizeScaleField() );
  copyRendererData( r );
  return r;
}

void QgsSingleSymbolRenderer::toSld( QDomDocument& doc, QDomElement &element ) const
{
  QgsStringMap props;
  if ( mRotation.data() )
    props[ "angle" ] = mRotation->expression();
  if ( mSizeScale.data() )
    props[ "scale" ] = mSizeScale->expression();

  QDomElement ruleElem = doc.createElement( "se:Rule" );
  element.appendChild( ruleElem );

  QDomElement nameElem = doc.createElement( "se:Name" );
  nameElem.appendChild( doc.createTextNode( "Single symbol" ) );
  ruleElem.appendChild( nameElem );

  if ( mSymbol.data() ) mSymbol->toSld( doc, ruleElem, props );
}

QgsSymbolList QgsSingleSymbolRenderer::symbols( QgsRenderContext &context )
{
  Q_UNUSED( context );
  QgsSymbolList lst;
  lst.append( mSymbol.data() );
  return lst;
}


QgsFeatureRenderer* QgsSingleSymbolRenderer::create( QDomElement& element )
{
  QDomElement symbolsElem = element.firstChildElement( "symbols" );
  if ( symbolsElem.isNull() )
    return nullptr;

  QgsSymbolMap symbolMap = QgsSymbolLayerUtils::loadSymbols( symbolsElem );

  if ( !symbolMap.contains( "0" ) )
    return nullptr;

  QgsSingleSymbolRenderer* r = new QgsSingleSymbolRenderer( symbolMap.take( "0" ) );

  // delete symbols if there are any more
  QgsSymbolLayerUtils::clearSymbolMap( symbolMap );

  QDomElement rotationElem = element.firstChildElement( "rotation" );
  if ( !rotationElem.isNull() && !rotationElem.attribute( "field" ).isEmpty() )
  {
    convertSymbolRotation( r->mSymbol.data(), rotationElem.attribute( "field" ) );
  }

  QDomElement sizeScaleElem = element.firstChildElement( "sizescale" );
  if ( !sizeScaleElem.isNull() && !sizeScaleElem.attribute( "field" ).isEmpty() )
  {
    convertSymbolSizeScale( r->mSymbol.data(),
                            QgsSymbolLayerUtils::decodeScaleMethod( sizeScaleElem.attribute( "scalemethod" ) ),
                            sizeScaleElem.attribute( "field" ) );
  }

  // TODO: symbol levels
  return r;
}

QgsFeatureRenderer* QgsSingleSymbolRenderer::createFromSld( QDomElement& element, QgsWkbTypes::GeometryType geomType )
{
  // XXX this renderer can handle only one Rule!

  // get the first Rule element
  QDomElement ruleElem = element.firstChildElement( "Rule" );
  if ( ruleElem.isNull() )
  {
    QgsDebugMsg( "no Rule elements found!" );
    return nullptr;
  }

  QString label, description;
  QgsSymbolLayerList layers;

  // retrieve the Rule element child nodes
  QDomElement childElem = ruleElem.firstChildElement();
  while ( !childElem.isNull() )
  {
    if ( childElem.localName() == "Name" )
    {
      // <se:Name> tag contains the rule identifier,
      // so prefer title tag for the label property value
      if ( label.isEmpty() )
        label = childElem.firstChild().nodeValue();
    }
    else if ( childElem.localName() == "Description" )
    {
      // <se:Description> can contains a title and an abstract
      QDomElement titleElem = childElem.firstChildElement( "Title" );
      if ( !titleElem.isNull() )
      {
        label = titleElem.firstChild().nodeValue();
      }

      QDomElement abstractElem = childElem.firstChildElement( "Abstract" );
      if ( !abstractElem.isNull() )
      {
        description = abstractElem.firstChild().nodeValue();
      }
    }
    else if ( childElem.localName() == "Abstract" )
    {
      // <sld:Abstract> (v1.0)
      description = childElem.firstChild().nodeValue();
    }
    else if ( childElem.localName() == "Title" )
    {
      // <sld:Title> (v1.0)
      label = childElem.firstChild().nodeValue();
    }
    else if ( childElem.localName().endsWith( "Symbolizer" ) )
    {
      // create symbol layers for this symbolizer
      QgsSymbolLayerUtils::createSymbolLayerListFromSld( childElem, geomType, layers );
    }

    childElem = childElem.nextSiblingElement();
  }

  if ( layers.isEmpty() )
    return nullptr;

  // now create the symbol
  QgsSymbol *symbol;
  switch ( geomType )
  {
    case QgsWkbTypes::LineGeometry:
      symbol = new QgsLineSymbol( layers );
      break;

    case QgsWkbTypes::PolygonGeometry:
      symbol = new QgsFillSymbol( layers );
      break;

    case QgsWkbTypes::PointGeometry:
      symbol = new QgsMarkerSymbol( layers );
      break;

    default:
      QgsDebugMsg( QString( "invalid geometry type: found %1" ).arg( geomType ) );
      return nullptr;
  }

  // and finally return the new renderer
  return new QgsSingleSymbolRenderer( symbol );
}

QDomElement QgsSingleSymbolRenderer::save( QDomDocument& doc )
{
  QDomElement rendererElem = doc.createElement( RENDERER_TAG_NAME );
  rendererElem.setAttribute( "type", "singleSymbol" );
  rendererElem.setAttribute( "symbollevels", ( mUsingSymbolLevels ? "1" : "0" ) );
  rendererElem.setAttribute( "forceraster", ( mForceRaster ? "1" : "0" ) );

  QgsSymbolMap symbols;
  symbols["0"] = mSymbol.data();
  QDomElement symbolsElem = QgsSymbolLayerUtils::saveSymbols( symbols, "symbols", doc );
  rendererElem.appendChild( symbolsElem );

  QDomElement rotationElem = doc.createElement( "rotation" );
  if ( mRotation.data() )
    rotationElem.setAttribute( "field", QgsSymbolLayerUtils::fieldOrExpressionFromExpression( mRotation.data() ) );
  rendererElem.appendChild( rotationElem );

  QDomElement sizeScaleElem = doc.createElement( "sizescale" );
  if ( mSizeScale.data() )
    sizeScaleElem.setAttribute( "field", QgsSymbolLayerUtils::fieldOrExpressionFromExpression( mSizeScale.data() ) );
  sizeScaleElem.setAttribute( "scalemethod", QgsSymbolLayerUtils::encodeScaleMethod( mScaleMethod ) );
  rendererElem.appendChild( sizeScaleElem );

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

QgsLegendSymbologyList QgsSingleSymbolRenderer::legendSymbologyItems( QSize iconSize )
{
  QgsLegendSymbologyList lst;
  if ( mSymbol.data() )
  {
    QPixmap pix = QgsSymbolLayerUtils::symbolPreviewPixmap( mSymbol.data(), iconSize );
    lst << qMakePair( QString(), pix );
  }
  return lst;
}

QgsLegendSymbolList QgsSingleSymbolRenderer::legendSymbolItems( double scaleDenominator, const QString& rule )
{
  Q_UNUSED( scaleDenominator );
  Q_UNUSED( rule );
  QgsLegendSymbolList lst;
  lst << qMakePair( QString(), mSymbol.data() );
  return lst;
}

QgsLegendSymbolListV2 QgsSingleSymbolRenderer::legendSymbolItemsV2() const
{
  QgsLegendSymbolListV2 lst;
  if ( mSymbol->type() == QgsSymbol::Marker )
  {
    const QgsMarkerSymbol * symbol = static_cast<const QgsMarkerSymbol *>( mSymbol.data() );
    QgsDataDefined sizeDD = symbol->dataDefinedSize();
    if ( sizeDD.isActive() && sizeDD.useExpression() )
    {
      QgsScaleExpression scaleExp( sizeDD.expressionString() );
      if ( scaleExp.type() != QgsScaleExpression::Unknown )
      {
        QgsLegendSymbolItem title( nullptr, scaleExp.baseExpression(), QString() );
        lst << title;
        Q_FOREACH ( double v, QgsSymbolLayerUtils::prettyBreaks( scaleExp.minValue(), scaleExp.maxValue(), 4 ) )
        {
          QgsLegendSymbolItem si( mSymbol.data(), QString::number( v ), QString() );
          QgsMarkerSymbol * s = static_cast<QgsMarkerSymbol *>( si.symbol() );
          s->setDataDefinedSize( 0 );
          s->setSize( scaleExp.size( v ) );
          lst << si;
        }
        return lst;
      }
    }
  }

  lst << QgsLegendSymbolItem( mSymbol.data(), QString(), QString() );
  return lst;
}

QSet< QString > QgsSingleSymbolRenderer::legendKeysForFeature( QgsFeature& feature, QgsRenderContext& context )
{
  Q_UNUSED( feature );
  Q_UNUSED( context );
  return QSet< QString >() << QString();
}

void QgsSingleSymbolRenderer::setLegendSymbolItem( const QString& key, QgsSymbol* symbol )
{
  Q_UNUSED( key );
  setSymbol( symbol );
}

QgsSingleSymbolRenderer* QgsSingleSymbolRenderer::convertFromRenderer( const QgsFeatureRenderer *renderer )
{
  QgsSingleSymbolRenderer* r = nullptr;
  if ( renderer->type() == "singleSymbol" )
  {
    r = dynamic_cast<QgsSingleSymbolRenderer*>( renderer->clone() );
  }
  else if ( renderer->type() == "pointDisplacement" )
  {
    const QgsPointDisplacementRenderer* pointDisplacementRenderer = dynamic_cast<const QgsPointDisplacementRenderer*>( renderer );
    if ( pointDisplacementRenderer )
      r = convertFromRenderer( pointDisplacementRenderer->embeddedRenderer() );
  }
  else if ( renderer->type() == "invertedPolygonRenderer" )
  {
    const QgsInvertedPolygonRenderer* invertedPolygonRenderer = dynamic_cast<const QgsInvertedPolygonRenderer*>( renderer );
    if ( invertedPolygonRenderer )
      r = convertFromRenderer( invertedPolygonRenderer->embeddedRenderer() );
  }

  if ( !r )
  {
    QgsRenderContext context;
    QgsSymbolList symbols = const_cast<QgsFeatureRenderer *>( renderer )->symbols( context );
    if ( !symbols.isEmpty() )
    {
      r = new QgsSingleSymbolRenderer( symbols.at( 0 )->clone() );
    }
  }

  if ( r )
  {
    r->setOrderBy( renderer->orderBy() );
    r->setOrderByEnabled( renderer->orderByEnabled() );
  }

  return r;
}
