/***************************************************************************
                             qgsscientificnumericformat.cpp
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

#include "qgsscientificnumericformat.h"
#include "qgis.h"


QgsScientificNumericFormat::QgsScientificNumericFormat()
{
  mUseScientific = true;
}

QString QgsScientificNumericFormat::id() const
{
  return QStringLiteral( "scientific" );
}

QString QgsScientificNumericFormat::visibleName() const
{
  return QObject::tr( "Scientific" );
}

int QgsScientificNumericFormat::sortKey()
{
  return DEFAULT_SORT_KEY;
}

QString QgsScientificNumericFormat::formatDouble( double value, const QgsNumericFormatContext &context ) const
{
  return QgsBasicNumericFormat::formatDouble( value, context );
}

QgsNumericFormat *QgsScientificNumericFormat::clone() const
{
  return new QgsScientificNumericFormat( *this );
}

QgsNumericFormat *QgsScientificNumericFormat::create( const QVariantMap &configuration, const QgsReadWriteContext &context ) const
{
  std::unique_ptr< QgsScientificNumericFormat > res = std::make_unique< QgsScientificNumericFormat >();
  res->setConfiguration( configuration, context );
  res->setRoundingType( QgsBasicNumericFormat::DecimalPlaces );
  return res.release();
}

QVariantMap QgsScientificNumericFormat::configuration( const QgsReadWriteContext &context ) const
{
  QVariantMap res = QgsBasicNumericFormat::configuration( context );
  return res;
}

void QgsScientificNumericFormat::setNumberDecimalPlaces( int numberDecimalPlaces )
{
  QgsBasicNumericFormat::setNumberDecimalPlaces( std::max( numberDecimalPlaces, 1 ) );
}
