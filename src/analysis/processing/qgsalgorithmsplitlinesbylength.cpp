/***************************************************************************
                         qgsalgorithmsplitlinesbylength.cpp
                         ---------------------
    begin                : December 2018
    copyright            : (C) 2018 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsalgorithmsplitlinesbylength.h"
#include "qgscurve.h"
#include "qgslinestring.h"
#include "qgscircularstring.h"
#include "qgscompoundcurve.h"
#include "qgsgeometrycollection.h"

///@cond PRIVATE

QString QgsSplitLinesByLengthAlgorithm::name() const
{
  return QStringLiteral( "splitlinesbylength" );
}

QString QgsSplitLinesByLengthAlgorithm::displayName() const
{
  return QObject::tr( "Split lines by maximum length" );
}

QStringList QgsSplitLinesByLengthAlgorithm::tags() const
{
  return QObject::tr( "segments,parts,distance,cut,chop" ).split( ',' );
}

QString QgsSplitLinesByLengthAlgorithm::group() const
{
  return QObject::tr( "Vector geometry" );
}

QString QgsSplitLinesByLengthAlgorithm::groupId() const
{
  return QStringLiteral( "vectorgeometry" );
}

QString QgsSplitLinesByLengthAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm takes a line (or curve) layer and splits each feature into multiple parts, "
                      "where each part is of a specified maximum length.\n\n"
                      "Z and M values at the start and end of the new line substrings are linearly interpolated from existing values." );
}

QString QgsSplitLinesByLengthAlgorithm::shortDescription() const
{
  return QObject::tr( "Splits lines into parts which are no longer than a specified length." );
}

QList<int> QgsSplitLinesByLengthAlgorithm::inputLayerTypes() const
{
  return QList<int>() << QgsProcessing::TypeVectorLine;
}

QgsProcessing::SourceType QgsSplitLinesByLengthAlgorithm::outputLayerType() const
{
  return QgsProcessing::TypeVectorLine;
}

QgsSplitLinesByLengthAlgorithm *QgsSplitLinesByLengthAlgorithm::createInstance() const
{
  return new QgsSplitLinesByLengthAlgorithm();
}

void QgsSplitLinesByLengthAlgorithm::initParameters( const QVariantMap & )
{
  std::unique_ptr< QgsProcessingParameterDistance > length = qgis::make_unique< QgsProcessingParameterDistance >( QStringLiteral( "LENGTH" ),
      QObject::tr( "Maximum line length" ), 10, QStringLiteral( "INPUT" ), false, 0 );
  length->setIsDynamic( true );
  length->setDynamicPropertyDefinition( QgsPropertyDefinition( QStringLiteral( "LENGTH" ), QObject::tr( "Maximum length" ), QgsPropertyDefinition::DoublePositive ) );
  length->setDynamicLayerParameterName( QStringLiteral( "INPUT" ) );
  addParameter( length.release() );
}

bool QgsSplitLinesByLengthAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mLength = parameterAsDouble( parameters, QStringLiteral( "LENGTH" ), context );
  mDynamicLength = QgsProcessingParameters::isDynamic( parameters, QStringLiteral( "LENGTH" ) );
  if ( mDynamicLength )
    mLengthProperty = parameters.value( QStringLiteral( "LENGTH" ) ).value< QgsProperty >();

  return true;
}

QString QgsSplitLinesByLengthAlgorithm::outputName() const
{
  return QObject::tr( "Split" );
}

QgsWkbTypes::Type QgsSplitLinesByLengthAlgorithm::outputWkbType( QgsWkbTypes::Type inputWkbType ) const
{
  return QgsWkbTypes::singleType( inputWkbType );
}

QgsFeatureList QgsSplitLinesByLengthAlgorithm::processFeature( const QgsFeature &f, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  if ( !f.hasGeometry() )
  {
    return QgsFeatureList() << f;
  }
  else
  {
    double distance = mLength;
    if ( mDynamicLength )
      distance = mLengthProperty.valueAsDouble( context.expressionContext(), distance );

    QgsFeature outputFeature;
    outputFeature.setAttributes( f.attributes() );
    QgsFeatureList features;
    const QgsGeometry inputGeom = f.geometry();
    for ( auto it = inputGeom.const_parts_begin(); it != inputGeom.const_parts_end(); ++it )
    {
      const QgsCurve *part = qgsgeometry_cast< const QgsCurve * >( *it );
      if ( !part )
        continue;

      double start = 0.0;
      double end = distance;
      const double length = part->length();
      while ( start < length )
      {
        outputFeature.setGeometry( QgsGeometry( part->curveSubstring( start, end ) ) );
        start += distance;
        end += distance;
        features << outputFeature;
      }

    }
    return features;
  }
}

QgsProcessingFeatureSource::Flag QgsSplitLinesByLengthAlgorithm::sourceFlags() const
{
  return QgsProcessingFeatureSource::FlagSkipGeometryValidityChecks;
}


///@endcond



