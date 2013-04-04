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
    : QgsFeatureRendererV2( "singleSymbol" ), mRotationFieldIdx( -1 ), mSizeScaleFieldIdx( -1 ), mTempSymbol( NULL )
{
  Q_ASSERT( symbol );
  mSymbol = symbol;
}

QgsSingleSymbolRendererV2::~QgsSingleSymbolRendererV2()
{
  delete mSymbol;
}

QgsSymbolV2* QgsSingleSymbolRendererV2::symbolForFeature( QgsFeature& feature )
{
  if ( mRotationFieldIdx == -1 && mSizeScaleFieldIdx == -1 )
    return mSymbol;

  double rotation = 0;
  double sizeScale = 1;
  if ( mRotationFieldIdx != -1 )
  {
    rotation = feature.attribute( mRotationFieldIdx ).toDouble();
  }
  if ( mSizeScaleFieldIdx != -1 )
  {
    sizeScale = feature.attribute( mSizeScaleFieldIdx ).toDouble();
  }

  if ( mTempSymbol->type() == QgsSymbolV2::Marker )
  {
    QgsMarkerSymbolV2* markerSymbol = static_cast<QgsMarkerSymbolV2*>( mTempSymbol );
    if ( mRotationFieldIdx != -1 )
      markerSymbol->setAngle( rotation );
    if ( mSizeScaleFieldIdx != -1 )
      markerSymbol->setSize( sizeScale * mOrigSize );
    markerSymbol->setScaleMethod( mScaleMethod );
  }
  else if ( mTempSymbol->type() == QgsSymbolV2::Line )
  {
    QgsLineSymbolV2* lineSymbol = static_cast<QgsLineSymbolV2*>( mTempSymbol );
    if ( mSizeScaleFieldIdx != -1 )
      lineSymbol->setWidth( sizeScale * mOrigSize );
  }
  else if ( mTempSymbol->type() == QgsSymbolV2::Fill )
  {
    QgsFillSymbolV2* fillSymbol = static_cast<QgsFillSymbolV2*>( mTempSymbol );
    if ( mRotationFieldIdx != -1 )
      fillSymbol->setAngle( rotation );
  }

  return mTempSymbol;
}

void QgsSingleSymbolRendererV2::startRender( QgsRenderContext& context, const QgsVectorLayer *vlayer )
{
  if ( !mSymbol )
  {
    return;
  }
  mRotationFieldIdx  = mRotationField.isEmpty()  ? -1 : vlayer->fieldNameIndex( mRotationField );
  mSizeScaleFieldIdx = mSizeScaleField.isEmpty() ? -1 : vlayer->fieldNameIndex( mSizeScaleField );

  mSymbol->startRender( context, vlayer );

  if ( mRotationFieldIdx != -1 || mSizeScaleFieldIdx != -1 )
  {
    // we are going to need a temporary symbol
    mTempSymbol = mSymbol->clone();

    int hints = 0;
    if ( mRotationFieldIdx != -1 )
      hints |= QgsSymbolV2::DataDefinedRotation;
    if ( mSizeScaleFieldIdx != -1 )
      hints |= QgsSymbolV2::DataDefinedSizeScale;
    mTempSymbol->setRenderHints( hints );

    mTempSymbol->startRender( context, vlayer );

    if ( mSymbol->type() == QgsSymbolV2::Marker )
    {
      mOrigSize = static_cast<QgsMarkerSymbolV2*>( mSymbol )->size();
    }
    else if ( mSymbol->type() == QgsSymbolV2::Line )
    {
      mOrigSize = static_cast<QgsLineSymbolV2*>( mSymbol )->width();
    }
    else
    {
      mOrigSize = 0;
    }
  }
}

void QgsSingleSymbolRendererV2::stopRender( QgsRenderContext& context )
{
  if ( !mSymbol )
  {
    return;
  }
  mSymbol->stopRender( context );

  if ( mRotationFieldIdx != -1 || mSizeScaleFieldIdx != -1 )
  {
    // we are going to need a temporary symbol
    mTempSymbol->stopRender( context );
    delete mTempSymbol;
    mTempSymbol = NULL;
  }
}

