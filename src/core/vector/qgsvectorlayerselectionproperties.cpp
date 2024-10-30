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
#include "moc_qgsvectorlayerselectionproperties.cpp"
#include "qgscolorutils.h"
#include "qgssymbollayerutils.h"

QgsVectorLayerSelectionProperties::QgsVectorLayerSelectionProperties( QObject *parent )
  :  QgsMapLayerSelectionProperties( parent )
{
}

QgsVectorLayerSelectionProperties::~QgsVectorLayerSelectionProperties() = default;


QDomElement QgsVectorLayerSelectionProperties::writeXml( QDomElement &parentElement, QDomDocument &document, const QgsReadWriteContext &context )
{
  QDomElement element = document.createElement( QStringLiteral( "selection" ) );

  element.setAttribute( QStringLiteral( "mode" ), qgsEnumValueToKey( mSelectionRenderingMode ) );

  QgsColorUtils::writeXml( mSelectionColor, QStringLiteral( "selectionColor" ), document, element, context );

  if ( mSelectionSymbol )
  {
    QDomElement selectionSymbolElement = document.createElement( QStringLiteral( "selectionSymbol" ) );
    selectionSymbolElement.appendChild( QgsSymbolLayerUtils::saveSymbol( QString(), mSelectionSymbol.get(), document, context ) );
    element.appendChild( selectionSymbolElement );
  }

  parentElement.appendChild( element );
  return element;
}

bool QgsVectorLayerSelectionProperties::readXml( const QDomElement &element, const QgsReadWriteContext &context )
{
  const QDomElement selectionElement = element.firstChildElement( QStringLiteral( "selection" ) ).toElement();
  if ( selectionElement.isNull() )
    return false;

  mSelectionRenderingMode = qgsEnumKeyToValue( selectionElement.attribute( QStringLiteral( "mode" ) ), Qgis::SelectionRenderingMode::Default );
  mSelectionColor = QgsColorUtils::readXml( selectionElement, QStringLiteral( "selectionColor" ), context );

  {
    const QDomElement selectionSymbolElement = selectionElement.firstChildElement( QStringLiteral( "selectionSymbol" ) ).firstChildElement( QStringLiteral( "symbol" ) );
    mSelectionSymbol.reset( QgsSymbolLayerUtils::loadSymbol( selectionSymbolElement, context ) );
  }
  return true;
}

QgsVectorLayerSelectionProperties *QgsVectorLayerSelectionProperties::clone() const
{
  std::unique_ptr< QgsVectorLayerSelectionProperties > res = std::make_unique< QgsVectorLayerSelectionProperties >( nullptr );
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
  mSelectionColor = color;
}

QgsSymbol *QgsVectorLayerSelectionProperties::selectionSymbol() const
{
  return mSelectionSymbol.get();
}

void QgsVectorLayerSelectionProperties::setSelectionSymbol( QgsSymbol *symbol )
{
  mSelectionSymbol.reset( symbol );
}

Qgis::SelectionRenderingMode QgsVectorLayerSelectionProperties::selectionRenderingMode() const
{
  return mSelectionRenderingMode;
}

void QgsVectorLayerSelectionProperties::setSelectionRenderingMode( Qgis::SelectionRenderingMode mode )
{
  mSelectionRenderingMode = mode;
}
