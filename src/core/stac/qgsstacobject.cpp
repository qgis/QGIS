/***************************************************************************
    qgsstacobject.cpp
    ---------------------
    begin                : August 2024
    copyright            : (C) 2024 by Stefanos Natsis
    email                : uclaros at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsstacobject.h"


QgsStacObject::QgsStacObject( const QString &id, const QString &version, const QVector< QgsStacLink > &links )
  : mId( id )
  , mStacVersion( version )
  , mLinks( links )
{
}

QString QgsStacObject::stacVersion() const
{
  return mStacVersion;
}

void QgsStacObject::setStacVersion( const QString &version )
{
  mStacVersion = version;
}

QStringList QgsStacObject::stacExtensions() const
{
  return mStacExtensions;
}

void QgsStacObject::setStacExtensions( const QStringList &extensions )
{
  mStacExtensions = extensions;
}

QString QgsStacObject::id() const
{
  return mId;
}

void QgsStacObject::setId( const QString &id )
{
  mId = id;
}

QVector< QgsStacLink > QgsStacObject::links() const
{
  return mLinks;
}

void QgsStacObject::setLinks( const QVector< QgsStacLink > &links )
{
  mLinks = links;
}

QString QgsStacObject::url() const
{
  for ( const QgsStacLink &link : mLinks )
  {
    if ( link.relation() == QLatin1String( "self" ) )
      return link.href();
  }
  return QString();
}

QString QgsStacObject::rootUrl() const
{
  for ( const QgsStacLink &link : mLinks )
  {
    if ( link.relation() == QLatin1String( "root" ) )
      return link.href();
  }
  return QString();
}

QString QgsStacObject::parentUrl() const
{
  for ( const QgsStacLink &link : mLinks )
  {
    if ( link.relation() == QLatin1String( "parent" ) )
      return link.href();
  }
  return QString();
}


