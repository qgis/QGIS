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

#include <QDomDocument>
#include <QFile>
#include <QTextStream>

bool QgsScaleUtils::saveScaleList( const QString &fileName, const QStringList &scales, QString &errorMessage )
{
  QDomDocument doc;
  QDomElement root = doc.createElement( u"qgsScales"_s );
  root.setAttribute( u"version"_s, u"1.0"_s );
  doc.appendChild( root );

  for ( int i = 0; i < scales.count(); ++i )
  {
    QDomElement el = doc.createElement( u"scale"_s );
    el.setAttribute( u"value"_s, scales.at( i ) );
    root.appendChild( el );
  }

  QFile file( fileName );
  if ( !file.open( QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate ) )
  {
    errorMessage = u"Cannot write file %1:\n%2."_s.arg( fileName, file.errorString() );
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
    errorMessage = u"Cannot read file %1:\n%2."_s.arg( fileName, file.errorString() );
    return false;
  }

  QDomDocument doc;
  QString errorStr;
  int errorLine;
  int errorColumn;

  if ( !doc.setContent( &file, true, &errorStr, &errorLine, &errorColumn ) )
  {
    errorMessage = u"Parse error at line %1, column %2:\n%3"_s
                   .arg( errorLine )
                   .arg( errorColumn )
                   .arg( errorStr );
    return false;
  }

  const QDomElement root = doc.documentElement();
  if ( root.tagName() != "qgsScales"_L1 )
  {
    errorMessage = u"The file is not an scales exchange file."_s;
    return false;
  }

  QDomElement child = root.firstChildElement();
  while ( !child.isNull() )
  {
    scales.append( child.attribute( u"value"_s ) );
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
