/***************************************************************************
                             qgsfallbacknumericformat.cpp
                             ----------------------------
    begin                : January 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com

 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#include "qgsfallbacknumericformat.h"


QString QgsFallbackNumericFormat::id() const
{
  return QStringLiteral( "default" );
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
