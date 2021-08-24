/***************************************************************************
                         qgsrasterrendererutils.cpp
                         -------------------
    begin                : September 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dawson dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsrasterrendererutils.h"
#include "qgis.h"
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>

bool QgsRasterRendererUtils::parseColorMapFile( const QString &path, QList<QgsColorRampShader::ColorRampItem> &items, QgsColorRampShader::Type &type, QStringList &errors )
{
  QFile inputFile( path );
  if ( !inputFile.open( QFile::ReadOnly ) )
  {
    errors.append( QObject::tr( "Read access denied. Adjust the file permissions and try again.\n\n" ) );
    return false;
  }

  bool res = true;

  QTextStream inputStream( &inputFile );
  int lineCounter = 0;
  const QRegularExpression itemRegex( QStringLiteral( "^(.+?),(.+?),(.+?),(.+?),(.+?),(.+)$" ) );

  //read through the input looking for valid data
  while ( !inputStream.atEnd() )
  {
    lineCounter++;
    const QString inputLine = inputStream.readLine();
    if ( !inputLine.isEmpty() )
    {
      if ( !inputLine.simplified().startsWith( '#' ) )
      {
        if ( inputLine.contains( QLatin1String( "INTERPOLATION" ), Qt::CaseInsensitive ) )
        {
          QStringList inputStringComponents = inputLine.split( ':' );
          if ( inputStringComponents.size() == 2 )
          {
            if ( inputStringComponents[1].trimmed().toUpper().compare( QLatin1String( "INTERPOLATED" ), Qt::CaseInsensitive ) == 0 )
            {
              type = QgsColorRampShader::Interpolated;
            }
            else if ( inputStringComponents[1].trimmed().toUpper().compare( QLatin1String( "DISCRETE" ), Qt::CaseInsensitive ) == 0 )
            {
              type = QgsColorRampShader::Discrete;
            }
            else
            {
              type = QgsColorRampShader::Exact;
            }
          }
          else
          {
            res = false;
            errors << QObject::tr( "Unknown interpolation type at line %1: %2" ).arg( lineCounter ).arg( inputLine );
          }
        }
        else
        {
          const QRegularExpressionMatch match = itemRegex.match( inputLine );
          if ( match.hasMatch() )
          {
            const QgsColorRampShader::ColorRampItem currentItem( match.captured( 1 ).toDouble(),
                QColor::fromRgb( match.captured( 2 ).toInt(), match.captured( 3 ).toInt(), match.captured( 4 ).toInt(), match.captured( 5 ).toInt() ),
                match.captured( 6 ) );
            items.push_back( currentItem );
          }
          else
          {
            res = false;
            errors << QObject::tr( "Invalid entry at line %1: %2" ).arg( lineCounter ).arg( inputLine );
          }
        }
      }
    }
    lineCounter++;
  }

  return res;
}

bool QgsRasterRendererUtils::saveColorMapFile( const QString &path, const QList<QgsColorRampShader::ColorRampItem> &items, QgsColorRampShader::Type type )
{
  QFile outputFile( path );
  if ( outputFile.open( QFile::WriteOnly | QIODevice::Truncate ) )
  {
    QTextStream outputStream( &outputFile );
    outputStream << "# " << QObject::tr( "QGIS Generated Color Map Export File" ) << '\n';
    outputStream << "INTERPOLATION:";
    switch ( type )
    {
      case QgsColorRampShader::Interpolated:
        outputStream << "INTERPOLATED\n";
        break;
      case QgsColorRampShader::Discrete:
        outputStream << "DISCRETE\n";
        break;
      case QgsColorRampShader::Exact:
        outputStream << "EXACT\n";
        break;
    }

    int i = 0;
    for ( const QgsColorRampShader::ColorRampItem &item : items )
    {
      outputStream << qgsDoubleToString( item.value ) << ',';
      outputStream << item.color.red() << ',' << item.color.green() << ',' << item.color.blue() << ',' << item.color.alpha() << ',';
      if ( item.label.isEmpty() )
      {
        outputStream << "Color entry " << i + 1 << '\n';
      }
      else
      {
        outputStream << item.label << '\n';
      }
      i++;
    }
    outputStream.flush();
    outputFile.close();
    return true;
  }
  else
  {
    return false;
  }
}

