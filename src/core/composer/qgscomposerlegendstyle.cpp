/***************************************************************************
                         qgscomposerlegendstyle.cpp
                         ---------------------
    begin                : March 2013
    copyright            : (C) 2013 by Radim Blazek
    email                : radim.blazek@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscomposerlegendstyle.h"
#include "qgscomposition.h"
#include "qgsfontutils.h"
#include <QFont>
#include <QMap>
#include <QSettings>
#include <QString>
#include <QDomElement>
#include <QDomDocument>
#include <QDomNode>

QgsComposerLegendStyle::QgsComposerLegendStyle()
{
  //get default composer font from settings
  QSettings settings;
  QString defaultFontString = settings.value( "/Composer/defaultFont" ).toString();
  if ( !defaultFontString.isEmpty() )
  {
    mFont.setFamily( defaultFontString );
  }
}

QgsComposerLegendStyle::~QgsComposerLegendStyle()
{
}

void QgsComposerLegendStyle::setMargin( double margin )
{
  mMarginMap[Top] = margin;
  mMarginMap[Bottom] = margin;
  mMarginMap[Left] = margin;
  mMarginMap[Right] = margin;
}

void QgsComposerLegendStyle::writeXML( QString name, QDomElement& elem, QDomDocument & doc ) const
{
  if ( elem.isNull() ) return;

  QDomElement styleElem = doc.createElement( "style" );

  styleElem.setAttribute( "name", name );

  if ( mMarginMap[Top] != 0 ) styleElem.setAttribute( "marginTop", QString::number( mMarginMap[Top] ) );
  if ( mMarginMap[Bottom] != 0 ) styleElem.setAttribute( "marginBottom", QString::number( mMarginMap[Bottom] ) );
  if ( mMarginMap[Left] != 0 ) styleElem.setAttribute( "marginLeft", QString::number( mMarginMap[Left] ) );
  if ( mMarginMap[Right] != 0 ) styleElem.setAttribute( "marginRight", QString::number( mMarginMap[Right] ) );

  styleElem.appendChild( QgsFontUtils::toXmlElement( mFont, doc, "styleFont" ) );

  elem.appendChild( styleElem );
}

void QgsComposerLegendStyle::readXML( const QDomElement& elem, const QDomDocument& doc )
{
  Q_UNUSED( doc );
  if ( elem.isNull() ) return;

  if ( !QgsFontUtils::setFromXmlChildNode( mFont, elem, "styleFont" ) )
  {
    mFont.fromString( elem.attribute( "font" ) );
  }

  mMarginMap[Top] = elem.attribute( "marginTop", "0" ).toDouble();
  mMarginMap[Bottom] = elem.attribute( "marginBottom", "0" ).toDouble();
  mMarginMap[Left] = elem.attribute( "marginLeft", "0" ).toDouble();
  mMarginMap[Right] = elem.attribute( "marginRight", "0" ).toDouble();
}

QString QgsComposerLegendStyle::styleName( Style s )
{
  switch ( s )
  {
    case Undefined:
      return "";
    case Hidden:
      return "hidden";
    case Title:
      return "title";
    case Group:
      return "group";
    case Subgroup:
      return "subgroup";
    case Symbol:
      return "symbol";
    case SymbolLabel:
      return "symbolLabel";
  }
  return "";
}

QgsComposerLegendStyle::Style QgsComposerLegendStyle::styleFromName( QString styleName )
{
  if ( styleName == "hidden" ) return Hidden;
  else if ( styleName == "title" ) return Title;
  else if ( styleName == "group" ) return Group;
  else if ( styleName == "subgroup" ) return Subgroup;
  else if ( styleName == "symbol" ) return Symbol;
  else if ( styleName == "symbolLabel" ) return SymbolLabel;
  return Undefined;
}

QString QgsComposerLegendStyle::styleLabel( Style s )
{
  switch ( s )
  {
    case Undefined:
      return QObject::tr( "Undefined" );
    case Hidden:
      return QObject::tr( "Hidden" );
    case Title:
      return QObject::tr( "Title" );
    case Group:
      return QObject::tr( "Group" );
    case Subgroup:
      return QObject::tr( "Subgroup" );
    case Symbol:
      return QObject::tr( "Symbol" );
    case SymbolLabel:
      return QObject::tr( "Symbol label" );
  }
  return "";
}
