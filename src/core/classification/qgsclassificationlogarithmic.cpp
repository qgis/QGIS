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

QgsClassificationLogarithmic::QgsClassificationLogarithmic()
  : QgsClassificationMethod( NoFlag, 0 )
{
  QgsProcessingParameterBoolean *param = new QgsProcessingParameterBoolean( QStringLiteral( "FILTER_ZERO_NEG_VALUES" ), QObject::tr( "Filter values <= 0" ), false );
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
  // not possible if only negative values
  if ( maximum <= 0 )
    return QList<double>();

  if ( parameterValue( QStringLiteral( "FILTER_ZERO_NEG_VALUES" ) ).toBool() && minimum <= 0 )
  {
    Q_ASSERT( values.count() );
    minimum = std::numeric_limits<double>::max();
    for ( int i = 0; i < values.count(); i++ )
    {
      if ( values.at( i ) > 0 )
        minimum = std::min( minimum, values.at( i ) );
    }
    if ( minimum == std::numeric_limits<double>::max() )
      return QList<double>();
  }

  // get the min/max in log10 scale
  double log_min = std::floor( std::log10( minimum ) );
  double log_max = std::ceil( std::log10( maximum ) );

  // calculate pretty breaks
  QList<double> breaks = QgsSymbolLayerUtils::prettyBreaks( log_min, log_max, nclasses );

  // create the value
  for ( int i = 0; i < breaks.count(); i++ )
    breaks[i] = std::pow( 10, breaks.at( i ) );

  return breaks;
}

QString QgsClassificationLogarithmic::valueToLabel( double value ) const
{
  QString label = QString( QStringLiteral( "10^%1" ) ).arg( std::log10( value ) );
  return label;
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
  return parameterValue( QStringLiteral( "FILTER_ZERO_NEG_VALUES" ) ).toBool();
}
