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

#include "qgis.h"
#include "qgsfontutils.h"
#include "qgspropertycollection.h"
#include "qgsreadwritecontext.h"

#include <QDomDocument>
#include <QDomElement>
#include <QDomNode>
#include <QFont>
#include <QMap>
#include <QString>

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

  QDomElement styleElem = doc.createElement( u"style"_s );

  styleElem.setAttribute( u"name"_s, name );
  styleElem.setAttribute( u"alignment"_s, QString::number( mAlignment ) );
  styleElem.setAttribute( u"indent"_s, QString::number( mIndent ) );

  if ( !qgsDoubleNear( mMarginMap[Top], 0.0 ) )
    styleElem.setAttribute( u"marginTop"_s, QString::number( mMarginMap[Top] ) );
  if ( !qgsDoubleNear( mMarginMap[Bottom], 0.0 ) )
    styleElem.setAttribute( u"marginBottom"_s, QString::number( mMarginMap[Bottom] ) );
  if ( !qgsDoubleNear( mMarginMap[Left], 0.0 ) )
    styleElem.setAttribute( u"marginLeft"_s, QString::number( mMarginMap[Left] ) );
  if ( !qgsDoubleNear( mMarginMap[Right], 0.0 ) )
    styleElem.setAttribute( u"marginRight"_s, QString::number( mMarginMap[Right] ) );

  QDomElement textElem = mTextFormat.writeXml( doc, context );
  styleElem.appendChild( textElem );

  elem.appendChild( styleElem );
}

void QgsLegendStyle::readXml( const QDomElement &elem, const QDomDocument &doc, const QgsReadWriteContext &context )
{
  Q_UNUSED( doc )
  if ( elem.isNull() ) return;

  QDomNodeList textFormatNodeList = elem.elementsByTagName( u"text-style"_s );
  if ( !textFormatNodeList.isEmpty() )
  {
    QDomElement textFormatElem = textFormatNodeList.at( 0 ).toElement();
    mTextFormat.readXml( textFormatElem, context );
  }
  else
  {
    QFont f;
    if ( !QgsFontUtils::setFromXmlChildNode( f, elem, u"styleFont"_s ) )
    {
      f.fromString( elem.attribute( u"font"_s ) );
    }
    mTextFormat = QgsTextFormat::fromQFont( f );
  }

  mMarginMap[Top] = elem.attribute( u"marginTop"_s, u"0"_s ).toDouble();
  mMarginMap[Bottom] = elem.attribute( u"marginBottom"_s, u"0"_s ).toDouble();
  mMarginMap[Left] = elem.attribute( u"marginLeft"_s, u"0"_s ).toDouble();
  mMarginMap[Right] = elem.attribute( u"marginRight"_s, u"0"_s ).toDouble();

  mAlignment = static_cast< Qt::Alignment >( elem.attribute( u"alignment"_s, QString::number( Qt::AlignLeft ) ).toInt() );
  mIndent = elem.attribute( u"indent"_s, u"0"_s ).toDouble();
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
      return u"hidden"_s;
    case Qgis::LegendComponent::Title:
      return u"title"_s;
    case Qgis::LegendComponent::Group:
      return u"group"_s;
    case Qgis::LegendComponent::Subgroup:
      return u"subgroup"_s;
    case Qgis::LegendComponent::Symbol:
      return u"symbol"_s;
    case Qgis::LegendComponent::SymbolLabel:
      return u"symbolLabel"_s;
  }
  return QString();
}

Qgis::LegendComponent QgsLegendStyle::styleFromName( const QString &styleName )
{
  if ( styleName == "hidden"_L1 )
    return Qgis::LegendComponent::Hidden;
  else if ( styleName == "title"_L1 )
    return Qgis::LegendComponent::Title;
  else if ( styleName == "group"_L1 )
    return Qgis::LegendComponent::Group;
  else if ( styleName == "subgroup"_L1 )
    return Qgis::LegendComponent::Subgroup;
  else if ( styleName == "symbol"_L1 )
    return Qgis::LegendComponent::Symbol;
  else if ( styleName == "symbolLabel"_L1 )
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
