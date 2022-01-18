/***************************************************************************
                               qgsexpressionutils.cpp
                             -------------------
    begin                : May 2017
    copyright            : (C) 2017 Matthias Kuhn
    email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspointcloudexpressionutils.h"
#include "qgspointcloudexpressionnode.h"
#include "qgsvectorlayer.h"
#include "qgscolorrampimpl.h"

///@cond PRIVATE

QgsPointcloudExpressionUtils::TVL QgsPointcloudExpressionUtils::AND[3][3] =
{
  // false  true    unknown
  { False, False,   False },   // false
  { False, True,    Unknown }, // true
  { False, Unknown, Unknown }  // unknown
};
QgsPointcloudExpressionUtils::TVL QgsPointcloudExpressionUtils::OR[3][3] =
{
  { False,   True, Unknown },  // false
  { True,    True, True },     // true
  { Unknown, True, Unknown }   // unknown
};

QgsPointcloudExpressionUtils::TVL QgsPointcloudExpressionUtils::NOT[3] = { True, False, Unknown };


QgsGradientColorRamp QgsPointcloudExpressionUtils::getRamp( const QVariant &value, QgsPointcloudExpression *parent, bool report_error )
{
  if ( value.canConvert<QgsGradientColorRamp>() )
    return value.value<QgsGradientColorRamp>();

  // If we get here then we can't convert so we just error and return invalid.
  if ( report_error )
    parent->setEvalErrorString( QObject::tr( "Cannot convert '%1' to gradient ramp" ).arg( value.toString() ) );

  return QgsGradientColorRamp();
}

///@endcond
