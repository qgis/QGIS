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

#include "qgsclassificationlogarithmic.h"
#include "qgssymbollayerutils.h"
#include "qgsapplication.h"

QgsClassificationLogarithmic::QgsClassificationLogarithmic()
  : QgsClassificationMethod( ValuesNotRequired, 0 )
{

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

QList<double> QgsClassificationLogarithmic::calculateBreaks( double minimum, double maximum, const QList<double> &values, int nclasses )
{
  Q_UNUSED( values );

  // get the min/max in log10 scale
  int lmin = std::floor( std::log10( minimum ) );
  int lmax = std::ceil( std::log10( maximum ) );

  // do not create too many classes
  nclasses = std::min( lmax - lmin + 1, nclasses );

  // calculate pretty breaks
  QList<double> breaks = QgsSymbolLayerUtils::prettyBreaks( lmin, lmax, nclasses );

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

