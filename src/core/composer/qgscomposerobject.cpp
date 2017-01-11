/***************************************************************************
                            qgscomposerobject.cpp
                             -------------------
    begin                : July 2014
    copyright            : (C) 2014 by Nyall Dawson,Radim Blazek
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QPainter>

#include "qgscomposition.h"
#include "qgscomposerutils.h"
#include "qgscomposerobject.h"
#include "qgsproject.h"
#include "qgsvectorlayer.h"

const QgsPropertyDefinition QgsComposerObject::sPropertyNameMap
{
  { QgsComposerObject::TestProperty, "dataDefinedProperty" },
  { QgsComposerObject::PresetPaperSize, "dataDefinedPaperSize" },
  { QgsComposerObject::PaperWidth, "dataDefinedPaperWidth"},
  { QgsComposerObject::PaperHeight, "dataDefinedPaperHeight" },
  { QgsComposerObject::NumPages, "dataDefinedNumPages" },
  { QgsComposerObject::PaperOrientation, "dataDefinedPaperOrientation" },
  { QgsComposerObject::PageNumber, "dataDefinedPageNumber" },
  { QgsComposerObject::PositionX, "dataDefinedPositionX" },
  { QgsComposerObject::PositionY, "dataDefinedPositionY" },
  { QgsComposerObject::ItemWidth, "dataDefinedWidth" },
  { QgsComposerObject::ItemHeight, "dataDefinedHeight" },
  { QgsComposerObject::ItemRotation, "dataDefinedRotation" },
  { QgsComposerObject::Transparency, "dataDefinedTransparency" },
  { QgsComposerObject::BlendMode, "dataDefinedBlendMode" },
  { QgsComposerObject::ExcludeFromExports, "dataDefinedExcludeExports"},
  { QgsComposerObject::MapRotation, "dataDefinedMapRotation" },
  { QgsComposerObject::MapScale, "dataDefinedMapScale" },
  { QgsComposerObject::MapXMin, "dataDefinedMapXMin" },
  { QgsComposerObject::MapYMin, "dataDefinedMapYMin" },
  { QgsComposerObject::MapXMax, "dataDefinedMapXMax" },
  { QgsComposerObject::MapYMax, "dataDefinedMapYMax" },
  { QgsComposerObject::MapAtlasMargin, "dataDefinedMapAtlasMargin" },
  { QgsComposerObject::MapLayers, "dataDefinedMapLayers" },
  { QgsComposerObject::MapStylePreset, "dataDefinedMapStylePreset" },
  { QgsComposerObject::PictureSource, "dataDefinedSource" },
  { QgsComposerObject::SourceUrl, "dataDefinedSourceUrl" },
  { QgsComposerObject::PresetPaperSize, "dataDefinedPaperSize" },
  { QgsComposerObject::PaperWidth, "dataDefinedPaperWidth" },
  { QgsComposerObject::PaperHeight, "dataDefinedPaperHeight" },
  { QgsComposerObject::NumPages, "dataDefinedNumPages" },
  { QgsComposerObject::PaperOrientation, "dataDefinedPaperOrientation" },
};


QgsComposerObject::QgsComposerObject( QgsComposition* composition )
    : QObject( nullptr )
    , mComposition( composition )
{

  // data defined strings

  if ( mComposition )
  {
    //connect to atlas toggling on/off and coverage layer and feature changes
    //to update data defined values
    connect( &mComposition->atlasComposition(), &QgsAtlasComposition::toggled, this, [this] { refreshDataDefinedProperty(); } );
    connect( &mComposition->atlasComposition(), &QgsAtlasComposition::coverageLayerChanged, this, [this] { refreshDataDefinedProperty(); } );
    connect( &mComposition->atlasComposition(), &QgsAtlasComposition::featureChanged, this, [this] { refreshDataDefinedProperty(); } );
    //also, refreshing composition triggers a recalculation of data defined properties
    connect( mComposition, &QgsComposition::refreshItemsTriggered, this, [this] { refreshDataDefinedProperty(); } );

    //toggling atlas or changing coverage layer requires data defined expressions to be reprepared
    connect( &mComposition->atlasComposition(), &QgsAtlasComposition::toggled, this, [this] { refreshDataDefinedProperty(); } );
    connect( &mComposition->atlasComposition(), &QgsAtlasComposition::coverageLayerChanged, this, [this] { refreshDataDefinedProperty(); } );
  }

}

bool QgsComposerObject::writeXml( QDomElement &elem, QDomDocument &doc ) const
{
  if ( elem.isNull() )
  {
    return false;
  }

  QDomElement ddPropsElement = doc.createElement( QStringLiteral( "dataDefinedProperties" ) );
  mProperties.writeXml( ddPropsElement, doc, sPropertyNameMap );
  elem.appendChild( ddPropsElement );

  //custom properties
  mCustomProperties.writeXml( elem, doc );

  return true;
}

bool QgsComposerObject::readXml( const QDomElement &itemElem, const QDomDocument &doc )
{
  Q_UNUSED( doc );
  if ( itemElem.isNull() )
  {
    return false;
  }

  //old (pre 3.0) data defined properties
  QgsComposerUtils::readOldDataDefinedPropertyMap( itemElem, mProperties );

  QDomNode propsNode = itemElem.namedItem( QStringLiteral( "dataDefinedProperties" ) );
  if ( !propsNode.isNull() )
  {
    mProperties.readXml( propsNode.toElement(), doc, sPropertyNameMap );
  }

  //custom properties
  mCustomProperties.readXml( itemElem );

  return true;
}

void QgsComposerObject::repaint()
{
  //nothing to do in base class for now
}

void QgsComposerObject::refreshDataDefinedProperty( const DataDefinedProperty property, const QgsExpressionContext *context )
{
  Q_UNUSED( property );
  Q_UNUSED( context );

  //nothing to do in base class for now
}

void QgsComposerObject::prepareProperties() const
{
  QgsExpressionContext context = createExpressionContext();
  mProperties.prepare( context );
}

void QgsComposerObject::setCustomProperty( const QString& key, const QVariant& value )
{
  mCustomProperties.setValue( key, value );
}

QVariant QgsComposerObject::customProperty( const QString& key, const QVariant& defaultValue ) const
{
  return mCustomProperties.value( key, defaultValue );
}

void QgsComposerObject::removeCustomProperty( const QString& key )
{
  mCustomProperties.remove( key );
}

QStringList QgsComposerObject::customProperties() const
{
  return mCustomProperties.keys();
}

QgsExpressionContext QgsComposerObject::createExpressionContext() const
{
  if ( mComposition )
  {
    return mComposition->createExpressionContext();
  }
  else
  {
    return QgsExpressionContext() << QgsExpressionContextUtils::globalScope();
  }
}
