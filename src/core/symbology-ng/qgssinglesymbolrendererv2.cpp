/***************************************************************************
    qgssinglesymbolrendererv2.cpp
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

#include "qgssinglesymbolrendererv2.h"

#include "qgssymbolv2.h"
#include "qgssymbollayerv2utils.h"

#include "qgslogger.h"
#include "qgsfeature.h"
#include "qgsvectorlayer.h"
#include "qgssymbollayerv2.h"

#include <QDomDocument>
#include <QDomElement>

QgsSingleSymbolRendererV2::QgsSingleSymbolRendererV2( QgsSymbolV2* symbol )
    : QgsFeatureRendererV2( "singleSymbol" )
    , mSymbol( symbol )
    , mScaleMethod( DEFAULT_SCALE_METHOD )
{
  Q_ASSERT( symbol );
}

// we need to clone symbol
QgsSingleSymbolRendererV2::QgsSingleSymbolRendererV2( const QgsSingleSymbolRendererV2 & src )
    : QgsFeatureRendererV2( "singleSymbol" )
    , mSymbol( src.mSymbol.get() ? src.mSymbol->clone() : NULL )
    , mRotation( src.mRotation.get() ? new QgsExpression( src.mRotation->expression() ) : NULL )
    , mSizeScale( src.mSizeScale.get() ?  new QgsExpression( src.mSizeScale->expression() ) : NULL )
    , mScaleMethod( src.mScaleMethod )
    , mTempSymbol( src.mTempSymbol.get() ? src.mTempSymbol->clone() : NULL )
{
}

// this is a copy + swap idiom implementation
// the copy is done with the 'pass by value'
QgsSingleSymbolRendererV2 & QgsSingleSymbolRendererV2::operator=( QgsSingleSymbolRendererV2 other )
{
  swap( other );
  return *this;
}

void QgsSingleSymbolRendererV2::swap( QgsSingleSymbolRendererV2 & other )
{
  std::swap( mSymbol, other.mSymbol );
  std::swap( mRotation, other.mRotation );
  std::swap( mSizeScale, other.mSizeScale );
  std::swap( mScaleMethod, other.mScaleMethod );
  std::swap( mTempSymbol, other.mTempSymbol );
  std::swap( mOrigSize, other.mOrigSize );
}

QgsSingleSymbolRendererV2::~QgsSingleSymbolRendererV2()
{
}

QgsSymbolV2* QgsSingleSymbolRendererV2::symbolForFeature( QgsFeature& feature )
{
  if ( !mRotation.get() && !mSizeScale.get() ) return mSymbol.get();

  const double rotation = mRotation.get() ? mRotation->evaluate( feature ).toDouble() : 0;
  const double sizeScale = mSizeScale.get() ? mSizeScale->evaluate( feature ).toDouble() : 1.;

  if ( mTempSymbol->type() == QgsSymbolV2::Marker )
  {
    QgsMarkerSymbolV2* markerSymbol = static_cast<QgsMarkerSymbolV2*>( mTempSymbol.get() );
    markerSymbol->setAngle( rotation );
    markerSymbol->setSize( sizeScale * mOrigSize );
    markerSymbol->setScaleMethod( mScaleMethod );
  }
  else if ( mTempSymbol->type() == QgsSymbolV2::Line )
  {
    QgsLineSymbolV2* lineSymbol = static_cast<QgsLineSymbolV2*>( mTempSymbol.get() );
    lineSymbol->setWidth( sizeScale * mOrigSize );
  }
  else if ( mTempSymbol->type() == QgsSymbolV2::Fill )
  {
    QgsFillSymbolV2* fillSymbol = static_cast<QgsFillSymbolV2*>( mTempSymbol.get() );
    fillSymbol->setAngle( rotation );
  }

  return mTempSymbol.get();
}

void QgsSingleSymbolRendererV2::startRender( QgsRenderContext& context, const QgsVectorLayer *vlayer )
{
  if ( !mSymbol.get() ) return;

  mSymbol->startRender( context, vlayer );

  if ( mRotation.get() || mSizeScale.get() )
  {
    // we are going to need a temporary symbol
    mTempSymbol.reset( mSymbol->clone() );

    int hints = 0;
    if ( mRotation.get() )
      hints |= QgsSymbolV2::DataDefinedRotation;
    if ( mSizeScale.get() )
      hints |= QgsSymbolV2::DataDefinedSizeScale;
    mTempSymbol->setRenderHints( hints );

    mTempSymbol->startRender( context, vlayer );

    if ( mSymbol->type() == QgsSymbolV2::Marker )
    {
      mOrigSize = static_cast<QgsMarkerSymbolV2*>( mSymbol.get() )->size();
    }
    else if ( mSymbol->type() == QgsSymbolV2::Line )
    {
      mOrigSize = static_cast<QgsLineSymbolV2*>( mSymbol.get() )->width();
    }
    else
    {
      mOrigSize = 0;
    }
  }
}

void QgsSingleSymbolRendererV2::stopRender( QgsRenderContext& context )
{
  if ( !mSymbol.get() ) return;

  mSymbol->stopRender( context );

  if ( mRotation.get() || mSizeScale.get() )
  {
    // we are going to need a temporary symbol
    mTempSymbol->stopRender( context );
    mTempSymbol.reset();
  }
}

QList<QString> QgsSingleSymbolRendererV2::usedAttributes()
{
  QSet<QString> attributes;
  if ( mSymbol.get() ) attributes.unite( mSymbol->usedAttributes() );
  if ( mRotation.get() ) attributes.unite( mRotation->referencedColumns().toSet() );
  if ( mSizeScale.get() ) attributes.unite( mSizeScale->referencedColumns().toSet() );
  return attributes.toList();
}

QgsSymbolV2* QgsSingleSymbolRendererV2::symbol() const
{
  return mSymbol.get();
}

void QgsSingleSymbolRendererV2::setSymbol( QgsSymbolV2* s )
{
  Q_ASSERT( s );
  mSymbol.reset( s );
}

void QgsSingleSymbolRendererV2::setScaleMethod( QgsSymbolV2::ScaleMethod scaleMethod )
{
  mScaleMethod = scaleMethod;
  setScaleMethodToSymbol( mSymbol.get(), scaleMethod );
}

QString QgsSingleSymbolRendererV2::dump() const
{
  return mSymbol.get() ? QString( "SINGLE: %1" ).arg( mSymbol->dump() ) : "" ;
}

QgsFeatureRendererV2* QgsSingleSymbolRendererV2::clone()
{
  QgsSingleSymbolRendererV2* r = new QgsSingleSymbolRendererV2( mSymbol->clone() );
  r->setUsingSymbolLevels( usingSymbolLevels() );
  r->setRotationField( rotationField() );
  r->setSizeScaleField( sizeScaleField() );
  r->setScaleMethod( scaleMethod() );
  return r;
}

void QgsSingleSymbolRendererV2::toSld( QDomDocument& doc, QDomElement &element ) const
{
  QgsStringMap props;
  if ( mRotation.get() )
    props[ "angle" ] = qgsXmlEncode( mRotation->expression() ).append( "\"" ).prepend( "\"" );
  if ( mSizeScale.get() )
    props[ "scale" ] = qgsXmlEncode( mSizeScale->expression() ).append( "\"" ).prepend( "\"" );

  QDomElement ruleElem = doc.createElement( "se:Rule" );
  element.appendChild( ruleElem );

  QDomElement nameElem = doc.createElement( "se:Name" );
  nameElem.appendChild( doc.createTextNode( "Single symbol" ) );
  ruleElem.appendChild( nameElem );

  if ( mSymbol.get() ) mSymbol->toSld( doc, ruleElem, props );
}

QgsSymbolV2List QgsSingleSymbolRendererV2::symbols()
{
  QgsSymbolV2List lst;
  lst.append( mSymbol.get() );
  return lst;
}

QgsFeatureRendererV2* QgsSingleSymbolRendererV2::create( QDomElement& element )
{
  QDomElement symbolsElem = element.firstChildElement( "symbols" );
  if ( symbolsElem.isNull() )
    return NULL;

  QgsSymbolV2Map symbolMap = QgsSymbolLayerV2Utils::loadSymbols( symbolsElem );

  if ( !symbolMap.contains( "0" ) )
    return NULL;

  QgsSingleSymbolRendererV2* r = new QgsSingleSymbolRendererV2( symbolMap.take( "0" ) );

  // delete symbols if there are any more
  QgsSymbolLayerV2Utils::clearSymbolMap( symbolMap );

  QDomElement rotationElem = element.firstChildElement( "rotation" );
  if ( !rotationElem.isNull() )
    r->setRotationField( qgsXmlDecode( rotationElem.attribute( "field" ) ) );

  QDomElement sizeScaleElem = element.firstChildElement( "sizescale" );
  if ( !sizeScaleElem.isNull() )
  {
    r->setSizeScaleField( qgsXmlDecode( sizeScaleElem.attribute( "field" ) ) );
    r->setScaleMethod( QgsSymbolLayerV2Utils::decodeScaleMethod( sizeScaleElem.attribute( "scalemethod" ) ) );
  }

  // TODO: symbol levels
  return r;
}

QgsFeatureRendererV2* QgsSingleSymbolRendererV2::createFromSld( QDomElement& element, QGis::GeometryType geomType )
{
  // XXX this renderer can handle only one Rule!

  // get the first Rule element
  QDomElement ruleElem = element.firstChildElement( "Rule" );
  if ( ruleElem.isNull() )
  {
    QgsDebugMsg( "no Rule elements found!" );
    return NULL;
  }

  QString label, description;
  QgsSymbolLayerV2List layers;

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
      QgsSymbolLayerV2Utils::createSymbolLayerV2ListFromSld( childElem, geomType, layers );
    }

    childElem = childElem.nextSiblingElement();
  }

  if ( layers.size() == 0 )
    return NULL;

  // now create the symbol
  QgsSymbolV2 *symbol;
  switch ( geomType )
  {
    case QGis::Line:
      symbol = new QgsLineSymbolV2( layers );
      break;

    case QGis::Polygon:
      symbol = new QgsFillSymbolV2( layers );
      break;

    case QGis::Point:
      symbol = new QgsMarkerSymbolV2( layers );
      break;

    default:
      QgsDebugMsg( QString( "invalid geometry type: found %1" ).arg( geomType ) );
      return NULL;
  }

  // and finally return the new renderer
  return new QgsSingleSymbolRendererV2( symbol );
}

QDomElement QgsSingleSymbolRendererV2::save( QDomDocument& doc )
{
  QDomElement rendererElem = doc.createElement( RENDERER_TAG_NAME );
  rendererElem.setAttribute( "type", "singleSymbol" );
  rendererElem.setAttribute( "symbollevels", ( mUsingSymbolLevels ? "1" : "0" ) );

  QgsSymbolV2Map symbols;
  symbols["0"] = mSymbol.get();
  QDomElement symbolsElem = QgsSymbolLayerV2Utils::saveSymbols( symbols, "symbols", doc );
  rendererElem.appendChild( symbolsElem );

  QDomElement rotationElem = doc.createElement( "rotation" );
  if ( mRotation.get() )
    rotationElem.setAttribute( "field", qgsXmlEncode( mRotation->expression() ) );
  rendererElem.appendChild( rotationElem );

  QDomElement sizeScaleElem = doc.createElement( "sizescale" );
  if ( mSizeScale.get() )
    sizeScaleElem.setAttribute( "field", qgsXmlEncode( mSizeScale->expression() ) );
  sizeScaleElem.setAttribute( "scalemethod", QgsSymbolLayerV2Utils::encodeScaleMethod( mScaleMethod ) );
  rendererElem.appendChild( sizeScaleElem );

  return rendererElem;
}

QgsLegendSymbologyList QgsSingleSymbolRendererV2::legendSymbologyItems( QSize iconSize )
{
  QgsLegendSymbologyList lst;
  if ( mSymbol.get() )
  {
    QPixmap pix = QgsSymbolLayerV2Utils::symbolPreviewPixmap( mSymbol.get(), iconSize );
    lst << qMakePair( QString(), pix );
  }
  return lst;
}

QgsLegendSymbolList QgsSingleSymbolRendererV2::legendSymbolItems( double scaleDenominator, QString rule )
{
  Q_UNUSED( scaleDenominator );
  Q_UNUSED( rule );
  QgsLegendSymbolList lst;
  lst << qMakePair( QString(), mSymbol.get() );
  return lst;
}
