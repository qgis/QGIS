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

#include <QObject>

#include "qgsclassificationfixedinterval.h"
#include "qgsapplication.h"
#include "qgsprocessingcontext.h"


QgsClassificationFixedInterval::QgsClassificationFixedInterval()
  : QgsClassificationMethod( IgnoresClassCount, 0 )
{
  std::unique_ptr< QgsProcessingParameterNumber > param = std::make_unique< QgsProcessingParameterNumber >( QStringLiteral( "INTERVAL" ), QObject::tr( "Interval size" ), Qgis::ProcessingNumberParameterType::Double, 1, false, 0.000000000001 );
  addParameter( param.release() );
}

QgsClassificationMethod *QgsClassificationFixedInterval::clone() const
{
  QgsClassificationFixedInterval *c = new QgsClassificationFixedInterval();
  copyBase( c );
  return c;
}

QString QgsClassificationFixedInterval::name() const
{
  return QObject::tr( "Fixed Interval" );
}

QString QgsClassificationFixedInterval::id() const
{
  return QStringLiteral( "Fixed" );
}

QIcon QgsClassificationFixedInterval::icon() const
{
  return QgsApplication::getThemeIcon( QStringLiteral( "classification_methods/mClassificationFixedInterval.svg" ) );
}

QList<double> QgsClassificationFixedInterval::calculateBreaks( double &minimum, double &maximum, const QList<double> &, int )
{
  const QgsProcessingContext context;
  const QgsProcessingParameterDefinition *def = parameterDefinition( QStringLiteral( "INTERVAL" ) );
  double interval = QgsProcessingParameters::parameterAsDouble( def, parameterValues(), context );

  // Limit the interval so we do not get more than 999 classes
  const double minInterval = ( maximum - minimum ) / 999;
  bool maxClassesReached = false;
  if ( minInterval > interval )
  {
    interval = minInterval;
    auto parameters = parameterValues();
    parameters["INTERVAL"] = interval;
    setParameterValues( parameters );
    maxClassesReached = true;
  }

  QList<double> breaks;
  if ( qgsDoubleNear( interval, 0 ) || interval < 0 )
  {
    breaks << maximum;
    return breaks;
  }

  double value = minimum;
  while ( true )
  {
    value += interval;
    breaks << value;
    if ( value > maximum ||
         ( maxClassesReached && qgsDoubleNear( value, maximum, 1e-12 ) ) )
      break;
  }

  return breaks;
}

bool QgsClassificationFixedInterval::valuesRequired() const
{
  return false;
}
