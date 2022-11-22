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
#include "qgsreadwritecontext.h"

#include <QFont>
#include <QMap>
#include <QString>
#include <QDomElement>
#include <QDomDocument>
#include <QDomNode>

QgsLegendStyle::QgsLegendStyle()
{
}

void QgsLegendStyle::setFont( const QFont &font )
{
  mTextFormat.setFont( font );
  if ( font.pointSizeF() > 0 )
  {
    mTextFormat.setSize( font.pointSizeF() );
    mTextFormat.setSizeUnit( QgsUnitTypes::RenderPoints );
  }
  else if ( font.pixelSize() > 0 )
  {
    mTextFormat.setSize( font.pixelSize() );
    mTextFormat.setSizeUnit( QgsUnitTypes::RenderPixels );
  }
}

void QgsLegendStyle::setMargin( double margin )
{
  mMarginMap[Top] = margin;
  mMarginMap[Bottom] = margin;
  mMarginMap[Left] = margin;
  mMarginMap[Right] = margin;
}

void QgsLegendStyle::writeXml( const QString &name, QDomElement &elem, QDomDocument &doc, const QgsReadWriteContext &context ) const
{
  if ( elem.isNull() )
    return;

  QDomElement styleElem = doc.createElement( QStringLiteral( "style" ) );

  styleElem.setAttribute( QStringLiteral( "name" ), name );
  styleElem.setAttribute( QStringLiteral( "alignment" ), QString::number( mAlignment ) );
  styleElem.setAttribute( QStringLiteral( "indent" ), QString::number( mIndent ) );

  if ( !qgsDoubleNear( mMarginMap[Top], 0.0 ) )
    styleElem.setAttribute( QStringLiteral( "marginTop" ), QString::number( mMarginMap[Top] ) );
  if ( !qgsDoubleNear( mMarginMap[Bottom], 0.0 ) )
    styleElem.setAttribute( QStringLiteral( "marginBottom" ), QString::number( mMarginMap[Bottom] ) );
  if ( !qgsDoubleNear( mMarginMap[Left], 0.0 ) )
    styleElem.setAttribute( QStringLiteral( "marginLeft" ), QString::number( mMarginMap[Left] ) );
  if ( !qgsDoubleNear( mMarginMap[Right], 0.0 ) )
    styleElem.setAttribute( QStringLiteral( "marginRight" ), QString::number( mMarginMap[Right] ) );

  QDomElement textElem = mTextFormat.writeXml( doc, context );
  styleElem.appendChild( textElem );

  elem.appendChild( styleElem );
}

void QgsLegendStyle::readXml( const QDomElement &elem, const QDomDocument &doc, const QgsReadWriteContext &context )
{
  Q_UNUSED( doc )
  if ( elem.isNull() ) return;

  QDomNodeList textFormatNodeList = elem.elementsByTagName( QStringLiteral( "text-style" ) );
  if ( !textFormatNodeList.isEmpty() )
  {
    QDomElement textFormatElem = textFormatNodeList.at( 0 ).toElement();
    mTextFormat.readXml( textFormatElem, context );
  }
  else
  {
    QFont f;
    if ( !QgsFontUtils::setFromXmlChildNode( f, elem, QStringLiteral( "styleFont" ) ) )
    {
      f.fromString( elem.attribute( QStringLiteral( "font" ) ) );
    }
    mTextFormat = QgsTextFormat::fromQFont( f );
  }

  mMarginMap[Top] = elem.attribute( QStringLiteral( "marginTop" ), QStringLiteral( "0" ) ).toDouble();
  mMarginMap[Bottom] = elem.attribute( QStringLiteral( "marginBottom" ), QStringLiteral( "0" ) ).toDouble();
  mMarginMap[Left] = elem.attribute( QStringLiteral( "marginLeft" ), QStringLiteral( "0" ) ).toDouble();
  mMarginMap[Right] = elem.attribute( QStringLiteral( "marginRight" ), QStringLiteral( "0" ) ).toDouble();

  mAlignment = static_cast< Qt::Alignment >( elem.attribute( QStringLiteral( "alignment" ), QString::number( Qt::AlignLeft ) ).toInt() );
  mIndent = elem.attribute( QStringLiteral( "indent" ), QStringLiteral( "0" ) ).toDouble();
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
  if ( styleName == QLatin1String( "hidden" ) )
    return Hidden;
  else if ( styleName == QLatin1String( "title" ) )
    return Title;
  else if ( styleName == QLatin1String( "group" ) )
    return Group;
  else if ( styleName == QLatin1String( "subgroup" ) )
    return Subgroup;
  else if ( styleName == QLatin1String( "symbol" ) )
    return Symbol;
  else if ( styleName == QLatin1String( "symbolLabel" ) )
    return SymbolLabel;
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
