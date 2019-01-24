/***************************************************************************
                         qgslegendstyle.cpp
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

#include "qgslegendstyle.h"
#include "qgsfontutils.h"
#include "qgssettings.h"
#include "qgis.h"

#include <QFont>
#include <QMap>
#include <QString>
#include <QDomElement>
#include <QDomDocument>
#include <QDomNode>

QgsLegendStyle::QgsLegendStyle()
{
  //get default composer font from settings
  QgsSettings settings;
  QString defaultFontString = settings.value( QStringLiteral( "LayoutDesigner/defaultFont" ), QVariant(), QgsSettings::Gui ).toString();
  if ( !defaultFontString.isEmpty() )
  {
    mFont.setFamily( defaultFontString );
  }
}

void QgsLegendStyle::setMargin( double margin )
{
  mMarginMap[Top] = margin;
  mMarginMap[Bottom] = margin;
  mMarginMap[Left] = margin;
  mMarginMap[Right] = margin;
}

void QgsLegendStyle::writeXml( const QString &name, QDomElement &elem, QDomDocument &doc ) const
{
  if ( elem.isNull() ) return;

  QDomElement styleElem = doc.createElement( QStringLiteral( "style" ) );

  styleElem.setAttribute( QStringLiteral( "name" ), name );

  if ( !qgsDoubleNear( mMarginMap[Top], 0.0 ) ) styleElem.setAttribute( QStringLiteral( "marginTop" ), QString::number( mMarginMap[Top] ) );
  if ( !qgsDoubleNear( mMarginMap[Bottom], 0.0 ) ) styleElem.setAttribute( QStringLiteral( "marginBottom" ), QString::number( mMarginMap[Bottom] ) );
  if ( !qgsDoubleNear( mMarginMap[Left], 0.0 ) ) styleElem.setAttribute( QStringLiteral( "marginLeft" ), QString::number( mMarginMap[Left] ) );
  if ( !qgsDoubleNear( mMarginMap[Right], 0.0 ) ) styleElem.setAttribute( QStringLiteral( "marginRight" ), QString::number( mMarginMap[Right] ) );

  styleElem.appendChild( QgsFontUtils::toXmlElement( mFont, doc, QStringLiteral( "styleFont" ) ) );

  elem.appendChild( styleElem );
}

void QgsLegendStyle::readXml( const QDomElement &elem, const QDomDocument &doc )
{
  Q_UNUSED( doc );
  if ( elem.isNull() ) return;

  if ( !QgsFontUtils::setFromXmlChildNode( mFont, elem, QStringLiteral( "styleFont" ) ) )
  {
    mFont.fromString( elem.attribute( QStringLiteral( "font" ) ) );
  }

  mMarginMap[Top] = elem.attribute( QStringLiteral( "marginTop" ), QStringLiteral( "0" ) ).toDouble();
  mMarginMap[Bottom] = elem.attribute( QStringLiteral( "marginBottom" ), QStringLiteral( "0" ) ).toDouble();
  mMarginMap[Left] = elem.attribute( QStringLiteral( "marginLeft" ), QStringLiteral( "0" ) ).toDouble();
  mMarginMap[Right] = elem.attribute( QStringLiteral( "marginRight" ), QStringLiteral( "0" ) ).toDouble();
}

QString QgsLegendStyle::styleName( Style s )
{
  switch ( s )
  {
    case Undefined:
      return QString();
    case Hidden:
      return QStringLiteral( "hidden" );
    case Title:
      return QStringLiteral( "title" );
    case Group:
      return QStringLiteral( "group" );
    case Subgroup:
      return QStringLiteral( "subgroup" );
    case Symbol:
      return QStringLiteral( "symbol" );
    case SymbolLabel:
      return QStringLiteral( "symbolLabel" );
  }
  return QString();
}

QgsLegendStyle::Style QgsLegendStyle::styleFromName( const QString &styleName )
{
  if ( styleName == QLatin1String( "hidden" ) ) return Hidden;
  else if ( styleName == QLatin1String( "title" ) ) return Title;
  else if ( styleName == QLatin1String( "group" ) ) return Group;
  else if ( styleName == QLatin1String( "subgroup" ) ) return Subgroup;
  else if ( styleName == QLatin1String( "symbol" ) ) return Symbol;
  else if ( styleName == QLatin1String( "symbolLabel" ) ) return SymbolLabel;
  return Undefined;
}

QString QgsLegendStyle::styleLabel( Style s )
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
  return QString();
}
