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
#include "qgis.h"
#include "qgsreadwritecontext.h"
#include "qgspropertycollection.h"

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
    mTextFormat.setSizeUnit( Qgis::RenderUnit::Points );
  }
  else if ( font.pixelSize() > 0 )
  {
    mTextFormat.setSize( font.pixelSize() );
    mTextFormat.setSizeUnit( Qgis::RenderUnit::Pixels );
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

void QgsLegendStyle::updateDataDefinedProperties( QgsRenderContext &context )
{
  if ( mTextFormat.dataDefinedProperties().hasActiveProperties() ) // note, we use format instead of tmpFormat here, it's const and potentially avoids a detach
    mTextFormat.updateDataDefinedProperties( context );

}

QString QgsLegendStyle::styleName( Qgis::LegendComponent s )
{
  switch ( s )
  {
    case Qgis::LegendComponent::Undefined:
      return QString();
    case Qgis::LegendComponent::Hidden:
      return QStringLiteral( "hidden" );
    case Qgis::LegendComponent::Title:
      return QStringLiteral( "title" );
    case Qgis::LegendComponent::Group:
      return QStringLiteral( "group" );
    case Qgis::LegendComponent::Subgroup:
      return QStringLiteral( "subgroup" );
    case Qgis::LegendComponent::Symbol:
      return QStringLiteral( "symbol" );
    case Qgis::LegendComponent::SymbolLabel:
      return QStringLiteral( "symbolLabel" );
  }
  return QString();
}

Qgis::LegendComponent QgsLegendStyle::styleFromName( const QString &styleName )
{
  if ( styleName == QLatin1String( "hidden" ) )
    return Qgis::LegendComponent::Hidden;
  else if ( styleName == QLatin1String( "title" ) )
    return Qgis::LegendComponent::Title;
  else if ( styleName == QLatin1String( "group" ) )
    return Qgis::LegendComponent::Group;
  else if ( styleName == QLatin1String( "subgroup" ) )
    return Qgis::LegendComponent::Subgroup;
  else if ( styleName == QLatin1String( "symbol" ) )
    return Qgis::LegendComponent::Symbol;
  else if ( styleName == QLatin1String( "symbolLabel" ) )
    return Qgis::LegendComponent::SymbolLabel;
  return Qgis::LegendComponent::Undefined;
}

QString QgsLegendStyle::styleLabel( Qgis::LegendComponent s )
{
  switch ( s )
  {
    case Qgis::LegendComponent::Undefined:
      return QObject::tr( "Undefined" );
    case Qgis::LegendComponent::Hidden:
      return QObject::tr( "Hidden" );
    case Qgis::LegendComponent::Title:
      return QObject::tr( "Title" );
    case Qgis::LegendComponent::Group:
      return QObject::tr( "Group" );
    case Qgis::LegendComponent::Subgroup:
      return QObject::tr( "Subgroup" );
    case Qgis::LegendComponent::Symbol:
      return QObject::tr( "Symbol" );
    case Qgis::LegendComponent::SymbolLabel:
      return QObject::tr( "Symbol label" );
  }
  return QString();
}
