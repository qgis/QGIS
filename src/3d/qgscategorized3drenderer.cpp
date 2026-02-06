/***************************************************************************
  qgscategorized3drenderer.cpp
  --------------------------------------
  Date                 : November 2025
  Copyright            : (C) 2025 by Jean Felder
  Email                : jean dot felder at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscategorized3drenderer.h"

#include "qgs3dmapsettings.h"
#include "qgs3dsymbolregistry.h"
#include "qgs3dsymbolutils.h"
#include "qgsabstract3dsymbol.h"
#include "qgsapplication.h"
#include "qgscategorizedchunkloader_p.h"
#include "qgscolorrampimpl.h"
#include "qgssymbollayerutils.h"
#include "qgsvectorlayer.h"
#include "qgsxmlutils.h"

#include <QString>

using namespace Qt::StringLiterals;

QgsCategorized3DRendererMetadata::QgsCategorized3DRendererMetadata()
  : Qgs3DRendererAbstractMetadata( u"categorized"_s )
{
}

QgsAbstract3DRenderer *QgsCategorized3DRendererMetadata::createRenderer( QDomElement &elem, const QgsReadWriteContext &context )
{
  QgsCategorized3DRenderer *renderer = new QgsCategorized3DRenderer();
  renderer->readXml( elem, context );
  return renderer;
}


// ---------


////////////////////

Qgs3DRendererCategory::Qgs3DRendererCategory( const QVariant &value, QgsAbstract3DSymbol *symbol, bool render )
  : mValue( value )
  , mSymbol( symbol )
  , mRender( render )
{
}

Qgs3DRendererCategory::Qgs3DRendererCategory( const Qgs3DRendererCategory &other )
  : Qgs3DRendererCategory( other.mValue, other.mSymbol ? other.mSymbol->clone() : nullptr, other.mRender )
{
}

Qgs3DRendererCategory &Qgs3DRendererCategory::operator=( const Qgs3DRendererCategory &other )
{
  if ( this != &other )
  {
    mValue = other.mValue;
    mSymbol = other.mSymbol ? std::unique_ptr<QgsAbstract3DSymbol>( other.mSymbol->clone() ) : nullptr;
    mRender = other.mRender;
  }

  return *this;
}

void Qgs3DRendererCategory::setRenderState( bool render )
{
  mRender = render;
}

void Qgs3DRendererCategory::setValue( const QVariant &value )
{
  mValue = value;
}

void Qgs3DRendererCategory::setSymbol( QgsAbstract3DSymbol *symbol )
{
  if ( mSymbol.get() != symbol )
  {
    mSymbol.reset( symbol );
  }
}

// ---------


////////////////////


QgsCategorized3DRenderer::QgsCategorized3DRenderer( const QString &attributeName, const Qgs3DCategoryList &categories )
  : mAttributeName( attributeName )
  , mCategories( categories )
{
}

QgsCategorized3DRenderer *QgsCategorized3DRenderer::clone() const
{
  QgsCategorized3DRenderer *renderer = new QgsCategorized3DRenderer( mAttributeName, mCategories );

  if ( mSourceSymbol )
  {
    renderer->setSourceSymbol( mSourceSymbol->clone() );
  }

  if ( mSourceColorRamp )
  {
    renderer->setSourceColorRamp( mSourceColorRamp->clone() );
  }

  copyBaseProperties( renderer );
  return renderer;
}

Qt3DCore::QEntity *QgsCategorized3DRenderer::createEntity( Qgs3DMapSettings *mapSettings ) const
{
  QgsVectorLayer *vectorLayer = layer();

  if ( !vectorLayer )
  {
    return nullptr;
  }

  // we start with a maximal z range because we can't know this upfront. There's too many
  // factors to consider eg vertex z data, terrain heights, data defined offsets and extrusion heights,...
  // This range will be refined after populating the nodes to the actual z range of the generated chunks nodes.
  // Assuming the vertical height is in meter, then it's extremely unlikely that a real vertical
  // height will exceed this amount!
  constexpr double MINIMUM_VECTOR_Z_ESTIMATE = -100000;
  constexpr double MAXIMUM_VECTOR_Z_ESTIMATE = 100000;

  return new QgsCategorizedChunkedEntity( mapSettings, vectorLayer, MINIMUM_VECTOR_Z_ESTIMATE, MAXIMUM_VECTOR_Z_ESTIMATE, tilingSettings(), this );
}

void QgsCategorized3DRenderer::updateSymbols( QgsAbstract3DSymbol *symbol )
{
  int i = 0;
  for ( const Qgs3DRendererCategory &cat : mCategories )
  {
    QgsAbstract3DSymbol *newSymbol = symbol->clone();
    Qgs3DSymbolUtils::copyVectorSymbolMaterial( cat.symbol(), newSymbol );
    updateCategorySymbol( i, newSymbol );
    ++i;
  }
  setSourceSymbol( symbol->clone() );
}

void QgsCategorized3DRenderer::setClassAttribute( QString attributeName )
{
  mAttributeName = attributeName;
}

QgsAbstract3DSymbol *QgsCategorized3DRenderer::sourceSymbol()
{
  return mSourceSymbol.get();
}

const QgsAbstract3DSymbol *QgsCategorized3DRenderer::sourceSymbol() const
{
  return mSourceSymbol.get();
}

void QgsCategorized3DRenderer::setSourceSymbol( QgsAbstract3DSymbol *symbol )
{
  mSourceSymbol.reset( symbol );
}

QgsColorRamp *QgsCategorized3DRenderer::sourceColorRamp()
{
  return mSourceColorRamp.get();
}

const QgsColorRamp *QgsCategorized3DRenderer::sourceColorRamp() const
{
  return mSourceColorRamp.get();
}

void QgsCategorized3DRenderer::setSourceColorRamp( QgsColorRamp *ramp )
{
  mSourceColorRamp.reset( ramp );
}

void QgsCategorized3DRenderer::updateColorRamp( QgsColorRamp *ramp )
{
  setSourceColorRamp( ramp );
  const double num = static_cast<double>( mCategories.count() - 1 );
  double count = 0.;

  QgsRandomColorRamp *randomRamp = dynamic_cast<QgsRandomColorRamp *>( ramp );
  if ( randomRamp )
  {
    //ramp is a random colors ramp, so inform it of the total number of required colors
    //this allows the ramp to pregenerate a set of visually distinctive colors
    randomRamp->setTotalColorCount( static_cast<int>( mCategories.count() ) );
  }

  for ( const Qgs3DRendererCategory &cat : mCategories )
  {
    Qgs3DSymbolUtils::setVectorSymbolBaseColor( cat.symbol(), mSourceColorRamp->color( count / num ) );
    count += 1.;
  }
}

bool QgsCategorized3DRenderer::updateCategoryValue( int catIndex, const QVariant &value )
{
  if ( catIndex < 0 || catIndex >= mCategories.size() )
  {
    return false;
  }

  mCategories[catIndex].setValue( value );
  return true;
}

bool QgsCategorized3DRenderer::updateCategorySymbol( int catIndex, QgsAbstract3DSymbol *symbol )
{
  if ( catIndex < 0 || catIndex >= mCategories.size() )
  {
    return false;
  }

  mCategories[catIndex].setSymbol( symbol );
  return true;
}

bool QgsCategorized3DRenderer::updateCategoryRenderState( int catIndex, bool render )
{
  if ( catIndex < 0 || catIndex >= mCategories.size() )
  {
    return false;
  }

  mCategories[catIndex].setRenderState( render );
  return true;
}

void QgsCategorized3DRenderer::addCategory( const Qgs3DRendererCategory &category )
{
  mCategories.append( category );
}

bool QgsCategorized3DRenderer::deleteCategory( int catIndex )
{
  if ( catIndex < 0 || catIndex >= mCategories.size() )
  {
    return false;
  }

  mCategories.removeAt( catIndex );
  return true;
}

void QgsCategorized3DRenderer::deleteAllCategories()
{
  mCategories.clear();
}

bool QgsCategorized3DRenderer::moveCategory( int from, int to )
{
  if ( from < 0 || from >= mCategories.size() || to < 0 || to >= mCategories.size() )
  {
    return false;
  }

  mCategories.move( from, to );
  return true;
}

void QgsCategorized3DRenderer::sortByValue( Qt::SortOrder order )
{
  std::sort( mCategories.begin(), mCategories.end(), [order]( const Qgs3DRendererCategory &cat1, const Qgs3DRendererCategory &cat2 ) {
    bool less = qgsVariantLessThan( cat1.value(), cat2.value() );
    return ( order == Qt::AscendingOrder ) ? less : !less;
  } );
}

void QgsCategorized3DRenderer::writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const
{
  QDomDocument doc = elem.ownerDocument();

  writeXmlBaseProperties( elem, context );

  elem.setAttribute( u"attribute"_s, mAttributeName );

  // save categories
  if ( !mCategories.isEmpty() )
  {
    QDomElement catsElem = doc.createElement( u"categories"_s );
    for ( const Qgs3DRendererCategory &category : mCategories )
    {
      QDomElement catElem = doc.createElement( u"category"_s );
      QDomElement valueElem = QgsXmlUtils::writeVariant( category.value(), doc );
      catElem.appendChild( valueElem );
      catElem.setAttribute( u"render"_s, category.renderState() ? 1 : 0 );

      QDomElement symbolElem = doc.createElement( u"symbol"_s );
      symbolElem.setAttribute( u"type"_s, category.symbol()->type() );
      category.symbol()->writeXml( symbolElem, context );
      catElem.appendChild( symbolElem );

      catsElem.appendChild( catElem );
    }
    elem.appendChild( catsElem );
  }

  // save source symbol
  if ( mSourceSymbol )
  {
    QDomElement sourceSymbolElem = doc.createElement( u"source-symbol"_s );
    sourceSymbolElem.setAttribute( u"type"_s, mSourceSymbol->type() );
    mSourceSymbol->writeXml( sourceSymbolElem, context );
    elem.appendChild( sourceSymbolElem );
  }

  // save source color ramp
  if ( mSourceColorRamp )
  {
    QDomElement colorRampElem = QgsSymbolLayerUtils::saveColorRamp( u"[source]"_s, mSourceColorRamp.get(), doc );
    elem.appendChild( colorRampElem );
  }
}

void QgsCategorized3DRenderer::readXml( const QDomElement &elem, const QgsReadWriteContext &context )
{
  readXmlBaseProperties( elem, context );

  mAttributeName = elem.attribute( u"attribute"_s );

  // load categories
  QDomElement catsElem = elem.firstChildElement( u"categories"_s );
  if ( !catsElem.isNull() )
  {
    QDomElement catElem = catsElem.firstChildElement();
    while ( !catElem.isNull() )
    {
      if ( catElem.tagName() == "category"_L1 )
      {
        const QVariant value = QgsXmlUtils::readVariant( catElem.firstChildElement( u"Option"_s ) );
        const bool render = static_cast<bool>( catElem.attribute( u"render"_s, u"0"_s ).toInt() );

        QgsAbstract3DSymbol *symbol = nullptr;
        QDomElement symbolElem = catElem.firstChildElement( u"symbol"_s );
        if ( !symbolElem.isNull() )
        {
          QString symbolType = symbolElem.attribute( u"type"_s );
          symbol = QgsApplication::symbol3DRegistry()->createSymbol( symbolType );
          if ( symbol )
          {
            symbol->readXml( symbolElem, context );
          }
        }

        mCategories.append( Qgs3DRendererCategory( value, symbol, render ) );
      }
      catElem = catElem.nextSiblingElement();
    }
  }

  // load source symbol
  QDomElement sourceSymbolElem = elem.firstChildElement( u"source-symbol"_s );
  if ( !sourceSymbolElem.isNull() )
  {
    QString symbolType = sourceSymbolElem.attribute( u"type"_s );
    QgsAbstract3DSymbol *sourceSymbol = QgsApplication::symbol3DRegistry()->createSymbol( symbolType );
    if ( sourceSymbol )
    {
      sourceSymbol->readXml( sourceSymbolElem, context );
      mSourceSymbol.reset( sourceSymbol );
    }
  }

  // load source color ramp
  QDomElement sourceColorRampElem = elem.firstChildElement( u"colorramp"_s );
  if ( !sourceColorRampElem.isNull() && sourceColorRampElem.attribute( u"name"_s ) == "[source]"_L1 )
  {
    mSourceColorRamp = QgsSymbolLayerUtils::loadColorRamp( sourceColorRampElem );
  }
}

Qgs3DCategoryList QgsCategorized3DRenderer::createCategories( const QList<QVariant> &values, const QgsAbstract3DSymbol *symbol, QgsVectorLayer *layer, const QString &attributeName )
{
  Qgs3DCategoryList cats;
  QVariantList vals = values;
  // sort the categories first
  QgsSymbolLayerUtils::sortVariantList( vals, Qt::AscendingOrder );

  if ( layer && !attributeName.isNull() )
  {
    const QgsFields fields = layer->fields();
    for ( const QVariant &value : vals )
    {
      if ( !QgsVariantUtils::isNull( value ) )
      {
        QgsAbstract3DSymbol *newSymbol = symbol->clone();
        cats.append( Qgs3DRendererCategory( value, newSymbol, true ) );
      }
    }
  }

  // add null (default) value
  QgsAbstract3DSymbol *newSymbol = symbol->clone();
  cats.append( Qgs3DRendererCategory( QVariant(), newSymbol, true ) );

  return cats;
}
