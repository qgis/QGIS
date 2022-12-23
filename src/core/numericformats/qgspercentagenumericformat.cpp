/***************************************************************************
                             qgspercentagenumericformat.cpp
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

#include "qgspercentagenumericformat.h"
#include "qgis.h"


QgsPercentageNumericFormat::QgsPercentageNumericFormat()
{
}

QString QgsPercentageNumericFormat::id() const
{
  return QStringLiteral( "percentage" );
}

QString QgsPercentageNumericFormat::visibleName() const
{
  return QObject::tr( "Percentage" );
}

int QgsPercentageNumericFormat::sortKey()
{
  return DEFAULT_SORT_KEY;
}

double QgsPercentageNumericFormat::suggestSampleValue() const
{
  switch ( mInputValues )
  {
    case ValuesArePercentage:
      return 50.1234;

    case ValuesAreFractions:
      return 0.501234;
  }
  return 50.1234; // no warnings
}

QString QgsPercentageNumericFormat::formatDouble( double value, const QgsNumericFormatContext &context ) const
{
  switch ( mInputValues )
  {
    case ValuesArePercentage:
      break;

    case ValuesAreFractions:
      value *= 100;
      break;
  }

  return QgsBasicNumericFormat::formatDouble( value, context ) + context.percent();
}

QgsNumericFormat *QgsPercentageNumericFormat::clone() const
{
  return new QgsPercentageNumericFormat( *this );
}

QgsNumericFormat *QgsPercentageNumericFormat::create( const QVariantMap &configuration, const QgsReadWriteContext &context ) const
{
  std::unique_ptr< QgsPercentageNumericFormat > res = std::make_unique< QgsPercentageNumericFormat >();
  res->setConfiguration( configuration, context );
  res->mInputValues = static_cast< InputValues >( configuration.value( QStringLiteral( "input_values" ), static_cast< int >( ValuesArePercentage ) ).toInt() );
  res->setRoundingType( QgsBasicNumericFormat::DecimalPlaces );
  return res.release();
}

QVariantMap QgsPercentageNumericFormat::configuration( const QgsReadWriteContext &context ) const
{
  QVariantMap res = QgsBasicNumericFormat::configuration( context );
  res.insert( QStringLiteral( "input_values" ), static_cast< int >( mInputValues ) );
  return res;
}

QgsPercentageNumericFormat::InputValues QgsPercentageNumericFormat::inputValues() const
{
  return mInputValues;
}

void QgsPercentageNumericFormat::setInputValues( InputValues inputValues )
{
  mInputValues = inputValues;
}
