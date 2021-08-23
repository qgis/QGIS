/***************************************************************************
    qgsclassificationquantile.h
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


#include "qgsclassificationquantile.h"
#include "qgsapplication.h"

QgsClassificationQuantile::QgsClassificationQuantile()
  : QgsClassificationMethod()
{
}

QString QgsClassificationQuantile::name() const
{
  return QObject::tr( "Equal Count (Quantile)" );
}

QString QgsClassificationQuantile::id() const
{
  return QStringLiteral( "Quantile" );
}

QgsClassificationMethod *QgsClassificationQuantile::clone() const
{
  QgsClassificationQuantile *c = new QgsClassificationQuantile();
  copyBase( c );
  return c;
}

QIcon QgsClassificationQuantile::icon() const
{
  return QgsApplication::getThemeIcon( "classification_methods/mClassificationEqualCount.svg" );
}


QList<double> QgsClassificationQuantile::calculateBreaks( double &minimum, double &maximum,
    const QList<double> &values, int nclasses )
{
  Q_UNUSED( minimum )
  Q_UNUSED( maximum )

  // q-th quantile of a data set:
  // value where q fraction of data is below and (1-q) fraction is above this value
  // Xq = (1 - r) * X_NI1 + r * X_NI2
  //   NI1 = (int) (q * (n+1))
  //   NI2 = NI1 + 1
  //   r = q * (n+1) - (int) (q * (n+1))
  // (indices of X: 1...n)

  // sort the values first
  QList<double> _values = values;
  std::sort( _values.begin(), _values.end() );

  QList<double> breaks;

  // If there are no values to process: bail out
  if ( _values.isEmpty() )
    return QList<double>();

  const int n = _values.count();
  double Xq = n > 0 ? _values[0] : 0.0;

  breaks.reserve( nclasses );
  for ( int i = 1; i < nclasses; i++ )
  {
    if ( n > 1 )
    {
      const double q = i  / static_cast< double >( nclasses );
      const double a = q * ( n - 1 );
      const int aa = static_cast<  int >( a );

      const double r = a - aa;
      Xq = ( 1 - r ) * _values[aa] + r * _values[aa + 1];
    }
    breaks.append( Xq );
  }

  breaks.append( _values[ n - 1 ] );

  return breaks;
}

