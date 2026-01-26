/***************************************************************************
                         qgsvectorlayerselectionproperties.cpp
                         ---------------
    begin                : July 2023
    copyright            : (C) 2023 by Nyall Dawson
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

#include "qgsvectorlayerselectionproperties.h"

#include "qgscolorutils.h"
#include "qgssymbollayerutils.h"

#include "moc_qgsvectorlayerselectionproperties.cpp"

QgsVectorLayerSelectionProperties::QgsVectorLayerSelectionProperties( QObject *parent )
  :  QgsMapLayerSelectionProperties( parent )
{
}

QgsVectorLayerSelectionProperties::~QgsVectorLayerSelectionProperties() = default;


QDomElement QgsVectorLayerSelectionProperties::writeXml( QDomElement &parentElement, QDomDocument &document, const QgsReadWriteContext &context )
{
  QDomElement element = document.createElement( u"selection"_s );

  element.setAttribute( u"mode"_s, qgsEnumValueToKey( mSelectionRenderingMode ) );

  QgsColorUtils::writeXml( mSelectionColor, u"selectionColor"_s, document, element, context );

  if ( mSelectionSymbol )
  {
    QDomElement selectionSymbolElement = document.createElement( u"selectionSymbol"_s );
    selectionSymbolElement.appendChild( QgsSymbolLayerUtils::saveSymbol( QString(), mSelectionSymbol.get(), document, context ) );
    element.appendChild( selectionSymbolElement );
  }

  parentElement.appendChild( element );
  return element;
}

bool QgsVectorLayerSelectionProperties::readXml( const QDomElement &element, const QgsReadWriteContext &context )
{
  const QDomElement selectionElement = element.firstChildElement( u"selection"_s ).toElement();
  if ( selectionElement.isNull() )
    return false;

  mSelectionRenderingMode = qgsEnumKeyToValue( selectionElement.attribute( u"mode"_s ), Qgis::SelectionRenderingMode::Default );
  mSelectionColor = QgsColorUtils::readXml( selectionElement, u"selectionColor"_s, context );

  {
    const QDomElement selectionSymbolElement = selectionElement.firstChildElement( u"selectionSymbol"_s ).firstChildElement( u"symbol"_s );
    mSelectionSymbol = QgsSymbolLayerUtils::loadSymbol( selectionSymbolElement, context );
  }
  return true;
}

QgsVectorLayerSelectionProperties *QgsVectorLayerSelectionProperties::clone() const
{
  auto res = std::make_unique< QgsVectorLayerSelectionProperties >( nullptr );
  res->mSelectionRenderingMode = mSelectionRenderingMode;
  res->mSelectionColor = mSelectionColor;
  res->mSelectionSymbol.reset( mSelectionSymbol ? mSelectionSymbol->clone() : nullptr );
  return res.release();
}

QColor QgsVectorLayerSelectionProperties::selectionColor() const
{
  return mSelectionColor;
}

void QgsVectorLayerSelectionProperties::setSelectionColor( const QColor &color )
{
  if ( mSelectionColor == color )
  {
    return;
  }

  mSelectionColor = color;
  emit selectionColorChanged();
}

QgsSymbol *QgsVectorLayerSelectionProperties::selectionSymbol() const
{
  return mSelectionSymbol.get();
}

void QgsVectorLayerSelectionProperties::setSelectionSymbol( QgsSymbol *symbol )
{
  mSelectionSymbol.reset( symbol );
  emit selectionSymbolChanged();
}

Qgis::SelectionRenderingMode QgsVectorLayerSelectionProperties::selectionRenderingMode() const
{
  return mSelectionRenderingMode;
}

void QgsVectorLayerSelectionProperties::setSelectionRenderingMode( Qgis::SelectionRenderingMode mode )
{
  if ( mSelectionRenderingMode == mode )
  {
    return;
  }

  mSelectionRenderingMode = mode;
  emit selectionRenderingModeChanged();
}
