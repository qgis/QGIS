/***************************************************************************
    qgsclassificationlogarithmic.h
    ---------------------
    begin                : September 2019
    copyright            : (C) 2019 by Denis Rouzaud
    email                : denis@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QObject>

#include "qgsclassificationlogarithmic.h"
#include "qgssymbollayerutils.h"
#include "qgsapplication.h"
#include "qgsprocessingcontext.h"


QgsClassificationLogarithmic::QgsClassificationLogarithmic()
  : QgsClassificationMethod( NoFlag, 0 )
{
  QgsProcessingParameterEnum *param = new QgsProcessingParameterEnum( QStringLiteral( "ZERO_NEG_VALUES_HANDLE" ), QObject::tr( "Handling of 0 or negative values" ), QStringList() << QObject::tr( "no handling (faster)" ) << QObject::tr( "discard (slower)" ) << QObject::tr( "prepend range (slower)" ), false, 0 );
  addParameter( param );
}


QgsClassificationMethod *QgsClassificationLogarithmic::clone() const
{
  QgsClassificationLogarithmic *c = new QgsClassificationLogarithmic();
  copyBase( c );
  return c;
}

QString QgsClassificationLogarithmic::name() const
{
  return QObject::tr( "Logarithmic scale" );
}

QString QgsClassificationLogarithmic::id() const
{
  return QStringLiteral( "Logarithmic" );
}

QIcon QgsClassificationLogarithmic::icon() const
{
  return QgsApplication::getThemeIcon( "classification_methods/mClassificationLogarithmic.svg" );
}

QList<double> QgsClassificationLogarithmic::calculateBreaks( double &minimum, double &maximum, const QList<double> &values, int nclasses )
{
  QgsProcessingContext context;
  const QgsProcessingParameterDefinition *def = parameterDefinition( QStringLiteral( "ZERO_NEG_VALUES_HANDLE" ) );
  NegativeValueHandling nvh = static_cast< NegativeValueHandling >( QgsProcessingParameters::parameterAsEnum( def, parameterValues(), context ) );

  double positiveMinimum = std::numeric_limits<double>::max();
  if ( nvh != NegativeValueHandling::NoHandling && minimum <= 0 )
  {
    Q_ASSERT( values.count() );
    if ( maximum > 0 )
    {
      for ( int i = 0; i < values.count(); i++ )
      {
        if ( values.at( i ) > 0 )
          positiveMinimum = std::min( positiveMinimum, values.at( i ) );
      }
    }
    if ( positiveMinimum == std::numeric_limits<double>::max() )
    {
      // there is no usable values
      if ( nvh == NegativeValueHandling::PrependBreak )
        return QList<double>( {0} );
      else
        return QList<double>();
    }
  }

  QList<double> breaks;

  if ( positiveMinimum != std::numeric_limits<double>::max() )
  {
    if ( nvh == NegativeValueHandling::PrependBreak )
      breaks << std::floor( std::log10( positiveMinimum ) );
    else if ( nvh == NegativeValueHandling::Discard )
      minimum = positiveMinimum; // the minimum gets updated
  }
  else
  {
    positiveMinimum = minimum;
  }

  // get the min/max in log10 scale
  double logMin = std::floor( std::log10( positiveMinimum ) );
  double logMax = std::ceil( std::log10( maximum ) );

  // calculate pretty breaks
  breaks.append( QgsSymbolLayerUtils::prettyBreaks( logMin, logMax, nclasses ) );

  // create the value
  for ( int i = 0; i < breaks.count(); i++ )
    breaks[i] = std::pow( 10, breaks.at( i ) );

  return breaks;
}

QString QgsClassificationLogarithmic::valueToLabel( double value ) const
{
  if ( value <= 0 )
  {
    return QLocale().toString( value );
  }
  else
  {
    if ( std::isnan( value ) )
    {
      return QObject::tr( "invalid (0 or negative values in the data)" );
    }
    else
    {
      return QString( QStringLiteral( "10^%1" ) ).arg( std::log10( value ) );
    }
  }
}

QString QgsClassificationLogarithmic::labelForRange( double lowerValue, double upperValue, QgsClassificationMethod::ClassPosition position ) const
{
  QString lowerLabel;
  const QString upperLabel = valueToLabel( upperValue );

  switch ( position )
  {
    case LowerBound:
      lowerLabel = formatNumber( lowerValue ); // avoid to have float exponent for the minimum value
      break;
    case Inner:
    case UpperBound:
      lowerLabel = valueToLabel( lowerValue );
      break;
  }

  return labelFormat().arg( lowerLabel ).arg( upperLabel );
}

bool QgsClassificationLogarithmic::valuesRequired() const
{
  QgsProcessingContext context;
  const QgsProcessingParameterDefinition *def = parameterDefinition( QStringLiteral( "ZERO_NEG_VALUES_HANDLE" ) );
  NegativeValueHandling nvh = static_cast< NegativeValueHandling >( QgsProcessingParameters::parameterAsEnum( def, parameterValues(), context ) );

  return nvh != NegativeValueHandling::NoHandling;
}
