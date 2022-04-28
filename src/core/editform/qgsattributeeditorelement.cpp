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
  elem.setAttribute( QStringLiteral( "labelColor" ), QgsSymbolLayerUtils::encodeColor( mLabelColor ) );
  elem.appendChild( QgsFontUtils::toXmlElement( mLabelFont, doc, QStringLiteral( "labelFont" ) ) );
  elem.setAttribute( QStringLiteral( "overrideLabelColor" ), mOverrideLabelColor ? QChar( '1' ) : QChar( '0' ) );
  elem.setAttribute( QStringLiteral( "overrideLabelFont" ), mOverrideLabelFont ? QChar( '1' ) : QChar( '0' ) );
  // Font properties
  elem.setAttribute( QStringLiteral( "fontBold" ), mLabelFont.bold() ? QChar( '1' ) : QChar( '0' ) );
  elem.setAttribute( QStringLiteral( "fontItalic" ), mLabelFont.italic() ? QChar( '1' ) : QChar( '0' ) );
  elem.setAttribute( QStringLiteral( "fontUnderline" ), mLabelFont.underline() ? QChar( '1' ) : QChar( '0' ) );
  elem.setAttribute( QStringLiteral( "fontStrikethrough" ), mLabelFont.strikeOut() ? QChar( '1' ) : QChar( '0' ) );
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

QFont QgsAttributeEditorElement::labelFont() const
{
  return mLabelFont;
}

void QgsAttributeEditorElement::setLabelFont( const QFont &labelFont )
{
  mLabelFont = labelFont;
}

QColor QgsAttributeEditorElement::labelColor() const
{
  return mLabelColor;
}

void QgsAttributeEditorElement::setLabelColor( const QColor &labelColor )
{
  mLabelColor = labelColor;
}

bool QgsAttributeEditorElement::overrideLabelColor() const
{
  return mOverrideLabelColor;
}

void QgsAttributeEditorElement::setOverrideLabelColor( bool overrideLabelColor )
{
  mOverrideLabelColor = overrideLabelColor;
}

bool QgsAttributeEditorElement::overrideLabelFont() const
{
  return mOverrideLabelFont;
}

void QgsAttributeEditorElement::setOverrideLabelFont( bool overrideLabelFont )
{
  mOverrideLabelFont = overrideLabelFont;
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
    if ( element.hasAttribute( QStringLiteral( "labelColor" ) ) )
    {
      newElement->setLabelColor( QgsSymbolLayerUtils::decodeColor( element.attribute( QStringLiteral( "labelColor" ) ) ) );
    }

    QFont font;
    QgsFontUtils::setFromXmlChildNode( font, element, QStringLiteral( "labelFont" ) );

    // Font properties
    if ( element.hasAttribute( QStringLiteral( "fontBold" ) ) )
    {
      font.setBold( element.attribute( QStringLiteral( "fontBold" ) ) == QChar( '1' ) );
    }
    if ( element.hasAttribute( QStringLiteral( "fontItalic" ) ) )
    {
      font.setItalic( element.attribute( QStringLiteral( "fontItalic" ) ) == QChar( '1' ) );
    }
    if ( element.hasAttribute( QStringLiteral( "fontUnderline" ) ) )
    {
      font.setUnderline( element.attribute( QStringLiteral( "fontUnderline" ) ) == QChar( '1' ) );
    }
    if ( element.hasAttribute( QStringLiteral( "fontStrikethrough" ) ) )
    {
      font.setStrikeOut( element.attribute( QStringLiteral( "fontStrikethrough" ) ) == QChar( '1' ) );
    }

    newElement->setLabelFont( font );

    if ( element.hasAttribute( QStringLiteral( "overrideLabelColor" ) ) )
    {
      newElement->setOverrideLabelColor( element.attribute( QStringLiteral( "overrideLabelColor" ) ) == QChar( '1' ) );
    }

    if ( element.hasAttribute( QStringLiteral( "overrideLabelFont" ) ) )
    {
      newElement->setOverrideLabelFont( element.attribute( QStringLiteral( "overrideLabelFont" ) ) == QChar( '1' ) );
    }

    newElement->loadConfiguration( element, layerId, context, fields );
  }

  return newElement;
}

