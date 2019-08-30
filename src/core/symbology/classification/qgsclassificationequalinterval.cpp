/***************************************************************************
    qgsclassificationequalinterval.h
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

#include "qgsclassificationequalinterval.h"

const QString QgsClassificationEqualInterval::METHOD_ID = QStringLiteral( "EqualInterval" );

QgsClassificationEqualInterval::QgsClassificationEqualInterval()
  : QgsClassificationMethod( false /*valuesRequired*/,
                             true /*symmetricMode*/,
                             0 /*codeComplexity*/ )
{
}

QString QgsClassificationEqualInterval::name() const
{
  return QObject::tr( "Equal Interval" );
}

QString QgsClassificationEqualInterval::id() const
{
  return METHOD_ID;
}

QList<double> QgsClassificationEqualInterval::calculateBreaks( double minimum, double maximum,
    const QList<double> &values, int numberOfClasses )
{
  Q_UNUSED( values )

  // Equal interval algorithm
  // Returns breaks based on dividing the range ('minimum' to 'maximum') into 'classes' parts.
  QList<double> breaks;
  if ( !symmetricModeEnabled() ) // normal mode
  {
    double step = ( maximum - minimum ) / numberOfClasses;

    double value = minimum;
    breaks.reserve( numberOfClasses );
    for ( int i = 0; i < numberOfClasses; i++ )
    {
      value += step;
      breaks << value;
    }
    // floating point arithmetics is not precise:
    // set the last break to be exactly maximum so we do not miss it
    breaks[numberOfClasses - 1] = maximum;
  }
  else // symmetric mode
  {
    double distBelowSymmetricValue = std::abs( minimum - symmetryPoint() );
    double distAboveSymmetricValue = std::abs( maximum - symmetryPoint() ) ;

    if ( astride() )
    {
      if ( numberOfClasses % 2 == 0 ) // we want odd number of classes
        ++numberOfClasses;
    }
    else
    {
      if ( numberOfClasses % 2 == 1 ) // we want even number of classes
        ++numberOfClasses;
    }
    double step = 2 * std::min( distBelowSymmetricValue, distAboveSymmetricValue ) / numberOfClasses;

    breaks.reserve( numberOfClasses );
    double value = ( distBelowSymmetricValue < distAboveSymmetricValue ) ?  minimum : maximum - numberOfClasses * step;

    for ( int i = 0; i < numberOfClasses; i++ )
    {
      value += step;
      breaks << value;
    }
    breaks[numberOfClasses - 1] = maximum;
  }

  return breaks;
}


QgsClassificationMethod *QgsClassificationEqualInterval::clone() const
{
  QgsClassificationEqualInterval *c = new QgsClassificationEqualInterval();
  copyBase( c );
  return c;
}
