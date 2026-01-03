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

#include "qgsattributeeditoraction.h"
#include "qgsattributeeditorcontainer.h"
#include "qgsattributeeditorfield.h"
#include "qgsattributeeditorhtmlelement.h"
#include "qgsattributeeditorqmlelement.h"
#include "qgsattributeeditorrelation.h"
#include "qgsattributeeditorspacerelement.h"
#include "qgsattributeeditortextelement.h"
#include "qgscolorutils.h"
#include "qgsfontutils.h"

QDomElement QgsAttributeEditorElement::toDomElement( QDomDocument &doc ) const
{
  QDomElement elem = doc.createElement( typeIdentifier() );
  elem.setAttribute( u"name"_s, mName );
  elem.setAttribute( u"showLabel"_s, mShowLabel );
  elem.setAttribute( u"horizontalStretch"_s, mHorizontalStretch );
  elem.setAttribute( u"verticalStretch"_s, mVerticalStretch );
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

  const QString name = element.attribute( u"name"_s );

  if ( element.tagName() == "attributeEditorContainer"_L1 )
  {
    newElement = new QgsAttributeEditorContainer( context.projectTranslator()->translate( u"project:layers:%1:formcontainers"_s.arg( layerId ),
        name ), parent );
  }
  else if ( element.tagName() == "attributeEditorField"_L1 )
  {
    const int idx = fields.lookupField( name );
    newElement = new QgsAttributeEditorField( name, idx, parent );
  }
  else if ( element.tagName() == "attributeEditorRelation"_L1 )
  {
    // At this time, the relations are not loaded
    // So we only grab the id and delegate the rest to onRelationsLoaded()
    newElement = new QgsAttributeEditorRelation( element.attribute( u"relation"_s, u"[None]"_s ), parent );
  }
  else if ( element.tagName() == "attributeEditorQmlElement"_L1 )
  {
    newElement = new QgsAttributeEditorQmlElement( element.attribute( u"name"_s ), parent );
  }
  else if ( element.tagName() == "attributeEditorHtmlElement"_L1 )
  {
    newElement = new QgsAttributeEditorHtmlElement( element.attribute( u"name"_s ), parent );
  }
  else if ( element.tagName() == "attributeEditorTextElement"_L1 )
  {
    newElement = new QgsAttributeEditorTextElement( element.attribute( u"name"_s ), parent );
  }
  else if ( element.tagName() == "attributeEditorSpacerElement"_L1 )
  {
    newElement = new QgsAttributeEditorSpacerElement( element.attribute( u"name"_s ), parent );
  }
  else if ( element.tagName() == "attributeEditorAction"_L1 )
  {
    newElement = new QgsAttributeEditorAction( QUuid( element.attribute( u"name"_s ) ), parent );
  }

  if ( newElement )
  {
    if ( element.hasAttribute( u"showLabel"_s ) )
      newElement->setShowLabel( element.attribute( u"showLabel"_s ).toInt() );
    else
      newElement->setShowLabel( true );

    newElement->setHorizontalStretch( element.attribute( u"horizontalStretch"_s, u"0"_s ).toInt() );
    newElement->setVerticalStretch( element.attribute( u"verticalStretch"_s, u"0"_s ).toInt() );

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
  QDomElement element { node.firstChildElement( u"labelStyle"_s ) };

  if ( ! element.isNull() )
  {

    // Label font and color
    if ( element.hasAttribute( u"labelColor"_s ) )
    {
      color = QgsColorUtils::colorFromString( element.attribute( u"labelColor"_s ) );
    }

    QFont newFont;
    QgsFontUtils::setFromXmlChildNode( newFont, element, u"labelFont"_s );

    font = newFont;

    if ( element.hasAttribute( u"overrideLabelColor"_s ) )
    {
      overrideColor = element.attribute( u"overrideLabelColor"_s ) == QChar( '1' );
    }

    if ( element.hasAttribute( u"overrideLabelFont"_s ) )
    {
      overrideFont = element.attribute( u"overrideLabelFont"_s ) == QChar( '1' );
    }
  }
}

QDomElement QgsAttributeEditorElement::LabelStyle::writeXml( QDomDocument &document ) const
{
  QDomElement elem {  document.createElement( u"labelStyle"_s ) };
  elem.setAttribute( u"labelColor"_s, QgsColorUtils::colorToString( color ) );
  elem.appendChild( QgsFontUtils::toXmlElement( font, document, u"labelFont"_s ) );
  elem.setAttribute( u"overrideLabelColor"_s, overrideColor ? QChar( '1' ) : QChar( '0' ) );
  elem.setAttribute( u"overrideLabelFont"_s, overrideFont ? QChar( '1' ) : QChar( '0' ) );
  return elem;
}

bool QgsAttributeEditorElement::LabelStyle::operator==( const LabelStyle &other ) const
{
  return overrideColor == other.overrideColor && overrideFont == other.overrideFont && color == other.color && font == other.font;
}
