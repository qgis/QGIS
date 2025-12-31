/***************************************************************************
                             qgsfallbacknumericformat.cpp
                             ----------------------------
    begin                : January 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsfallbacknumericformat.h"

QString QgsFallbackNumericFormat::id() const
{
  return u"default"_s;
}

QString QgsFallbackNumericFormat::visibleName() const
{
  return QObject::tr( "General" );
}

int QgsFallbackNumericFormat::sortKey()
{
  return 0;
}

QString QgsFallbackNumericFormat::formatDouble( double value, const QgsNumericFormatContext & ) const
{
  return QString::number( value );
}

QgsNumericFormat *QgsFallbackNumericFormat::clone() const
{
  return new QgsFallbackNumericFormat();
}

QgsNumericFormat *QgsFallbackNumericFormat::create( const QVariantMap &, const QgsReadWriteContext & ) const
{
  return new QgsFallbackNumericFormat();
}

QVariantMap QgsFallbackNumericFormat::configuration( const QgsReadWriteContext & ) const
{
  return QVariantMap();
}
