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
#include "qgsapplication.h"
#include "qgscategorizedchunkloader_p.h"
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

void QgsCategorized3DRenderer::setClassAttribute( QString attributeName )
{
  mAttributeName = attributeName;
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
        QDomElement elemSymbol = catElem.firstChildElement( u"symbol"_s );
        if ( !elemSymbol.isNull() )
        {
          QString symbolType = elemSymbol.attribute( u"type"_s );
          symbol = QgsApplication::symbol3DRegistry()->createSymbol( symbolType );
          if ( symbol )
          {
            symbol->readXml( elemSymbol, context );
          }
        }

        mCategories.append( Qgs3DRendererCategory( value, symbol, render ) );
      }
      catElem = catElem.nextSiblingElement();
    }
  }
}
