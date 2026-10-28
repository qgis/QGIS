/***************************************************************************
  qgsprocessingparameterheatmappixelsize.cpp
  ---------------------
  Date                 : June 2026
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

#include "qgsprocessingparameterheatmappixelsize.h"

#include "qgsprocessingcontext.h"

#include <QString>

using namespace Qt::StringLiterals;


QgsProcessingParameterHeatmapPixelSize::QgsProcessingParameterHeatmapPixelSize(
  const QString &name, const QString &description, const QString &parentLayerParameter, const QString &radiusParameter, const QString &radiusFieldParameter, const QVariant &defaultValue
)
  : QgsProcessingParameterNumber( name, description, Qgis::ProcessingNumberParameterType::Double, defaultValue, false, 0 )
  , mParentLayer( parentLayerParameter )
  , mRadiusParam( radiusParameter )
  , mRadiusFieldParam( radiusFieldParameter )
{}

QgsProcessingParameterHeatmapPixelSize *QgsProcessingParameterHeatmapPixelSize::clone() const
{
  return new QgsProcessingParameterHeatmapPixelSize( name(), description(), mParentLayer, mRadiusParam, mRadiusFieldParam, defaultValue() );
}

QString QgsProcessingParameterHeatmapPixelSize::type() const
{
  return typeName();
}

QString QgsProcessingParameterHeatmapPixelSize::asPythonString( QgsProcessing::PythonOutputType outputType ) const
{
  switch ( outputType )
  {
    case QgsProcessing::PythonOutputType::PythonQgsProcessingAlgorithmSubclass:
    {
      QString code
        = u"QgsProcessingParameterHeatmapPixelSize('%1', %2, %3, %4, %5"_s
            .arg( name(), QgsProcessingUtils::stringToPythonLiteral( description() ), QgsProcessingUtils::stringToPythonLiteral( mParentLayer ), QgsProcessingUtils::stringToPythonLiteral( mRadiusParam ), QgsProcessingUtils::stringToPythonLiteral( mRadiusFieldParam ) );
      QgsProcessingContext c;
      code += u", defaultValue=%1)"_s.arg( valueAsPythonString( mDefault, c ) );
      return code;
    }
  }
  return QString();
}
