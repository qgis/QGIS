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

#include <QFile>
#include <QDomDocument>
#include <QTextStream>

#include "qgsscaleutils.h"

bool QgsScaleUtils::saveScaleList( const QString &fileName, const QStringList &scales, QString &errorMessage )
{
  QDomDocument doc;
  QDomElement root = doc.createElement( "qgsScales" );
  root.setAttribute( "version", "1.0" );
  doc.appendChild( root );

  for ( int i = 0; i < scales.count(); ++i )
  {
    QDomElement el = doc.createElement( "scale" );
    el.setAttribute( "value", scales.at( i ) );
    root.appendChild( el );
  }

  QFile file( fileName );
  if ( !file.open( QIODevice::WriteOnly | QIODevice::Text ) )
  {
    errorMessage = QString( "Cannot write file %1:\n%2." ).arg( fileName ).arg( file.errorString() );
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
    errorMessage = QString( "Cannot read file %1:\n%2." ).arg( fileName ).arg( file.errorString() );
    return false;
  }

  QDomDocument doc;
  QString errorStr;
  int errorLine;
  int errorColumn;

  if ( !doc.setContent( &file, true, &errorStr, &errorLine, &errorColumn ) )
  {
    errorMessage = QString( "Parse error at line %1, column %2:\n%3" )
                   .arg( errorLine )
                   .arg( errorColumn )
                   .arg( errorStr );
    return false;
  }

  QDomElement root = doc.documentElement();
  if ( root.tagName() != "qgsScales" )
  {
    errorMessage = "The file is not an scales exchange file.";
    return false;
  }

  QDomElement child = root.firstChildElement();
  while ( !child.isNull() )
  {
    scales.append( child.attribute( "value" ) );
    child = child.nextSiblingElement();
  }

  return true;
}