QList<QString> QgsSingleSymbolRendererV2::usedAttributes()
{
  QSet<QString> attributes;
  if ( mSymbol )
  {
    attributes.unite( mSymbol->usedAttributes() );
  }
  if ( !mRotationField.isEmpty() )
  {
    attributes.insert( mRotationField );
  }
  if ( !mSizeScaleField.isEmpty() )
  {
    attributes.insert( mSizeScaleField );
  }
  return attributes.toList();
}

QgsSymbolV2* QgsSingleSymbolRendererV2::symbol() const
{
  return mSymbol;
}

void QgsSingleSymbolRendererV2::setSymbol( QgsSymbolV2* s )
{
  Q_ASSERT( s );
  delete mSymbol;
  mSymbol = s;
}

void QgsSingleSymbolRendererV2::setScaleMethod( QgsSymbolV2::ScaleMethod scaleMethod )
{
  mScaleMethod = scaleMethod;
  setScaleMethodToSymbol( mSymbol, scaleMethod );
}

QString QgsSingleSymbolRendererV2::dump()
{
  if ( mSymbol )
  {
    return QString( "SINGLE: %1" ).arg( mSymbol->dump() );
  }
  else
  {
    return "";
  }
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
  if ( !mRotationField.isEmpty() )
    props[ "angle" ] = QString( mRotationField ).append( "\"" ).prepend( "\"" );
  if ( !mSizeScaleField.isEmpty() )
    props[ "scale" ] = QString( mSizeScaleField ).append( "\"" ).prepend( "\"" );

  QDomElement ruleElem = doc.createElement( "se:Rule" );
  element.appendChild( ruleElem );

  QDomElement nameElem = doc.createElement( "se:Name" );
  nameElem.appendChild( doc.createTextNode( "Single symbol" ) );
  ruleElem.appendChild( nameElem );

  mSymbol->toSld( doc, ruleElem, props );
}

QgsSymbolV2List QgsSingleSymbolRendererV2::symbols()
{
  QgsSymbolV2List lst;
  lst.append( mSymbol );
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
    r->setRotationField( rotationElem.attribute( "field" ) );

  QDomElement sizeScaleElem = element.firstChildElement( "sizescale" );
  if ( !sizeScaleElem.isNull() )
  {
    r->setSizeScaleField( sizeScaleElem.attribute( "field" ) );
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
  symbols["0"] = mSymbol;
  QDomElement symbolsElem = QgsSymbolLayerV2Utils::saveSymbols( symbols, "symbols", doc );
  rendererElem.appendChild( symbolsElem );

  QDomElement rotationElem = doc.createElement( "rotation" );
  rotationElem.setAttribute( "field", mRotationField );
  rendererElem.appendChild( rotationElem );

  QDomElement sizeScaleElem = doc.createElement( "sizescale" );
  sizeScaleElem.setAttribute( "field", mSizeScaleField );
  sizeScaleElem.setAttribute( "scalemethod", QgsSymbolLayerV2Utils::encodeScaleMethod( mScaleMethod ) );
  rendererElem.appendChild( sizeScaleElem );

  return rendererElem;
}

QgsLegendSymbologyList QgsSingleSymbolRendererV2::legendSymbologyItems( QSize iconSize )
{
  QgsLegendSymbologyList lst;
  if ( mSymbol )
  {
    QPixmap pix = QgsSymbolLayerV2Utils::symbolPreviewPixmap( mSymbol, iconSize );
    lst << qMakePair( QString(), pix );
  }
  return lst;
}

QgsLegendSymbolList QgsSingleSymbolRendererV2::legendSymbolItems()
{
  QgsLegendSymbolList lst;
  lst << qMakePair( QString(), mSymbol );
  return lst;
}
