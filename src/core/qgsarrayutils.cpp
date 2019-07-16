/***************************************************************************
                                  qgsarrayutils.h
                              ---------------------
    begin                : July 2019
    copyright            : (C) 2019 by David Signer
    email                : david at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsarrayutils.h"
#include <QDebug>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

QVariantList QgsArrayUtils::parse( const QString &string )
{
  QVariantList variantList;

  QString newVal = string;
  if ( newVal.trimmed().startsWith( '{' ) )
  {
    newVal =  newVal.trimmed().mid( 1 ).mid( 0, newVal.length() - 2 ).prepend( '[' ).append( ']' );

    if ( !json::accept( newVal.toStdString() ) )
    {
      //fallback for wrongly stored string data without quotes
      newVal = string;
      const QStringList stringList = newVal.remove( QChar( '{' ) ).remove( QChar( '}' ) ).split( ',' );
      for ( const QString &s : qgis::as_const( stringList ) )
      {
        variantList.push_back( s );
      }
    }
  }

  if ( newVal.trimmed().startsWith( '[' ) )
  {
    try
    {
      for ( auto &element : json::parse( newVal.toStdString() ) )
      {
        if ( element.is_number_integer() )
        {
          variantList.push_back( element.get<int>() );
        }
        else if ( element.is_number_unsigned() )
        {
          variantList.push_back( element.get<unsigned>() );
        }
        else if ( element.is_string() )
        {
          variantList.push_back( QString::fromStdString( element.get<std::string>() ) );
        }
      }
    }
    catch ( json::parse_error &ex )
    {
      qDebug() << QString::fromStdString( ex.what() );
    }
  }
  return variantList;
}

QString QgsArrayUtils::build( const QVariantList &list )
{
  QStringList sl;
  for ( const QVariant &v : qgis::as_const( list ) )
  {
    // Convert to proper type
    switch ( v.type() )
    {
      case QVariant::Type::Int:
      case QVariant::Type::LongLong:
        sl.push_back( v.toString() );
        break;
      default:
        QString newS = v.toString();
        newS.replace( '\\', QStringLiteral( R"(\\)" ) );
        newS.replace( '\"', QStringLiteral( R"(\")" ) );
        sl.push_back( "\"" + newS + "\"" );
        break;
    }
  }
  //store as a formatted string because the fields supports only string
  QString s = sl.join( ',' ).prepend( '{' ).append( '}' );

  return s;
}
