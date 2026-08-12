/***************************************************************************
  qgsprocessingparameterinterpolationpixelsize.cpp
  ---------------------
  Date                 : August 2026
  Copyright            : (C) 2026 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsprocessingparameterinterpolationpixelsize.h"

#include "qgsprocessingcontext.h"

#include <QString>

using namespace Qt::StringLiterals;


QgsProcessingParameterInterpolationPixelSize::QgsProcessingParameterInterpolationPixelSize(
  const QString &name, const QString &description, const QString &interpolationSourceParameter, const QString &extentParameter, const QVariant &defaultValue
)
  : QgsProcessingParameterNumber( name, description, Qgis::ProcessingNumberParameterType::Double, defaultValue, false, 0 )
  , mInterpolationSourceParam( interpolationSourceParameter )
  , mExtentParam( extentParameter )
{}

QgsProcessingParameterInterpolationPixelSize *QgsProcessingParameterInterpolationPixelSize::clone() const
{
  return new QgsProcessingParameterInterpolationPixelSize( name(), description(), mInterpolationSourceParam, mExtentParam, defaultValue() );
}

QString QgsProcessingParameterInterpolationPixelSize::type() const
{
  return typeName();
}

QString QgsProcessingParameterInterpolationPixelSize::asPythonString( QgsProcessing::PythonOutputType outputType ) const
{
  switch ( outputType )
  {
    case QgsProcessing::PythonOutputType::PythonQgsProcessingAlgorithmSubclass:
    {
      QString code
        = u"QgsProcessingParameterInterpolationPixelSize('%1', %2, %3, %4"_s
            .arg( name(), QgsProcessingUtils::stringToPythonLiteral( description() ), QgsProcessingUtils::stringToPythonLiteral( mInterpolationSourceParam ), QgsProcessingUtils::stringToPythonLiteral( mExtentParam ) );
      QgsProcessingContext c;
      code += u", defaultValue=%1)"_s.arg( valueAsPythonString( mDefault, c ) );
      return code;
    }
  }
  return QString();
}
