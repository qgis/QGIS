/***************************************************************************
    qgsscaleutils.cpp
    ---------------------
    begin                : July 2012
    copyright            : (C) 2012 by Alexander Bruy
    email                : alexander dot bruy at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsscaleutils.h"
#include "qgis.h"

#include <QFile>
#include <QDomDocument>
#include <QTextStream>

bool QgsScaleUtils::saveScaleList( const QString &fileName, const QStringList &scales, QString &errorMessage )
{
  QDomDocument doc;
  QDomElement root = doc.createElement( QStringLiteral( "qgsScales" ) );
  root.setAttribute( QStringLiteral( "version" ), QStringLiteral( "1.0" ) );
  doc.appendChild( root );

  for ( int i = 0; i < scales.count(); ++i )
  {
    QDomElement el = doc.createElement( QStringLiteral( "scale" ) );
    el.setAttribute( QStringLiteral( "value" ), scales.at( i ) );
    root.appendChild( el );
  }

  QFile file( fileName );
  if ( !file.open( QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate ) )
  {
    errorMessage = QStringLiteral( "Cannot write file %1:\n%2." ).arg( fileName, file.errorString() );
    return false;
  }

  QTextStream out( &file );
  doc.save( out, 4 );
  return true;
}

bool QgsScaleUtils::loadScaleList( const QString &fileName, QStringList &scales, QString &errorMessage )
{
  QFile file( fileName );
  if ( !file.open( QIODevice::ReadOnly | QIODevice::Text ) )
  {
    errorMessage = QStringLiteral( "Cannot read file %1:\n%2." ).arg( fileName, file.errorString() );
    return false;
  }

  QDomDocument doc;
  QString errorStr;
  int errorLine;
  int errorColumn;

  if ( !doc.setContent( &file, true, &errorStr, &errorLine, &errorColumn ) )
  {
    errorMessage = QStringLiteral( "Parse error at line %1, column %2:\n%3" )
                   .arg( errorLine )
                   .arg( errorColumn )
                   .arg( errorStr );
    return false;
  }

  const QDomElement root = doc.documentElement();
  if ( root.tagName() != QLatin1String( "qgsScales" ) )
  {
    errorMessage = QStringLiteral( "The file is not an scales exchange file." );
    return false;
  }

  QDomElement child = root.firstChildElement();
  while ( !child.isNull() )
  {
    scales.append( child.attribute( QStringLiteral( "value" ) ) );
    child = child.nextSiblingElement();
  }

  return true;
}

bool QgsScaleUtils::equalToOrGreaterThanMinimumScale( const double scale, const double minScale )
{
  return scale > minScale || qgsDoubleNear( scale, minScale, 1E-8 );
}

bool QgsScaleUtils::lessThanMaximumScale( const double scale, const double maxScale )
{
  return scale < maxScale && !qgsDoubleNear( scale, maxScale, 1E-8 );
}
