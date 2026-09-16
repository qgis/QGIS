/***************************************************************************
  qgsprocessingparameterinterpolationsource.cpp
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

#include "qgsprocessingparameterinterpolationsource.h"

#include "qgsprocessingcontext.h"

#include <QString>

using namespace Qt::StringLiterals;


QgsProcessingParameterInterpolationSource::QgsProcessingParameterInterpolationSource( const QString &name, const QString &description )
  : QgsProcessingParameterDefinition( name, description )
{}

QgsProcessingParameterInterpolationSource *QgsProcessingParameterInterpolationSource::clone() const
{
  return new QgsProcessingParameterInterpolationSource( name(), description() );
}

QString QgsProcessingParameterInterpolationSource::type() const
{
  return typeName();
}

QString QgsProcessingParameterInterpolationSource::asPythonString( QgsProcessing::PythonOutputType outputType ) const
{
  switch ( outputType )
  {
    case QgsProcessing::PythonOutputType::PythonQgsProcessingAlgorithmSubclass:
    {
      QString code = u"QgsProcessingParameterInterpolationSource('%1', %2)"_s.arg( name(), QgsProcessingUtils::stringToPythonLiteral( description() ) );
      return code;
    }
  }
  return QString();
}

bool QgsProcessingParameterInterpolationSource::checkValueIsAcceptable( const QVariant &input, QgsProcessingContext * ) const
{
  if ( !input.isValid() && !mDefault.isValid() )
    return mFlags & Qgis::ProcessingParameterFlag::Optional;

  const QString stringValue = input.toString();
  if ( stringValue.isEmpty() || ( !input.isValid() && mDefault.userType() == QMetaType::Type::QString && mDefault.toString().isEmpty() ) )
    return mFlags & Qgis::ProcessingParameterFlag::Optional;

  const QStringList layerRows = stringValue.split( "::|::"_L1 );
  for ( const QString &row : layerRows )
  {
    if ( row.trimmed().isEmpty() )
      return false;

    const QStringList tokens = row.split( "::~::"_L1 );
    if ( tokens.count() < 4 )
    {
      return false;
    }
  }

  return true;
}
