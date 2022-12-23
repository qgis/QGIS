/***************************************************************************
  qgsattributeeditorelement.cpp - QgsAttributeEditorElement

 ---------------------
 begin                : 18.8.2016
 copyright            : (C) 2016 by Matthias Kuhn
 email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsattributeeditorelement.h"

#include "qgsattributeeditorcontainer.h"
#include "qgsattributeeditorfield.h"
#include "qgsattributeeditoraction.h"
#include "qgsattributeeditorhtmlelement.h"
#include "qgsattributeeditorqmlelement.h"
#include "qgsattributeeditorrelation.h"
#include "qgssymbollayerutils.h"
#include "qgsfontutils.h"

QDomElement QgsAttributeEditorElement::toDomElement( QDomDocument &doc ) const
{
  QDomElement elem = doc.createElement( typeIdentifier() );
  elem.setAttribute( QStringLiteral( "name" ), mName );
  elem.setAttribute( QStringLiteral( "showLabel" ), mShowLabel );
  elem.appendChild( mLabelStyle.writeXml( doc ) );
  saveConfiguration( elem, doc );
  return elem;
}

bool QgsAttributeEditorElement::showLabel() const
{
  return mShowLabel;
}

void QgsAttributeEditorElement::setShowLabel( bool showLabel )
{
  mShowLabel = showLabel;
}

QgsAttributeEditorElement::LabelStyle QgsAttributeEditorElement::labelStyle() const
{
  return mLabelStyle;
}

void QgsAttributeEditorElement::setLabelStyle( const QgsAttributeEditorElement::LabelStyle &labelStyle )
{
  mLabelStyle = labelStyle;
}

QgsAttributeEditorElement *QgsAttributeEditorElement::create( const QDomElement &element, const QString &layerId, const QgsFields &fields, const QgsReadWriteContext &context, QgsAttributeEditorElement *parent )
{
  QgsAttributeEditorElement *newElement = nullptr;

  const QString name = element.attribute( QStringLiteral( "name" ) );

  if ( element.tagName() == QLatin1String( "attributeEditorContainer" ) )
  {
    newElement = new QgsAttributeEditorContainer( context.projectTranslator()->translate( QStringLiteral( "project:layers:%1:formcontainers" ).arg( layerId ),
        name ), parent );
  }
  else if ( element.tagName() == QLatin1String( "attributeEditorField" ) )
  {
    const int idx = fields.lookupField( name );
    newElement = new QgsAttributeEditorField( name, idx, parent );
  }
  else if ( element.tagName() == QLatin1String( "attributeEditorRelation" ) )
  {
    // At this time, the relations are not loaded
    // So we only grab the id and delegate the rest to onRelationsLoaded()
    newElement = new QgsAttributeEditorRelation( element.attribute( QStringLiteral( "relation" ), QStringLiteral( "[None]" ) ), parent );
  }
  else if ( element.tagName() == QLatin1String( "attributeEditorQmlElement" ) )
  {
    newElement = new QgsAttributeEditorQmlElement( element.attribute( QStringLiteral( "name" ) ), parent );
  }
  else if ( element.tagName() == QLatin1String( "attributeEditorHtmlElement" ) )
  {
    newElement = new QgsAttributeEditorHtmlElement( element.attribute( QStringLiteral( "name" ) ), parent );
  }
  else if ( element.tagName() == QLatin1String( "attributeEditorAction" ) )
  {
    newElement = new QgsAttributeEditorAction( QUuid( element.attribute( QStringLiteral( "name" ) ) ), parent );
  }

  if ( newElement )
  {
    if ( element.hasAttribute( QStringLiteral( "showLabel" ) ) )
      newElement->setShowLabel( element.attribute( QStringLiteral( "showLabel" ) ).toInt() );
    else
      newElement->setShowLabel( true );

    // Label font and color
    LabelStyle style;
    style.readXml( element );
    newElement->setLabelStyle( style );

    newElement->loadConfiguration( element, layerId, context, fields );
  }

  return newElement;
}


void QgsAttributeEditorElement::LabelStyle::readXml( const QDomNode &node )
{
  QDomElement element { node.firstChildElement( QStringLiteral( "labelStyle" ) ) };

  if ( ! element.isNull() )
  {

    // Label font and color
    if ( element.hasAttribute( QStringLiteral( "labelColor" ) ) )
    {
      color = QgsSymbolLayerUtils::decodeColor( element.attribute( QStringLiteral( "labelColor" ) ) );
    }

    QFont newFont;
    QgsFontUtils::setFromXmlChildNode( newFont, element, QStringLiteral( "labelFont" ) );

    font = newFont;

    if ( element.hasAttribute( QStringLiteral( "overrideLabelColor" ) ) )
    {
      overrideColor = element.attribute( QStringLiteral( "overrideLabelColor" ) ) == QChar( '1' );
    }

    if ( element.hasAttribute( QStringLiteral( "overrideLabelFont" ) ) )
    {
      overrideFont = element.attribute( QStringLiteral( "overrideLabelFont" ) ) == QChar( '1' );
    }
  }
}

QDomElement QgsAttributeEditorElement::LabelStyle::writeXml( QDomDocument &document ) const
{
  QDomElement elem {  document.createElement( QStringLiteral( "labelStyle" ) ) };
  elem.setAttribute( QStringLiteral( "labelColor" ), QgsSymbolLayerUtils::encodeColor( color ) );
  elem.appendChild( QgsFontUtils::toXmlElement( font, document, QStringLiteral( "labelFont" ) ) );
  elem.setAttribute( QStringLiteral( "overrideLabelColor" ), overrideColor ? QChar( '1' ) : QChar( '0' ) );
  elem.setAttribute( QStringLiteral( "overrideLabelFont" ), overrideFont ? QChar( '1' ) : QChar( '0' ) );
  return elem;
}

bool QgsAttributeEditorElement::LabelStyle::operator==( const LabelStyle &other ) const
{
  return overrideColor == other.overrideColor && overrideFont == other.overrideFont && color == other.color && font == other.font;
}
