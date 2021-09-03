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
#include "qgsapplication.h"

const QString QgsClassificationEqualInterval::METHOD_ID = QStringLiteral( "EqualInterval" );

QgsClassificationEqualInterval::QgsClassificationEqualInterval()
  : QgsClassificationMethod( SymmetricModeAvailable, 0 /*codeComplexity*/ )
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

QList<double> QgsClassificationEqualInterval::calculateBreaks( double &minimum, double &maximum,
    const QList<double> &values, int nclasses )
{
  Q_UNUSED( values )

  // Equal interval algorithm
  // Returns breaks based on dividing the range ('minimum' to 'maximum') into 'classes' parts.
  QList<double> breaks;
  if ( !symmetricModeEnabled() ) // normal mode
  {
    const double step = ( maximum - minimum ) / nclasses;

    double value = minimum;
    breaks.reserve( nclasses );
    for ( int i = 0; i < nclasses; i++ )
    {
      value += step;
      breaks << value;
    }
    // floating point arithmetic is not precise:
    // set the last break to be exactly maximum so we do not miss it
    breaks[nclasses - 1] = maximum;
  }
  else // symmetric mode
  {
    const double distBelowSymmetricValue = std::abs( minimum - symmetryPoint() );
    const double distAboveSymmetricValue = std::abs( maximum - symmetryPoint() ) ;

    if ( symmetryAstride() )
    {
      if ( nclasses % 2 == 0 ) // we want odd number of classes
        ++nclasses;
    }
    else
    {
      if ( nclasses % 2 == 1 ) // we want even number of classes
        ++nclasses;
    }
    const double step = 2 * std::min( distBelowSymmetricValue, distAboveSymmetricValue ) / nclasses;

    breaks.reserve( nclasses );
    double value = ( distBelowSymmetricValue < distAboveSymmetricValue ) ?  minimum : maximum - nclasses * step;

    for ( int i = 0; i < nclasses; i++ )
    {
      value += step;
      breaks << value;
    }
    breaks[nclasses - 1] = maximum;
  }

  return breaks;
}


QgsClassificationMethod *QgsClassificationEqualInterval::clone() const
{
  QgsClassificationEqualInterval *c = new QgsClassificationEqualInterval();
  copyBase( c );
  return c;
}

QIcon QgsClassificationEqualInterval::icon() const
{
  return QgsApplication::getThemeIcon( "classification_methods/mClassificationEqualInterval.svg" );
}

