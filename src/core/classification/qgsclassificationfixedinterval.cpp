/***************************************************************************
    qgsclassificationfixedinterval.cpp
    ---------------------
    begin                : May 2022
    copyright            : (C) 2022 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsclassificationfixedinterval.h"

#include "qgsapplication.h"
#include "qgsprocessingcontext.h"

#include <QObject>

QgsClassificationFixedInterval::QgsClassificationFixedInterval()
  : QgsClassificationMethod( IgnoresClassCount, 0 )
{
  auto param = std::make_unique< QgsProcessingParameterNumber >( u"INTERVAL"_s, QObject::tr( "Interval size" ), Qgis::ProcessingNumberParameterType::Double, 1, false, 0.000000000001 );
  addParameter( param.release() );
}

std::unique_ptr<QgsClassificationMethod> QgsClassificationFixedInterval::clone() const
{
  auto c = std::make_unique< QgsClassificationFixedInterval >();
  copyBase( c.get() );
  return c;
}

QString QgsClassificationFixedInterval::name() const
{
  return QObject::tr( "Fixed Interval" );
}

QString QgsClassificationFixedInterval::id() const
{
  return u"Fixed"_s;
}

QIcon QgsClassificationFixedInterval::icon() const
{
  return QgsApplication::getThemeIcon( u"classification_methods/mClassificationFixedInterval.svg"_s );
}

QList<double> QgsClassificationFixedInterval::calculateBreaks( double &minimum, double &maximum, const QList<double> &, int, QString &error )
{
  const QgsProcessingContext context;
  const QgsProcessingParameterDefinition *def = parameterDefinition( u"INTERVAL"_s );
  const double interval = QgsProcessingParameters::parameterAsDouble( def, parameterValues(), context );

  QList<double> breaks;
  if ( qgsDoubleNear( interval, 0 ) || interval < 0 )
  {
    breaks << maximum;
    return breaks;
  }

  // Single class
  if ( minimum == maximum )
  {
    breaks << maximum + interval;
    return breaks;
  }

  double value = minimum;

  while ( value < maximum )
  {
    value += interval;
    breaks << value;
    // Limit number of classes to 999 to avoid overwhelming the gui
    if ( breaks.length() >= 999 )
    {
      error = QObject::tr( "The specified interval would generate too many classes.\nOnly the first 999 classes have been generated." );
      break;
    }
  }

  return breaks;
}

bool QgsClassificationFixedInterval::valuesRequired() const
{
  return false;
}
