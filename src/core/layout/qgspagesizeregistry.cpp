/***************************************************************************
                            qgspagesizeregistry.cpp
                            ------------------------
    begin                : June 2017
    copyright            : (C) 2017 by Nyall Dawson
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

#include "qgspagesizeregistry.h"

#include "qgis.h"
#include "qgslayoutmeasurementconverter.h"

//
// QgsPageSizeRegistry
//

QgsPageSizeRegistry::QgsPageSizeRegistry()
{
  add( QgsPageSize( u"A6"_s, QgsLayoutSize( 105, 148 ), QObject::tr( "A6" ) ) );
  add( QgsPageSize( u"A5"_s, QgsLayoutSize( 148, 210 ), QObject::tr( "A5" ) ) );
  add( QgsPageSize( u"A4"_s, QgsLayoutSize( 210, 297 ), QObject::tr( "A4" ) ) );
  add( QgsPageSize( u"A3"_s, QgsLayoutSize( 297, 420 ), QObject::tr( "A3" ) ) );
  add( QgsPageSize( u"A2"_s, QgsLayoutSize( 420, 594 ), QObject::tr( "A2" ) ) );
  add( QgsPageSize( u"A1"_s, QgsLayoutSize( 594, 841 ), QObject::tr( "A1" ) ) );
  add( QgsPageSize( u"A0"_s, QgsLayoutSize( 841, 1189 ), QObject::tr( "A0" ) ) );
  add( QgsPageSize( u"B6"_s, QgsLayoutSize( 125, 176 ), QObject::tr( "B6" ) ) );
  add( QgsPageSize( u"B5"_s, QgsLayoutSize( 176, 250 ), QObject::tr( "B5" ) ) );
  add( QgsPageSize( u"B4"_s, QgsLayoutSize( 250, 353 ), QObject::tr( "B4" ) ) );
  add( QgsPageSize( u"B3"_s, QgsLayoutSize( 353, 500 ), QObject::tr( "B3" ) ) );
  add( QgsPageSize( u"B2"_s, QgsLayoutSize( 500, 707 ), QObject::tr( "B2" ) ) );
  add( QgsPageSize( u"B1"_s, QgsLayoutSize( 707, 1000 ), QObject::tr( "B1" ) ) );
  add( QgsPageSize( u"B0"_s, QgsLayoutSize( 1000, 1414 ), QObject::tr( "B0" ) ) );
  add( QgsPageSize( u"Legal"_s, QgsLayoutSize( 215.9, 355.6 ), QObject::tr( "Legal" ) ) );
  add( QgsPageSize( u"Letter"_s, QgsLayoutSize( 215.9, 279.4 ), QObject::tr( "Letter" ) ) );
  add( QgsPageSize( u"ANSI A"_s, QgsLayoutSize( 215.9, 279.4 ), QObject::tr( "ANSI A" ) ) );
  add( QgsPageSize( u"ANSI B"_s, QgsLayoutSize( 279.4, 431.8 ), QObject::tr( "ANSI B" ) ) );
  add( QgsPageSize( u"ANSI C"_s, QgsLayoutSize( 431.8, 558.8 ), QObject::tr( "ANSI C" ) ) );
  add( QgsPageSize( u"ANSI D"_s, QgsLayoutSize( 558.8, 863.6 ), QObject::tr( "ANSI D" ) ) );
  add( QgsPageSize( u"ANSI E"_s, QgsLayoutSize( 863.6, 1117.6 ), QObject::tr( "ANSI E" ) ) );
  add( QgsPageSize( u"Arch A"_s, QgsLayoutSize( 228.6, 304.8 ), QObject::tr( "Arch A" ) ) );
  add( QgsPageSize( u"Arch B"_s, QgsLayoutSize( 304.8, 457.2 ), QObject::tr( "Arch B" ) ) );
  add( QgsPageSize( u"Arch C"_s, QgsLayoutSize( 457.2, 609.6 ), QObject::tr( "Arch C" ) ) );
  add( QgsPageSize( u"Arch D"_s, QgsLayoutSize( 609.6, 914.4 ), QObject::tr( "Arch D" ) ) );
  add( QgsPageSize( u"Arch E"_s, QgsLayoutSize( 914.4, 1219.2 ), QObject::tr( "Arch E" ) ) );
  add( QgsPageSize( u"Arch E1"_s, QgsLayoutSize( 762, 1066.8 ), QObject::tr( "Arch E1" ) ) );
  add( QgsPageSize( u"Arch E2"_s, QgsLayoutSize( 660, 965 ), QObject::tr( "Arch E2" ) ) );
  add( QgsPageSize( u"Arch E3"_s, QgsLayoutSize( 686, 991 ), QObject::tr( "Arch E3" ) ) );
  add( QgsPageSize( u"1920x1080"_s, QgsLayoutSize( 1080, 1920, Qgis::LayoutUnit::Pixels ), QObject::tr( "1920×1080 (16:9)" ) ) );
  add( QgsPageSize( u"1280x800"_s, QgsLayoutSize( 800, 1280, Qgis::LayoutUnit::Pixels ), QObject::tr( "1280×800 (16:10)" ) ) );
  add( QgsPageSize( u"1024x768"_s, QgsLayoutSize( 768, 1024, Qgis::LayoutUnit::Pixels ), QObject::tr( "1024×768 (4:3)" ) ) );
}

void QgsPageSizeRegistry::add( const QgsPageSize &size )
{
  mPageSizes.append( size );
}

QList<QgsPageSize> QgsPageSizeRegistry::entries() const
{
  QList< QgsPageSize > result;
  QList< QgsPageSize >::const_iterator it = mPageSizes.constBegin();
  for ( ; it != mPageSizes.constEnd(); ++it )
  {
    result.push_back( *it );
  }
  return result;
}

QList<QgsPageSize> QgsPageSizeRegistry::find( const QString &name ) const
{
  QList< QgsPageSize > result;
  QList< QgsPageSize >::const_iterator it = mPageSizes.constBegin();
  for ( ; it != mPageSizes.constEnd(); ++it )
  {
    if ( ( *it ).name.compare( name, Qt::CaseInsensitive ) == 0 )
    {
      result.push_back( *it );
    }
  }
  return result;
}

QString QgsPageSizeRegistry::find( const QgsLayoutSize &size ) const
{
  //try to match to existing page size
  const QgsLayoutMeasurementConverter converter;
  const auto constMPageSizes = mPageSizes;
  for ( const QgsPageSize &pageSize : constMPageSizes )
  {
    // convert passed size to same units
    const QgsLayoutSize xSize = converter.convert( size, pageSize.size.units() );

    //consider width and height values may be exchanged
    if ( ( qgsDoubleNear( xSize.width(), pageSize.size.width(), 0.01 ) && qgsDoubleNear( xSize.height(), pageSize.size.height(), 0.01 ) )
         || ( qgsDoubleNear( xSize.height(), pageSize.size.width(), 0.01 ) && qgsDoubleNear( xSize.width(), pageSize.size.height(), 0.01 ) ) )
    {
      return pageSize.name;
    }
  }
  return QString();
}

bool QgsPageSizeRegistry::decodePageSize( const QString &pageSizeName, QgsPageSize &pageSize ) const
{
  const QList< QgsPageSize > matches = find( pageSizeName.trimmed() );
  if ( matches.length() > 0 )
  {
    pageSize = matches.at( 0 );
    return true;
  }
  return false;
}

//
// QgsPageSize
//

QgsPageSize::QgsPageSize()
  : size( QgsLayoutSize( 0.0, 0.0 ) )
{
}

QgsPageSize::QgsPageSize( const QString &pageName, const QgsLayoutSize &pageSize, const QString &displayName )
  : name( pageName )
  , size( pageSize )
  , displayName( displayName )
{
}

QgsPageSize::QgsPageSize( const QgsLayoutSize &pageSize )
  :  size( pageSize )
{

}

bool QgsPageSize::operator==( const QgsPageSize &other ) const
{
  return ( name == other.name && size == other.size );
}

bool QgsPageSize::operator!=( const QgsPageSize &other ) const
{
  return ( ! operator==( other ) );
}
