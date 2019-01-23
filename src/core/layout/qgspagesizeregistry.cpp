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
#include "qgslayoutmeasurementconverter.h"
#include "qgis.h"

//
// QgsPageSizeRegistry
//

QgsPageSizeRegistry::QgsPageSizeRegistry()
{
  add( QgsPageSize( QStringLiteral( "A6" ), QgsLayoutSize( 105, 148 ), QObject::tr( "A6" ) ) );
  add( QgsPageSize( QStringLiteral( "A5" ), QgsLayoutSize( 148, 210 ), QObject::tr( "A5" ) ) );
  add( QgsPageSize( QStringLiteral( "A4" ), QgsLayoutSize( 210, 297 ), QObject::tr( "A4" ) ) );
  add( QgsPageSize( QStringLiteral( "A3" ), QgsLayoutSize( 297, 420 ), QObject::tr( "A3" ) ) );
  add( QgsPageSize( QStringLiteral( "A2" ), QgsLayoutSize( 420, 594 ), QObject::tr( "A2" ) ) );
  add( QgsPageSize( QStringLiteral( "A1" ), QgsLayoutSize( 594, 841 ), QObject::tr( "A1" ) ) );
  add( QgsPageSize( QStringLiteral( "A0" ), QgsLayoutSize( 841, 1189 ), QObject::tr( "A0" ) ) );
  add( QgsPageSize( QStringLiteral( "B6" ), QgsLayoutSize( 125, 176 ), QObject::tr( "B6" ) ) );
  add( QgsPageSize( QStringLiteral( "B5" ), QgsLayoutSize( 176, 250 ), QObject::tr( "B5" ) ) );
  add( QgsPageSize( QStringLiteral( "B4" ), QgsLayoutSize( 250, 353 ), QObject::tr( "B4" ) ) );
  add( QgsPageSize( QStringLiteral( "B3" ), QgsLayoutSize( 353, 500 ), QObject::tr( "B3" ) ) );
  add( QgsPageSize( QStringLiteral( "B2" ), QgsLayoutSize( 500, 707 ), QObject::tr( "B2" ) ) );
  add( QgsPageSize( QStringLiteral( "B1" ), QgsLayoutSize( 707, 1000 ), QObject::tr( "B1" ) ) );
  add( QgsPageSize( QStringLiteral( "B0" ), QgsLayoutSize( 1000, 1414 ), QObject::tr( "B0" ) ) );
  add( QgsPageSize( QStringLiteral( "Legal" ), QgsLayoutSize( 215.9, 355.6 ), QObject::tr( "Legal" ) ) );
  add( QgsPageSize( QStringLiteral( "Letter" ), QgsLayoutSize( 215.9, 279.4 ), QObject::tr( "Letter" ) ) );
  add( QgsPageSize( QStringLiteral( "ANSI A" ), QgsLayoutSize( 215.9, 279.4 ), QObject::tr( "ANSI A" ) ) );
  add( QgsPageSize( QStringLiteral( "ANSI B" ), QgsLayoutSize( 279.4, 431.8 ), QObject::tr( "ANSI B" ) ) );
  add( QgsPageSize( QStringLiteral( "ANSI C" ), QgsLayoutSize( 431.8, 558.8 ), QObject::tr( "ANSI C" ) ) );
  add( QgsPageSize( QStringLiteral( "ANSI D" ), QgsLayoutSize( 558.8, 863.6 ), QObject::tr( "ANSI D" ) ) );
  add( QgsPageSize( QStringLiteral( "ANSI E" ), QgsLayoutSize( 863.6, 1117.6 ), QObject::tr( "ANSI E" ) ) );
  add( QgsPageSize( QStringLiteral( "Arch A" ), QgsLayoutSize( 228.6, 304.8 ), QObject::tr( "Arch A" ) ) );
  add( QgsPageSize( QStringLiteral( "Arch B" ), QgsLayoutSize( 304.8, 457.2 ), QObject::tr( "Arch B" ) ) );
  add( QgsPageSize( QStringLiteral( "Arch C" ), QgsLayoutSize( 457.2, 609.6 ), QObject::tr( "Arch C" ) ) );
  add( QgsPageSize( QStringLiteral( "Arch D" ), QgsLayoutSize( 609.6, 914.4 ), QObject::tr( "Arch D" ) ) );
  add( QgsPageSize( QStringLiteral( "Arch E" ), QgsLayoutSize( 914.4, 1219.2 ), QObject::tr( "Arch E" ) ) );
  add( QgsPageSize( QStringLiteral( "Arch E1" ), QgsLayoutSize( 762, 1066.8 ), QObject::tr( "Arch E1" ) ) );
  add( QgsPageSize( QStringLiteral( "Arch E2" ), QgsLayoutSize( 660, 965 ), QObject::tr( "Arch E2" ) ) );
  add( QgsPageSize( QStringLiteral( "Arch E3" ), QgsLayoutSize( 686, 991 ), QObject::tr( "Arch E3" ) ) );
  add( QgsPageSize( QStringLiteral( "1920x1080" ), QgsLayoutSize( 1080, 1920, QgsUnitTypes::LayoutPixels ), QObject::tr( "1920×1080" ) ) );
  add( QgsPageSize( QStringLiteral( "1280x800" ), QgsLayoutSize( 800, 1280, QgsUnitTypes::LayoutPixels ), QObject::tr( "1280×800" ) ) );
  add( QgsPageSize( QStringLiteral( "1024x768" ), QgsLayoutSize( 768, 1024, QgsUnitTypes::LayoutPixels ), QObject::tr( "1024×768" ) ) );
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
  QgsLayoutMeasurementConverter converter;
  Q_FOREACH ( const QgsPageSize &pageSize, mPageSizes )
  {
    // convert passed size to same units
    QgsLayoutSize xSize = converter.convert( size, pageSize.size.units() );

    //consider width and height values may be exchanged
    if ( ( qgsDoubleNear( xSize.width(), pageSize.size.width(), 0.01 ) && qgsDoubleNear( xSize.height(), pageSize.size.height(), 0.01 ) )
         || ( qgsDoubleNear( xSize.height(), pageSize.size.width(), 0.01 ) && qgsDoubleNear( xSize.width(), pageSize.size.height(), 0.01 ) ) )
    {
      return pageSize.name;
    }
  }
  return QString();
}

bool QgsPageSizeRegistry::decodePageSize( const QString &pageSizeName, QgsPageSize &pageSize )
{
  QList< QgsPageSize > matches = find( pageSizeName.trimmed() );
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
