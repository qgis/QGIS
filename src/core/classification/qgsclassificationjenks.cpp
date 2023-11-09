/***************************************************************************
    qgsclassificationjenks.h
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

#include <limits>
#include "qgsclassificationjenks.h"
#include "qgsapplication.h"

#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
#include <QRandomGenerator>
#endif

QgsClassificationJenks::QgsClassificationJenks()
  : QgsClassificationMethod()
{

}

QString QgsClassificationJenks::name() const
{
  return QObject::tr( "Natural Breaks (Jenks)" );
}

QString QgsClassificationJenks::id() const
{
  return QStringLiteral( "Jenks" );
}

QgsClassificationMethod *QgsClassificationJenks::clone() const
{
  QgsClassificationJenks *c = new QgsClassificationJenks();
  copyBase( c );
  return c;
}

QIcon QgsClassificationJenks::icon() const
{
  return QgsApplication::getThemeIcon( "classification_methods/mClassificationNaturalBreak.svg" );
}


QList<double> QgsClassificationJenks::calculateBreaks( double &minimum, double &maximum,
    const QList<double> &values, int nclasses )
{
  // Jenks Optimal (Natural Breaks) algorithm
  // Based on the Jenks algorithm from the 'classInt' package available for
  // the R statistical prgramming language, and from Python code from here:
  // http://danieljlewis.org/2010/06/07/jenks-natural-breaks-algorithm-in-python/
  // and is based on a JAVA and Fortran code available here:
  // https://stat.ethz.ch/pipermail/r-sig-geo/2006-March/000811.html

  // Returns class breaks such that classes are internally homogeneous while
  // assuring heterogeneity among classes.

  if ( values.isEmpty() )
    return QList<double>();

  if ( nclasses <= 1 )
  {
    return QList<double>() <<  maximum;
  }

  if ( nclasses >= values.size() )
  {
    return values;
  }

  QVector<double> sample;
  QVector<double> sorted;

  // if we have lots of values, we need to take a random sample
  if ( values.size() > mMaximumSize )
  {
    // for now, sample at least maximumSize values or a 10% sample, whichever
    // is larger. This will produce a more representative sample for very large
    // layers, but could end up being computationally intensive...

    sample.resize( std::max( mMaximumSize, static_cast<int>( values.size() ) / 10 ) );

    QgsDebugMsgLevel( QStringLiteral( "natural breaks (jenks) sample size: %1" ).arg( sample.size() ), 2 );
    QgsDebugMsgLevel( QStringLiteral( "values:%1" ).arg( values.size() ), 2 );

    sample[ 0 ] = minimum;
    sample[ 1 ] = maximum;

    sorted = values.toVector();
    std::sort( sorted.begin(), sorted.end() );

    int j = -1;

    // loop through all values in initial array
    // skip the first value as it is a minimum one
    // skip the last value as that one is a maximum one
    // and those are already in the sample as items 0 and 1
    for ( int i = 1; i < sorted.size() - 2; i++ )
    {
      if ( ( i * ( mMaximumSize - 2 ) / ( sorted.size() - 2 ) ) > j )
      {
        j++;
        sample[ j + 2 ] = sorted[ i ];
      }
    }
  }
  else
  {
    sample = values.toVector();
  }

  const int n = sample.size();

  // sort the sample values
  std::sort( sample.begin(), sample.end() );

  QVector< QVector<int> > matrixOne( n + 1 );
  QVector< QVector<double> > matrixTwo( n + 1 );

  for ( int i = 0; i <= n; i++ )
  {
    matrixOne[i].resize( nclasses + 1 );
    matrixTwo[i].resize( nclasses + 1 );
  }

  for ( int i = 1; i <= nclasses; i++ )
  {
    matrixOne[0][i] = 1;
    matrixOne[1][i] = 1;
    matrixTwo[0][i] = 0.0;
    for ( int j = 2; j <= n; j++ )
    {
      matrixTwo[j][i] = std::numeric_limits<double>::max();
    }
  }

  for ( int l = 2; l <= n; l++ )
  {
    double s1 = 0.0;
    double s2 = 0.0;
    int w = 0;

    double v = 0.0;

    for ( int m = 1; m <= l; m++ )
    {
      const int i3 = l - m + 1;

      const double val = sample[ i3 - 1 ];

      s2 += val * val;
      s1 += val;
      w++;

      v = s2 - ( s1 * s1 ) / static_cast< double >( w );
      const int i4 = i3 - 1;
      if ( i4 != 0 )
      {
        for ( int j = 2; j <= nclasses; j++ )
        {
          if ( matrixTwo[l][j] >= v + matrixTwo[i4][j - 1] )
          {
            matrixOne[l][j] = i4;
            matrixTwo[l][j] = v + matrixTwo[i4][j - 1];
          }
        }
      }
    }
    matrixOne[l][1] = 1;
    matrixTwo[l][1] = v;
  }

  QVector<double> breaks( nclasses );
  breaks[nclasses - 1] = sample[n - 1];

  for ( int j = nclasses, k = n; j >= 2; j-- )
  {
    const int id = matrixOne[k][j] - 1;
    breaks[j - 2] = sample[id];
    k = matrixOne[k][j] - 1;
  }

  return breaks.toList();
}
